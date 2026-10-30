// SPDX-License-Identifier: GPL-2.0
/*
 * ST7123 Touchscreen Controller Driver
 *
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/input/touchscreen.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm_qos.h>
#include <linux/slab.h>
#include <linux/types.h>

#define ST7123_TS_NAME	"st7123-ts"

#define REG_STATUS		0x0001

#define STATUS_NORMAL		0x00
#define STATUS_INIT		0x01
#define STATUS_ERROR		0x02
#define STATUS_SMART_WAKEUP	0x03
#define STATUS_IDLE		0x04
#define STATUS_POWER_DOWN	0x05
#define STATUS_PRE_SMART_WAKEUP	0x06
#define STATUS_PROXIMITY	0x07
#define STATUS_TRGT_PROXIMITY   0x08

#define ERROR_NONE		0x00
#define ERROR_INVALID_ADDRESS	0x10
#define ERROR_INVALID_VALUE	0x20
#define ERROR_INVALID_PLATFORM	0x30
#define ERROR_DEV_NOT_FOUND	0x40
#define ERROR_STACK_OVERFLOW	0x50
#define ERROR_INVALID_FWT	0x60

#define REG_FW_VERSION		0x0000
#define REG_FW_REVISION		0x000C
#define REG_MAX_X_COORD_H	0x0005
#define REG_MAX_X_COORD_L	0x0006
#define REG_MAX_Y_COORD_H	0x0007
#define REG_MAX_Y_COORD_L	0x0008
#define REG_MAX_TOUCHES		0x0009
#define REG_REPORT_TABLE	0x0010
#define ST_TS_MAX_FINGERS	10

struct st_chip_info {
	bool	have_z;
	u16	max_area;
	u16	max_fingers;
};

struct st7123_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct touchscreen_properties prop;
	struct dev_pm_qos_request low_latency_req;
	struct gpio_desc *reset_gpio;
	const struct st_chip_info *chip_info;
	int read_buf_len;
	u8 *read_buf;
};

static int st7123_ts_read_data(struct st7123_ts_data *ts, u16 reg,
			       unsigned int n)
{
	u8 reg_bytes[2];

	reg_bytes[0] = reg>>8;
	reg_bytes[1] = reg&0x00FF;

	struct i2c_client *client = ts->client;
	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.len	= sizeof(reg_bytes),
			.buf	= reg_bytes,
		},
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD | I2C_M_DMA_SAFE,
			.len	= n,
			.buf	= ts->read_buf,
		}
	};
	int ret;

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg))
		return ret < 0 ? ret : -EIO;

	return 0;
}

static int st7123_ts_wait_ready(struct st7123_ts_data *ts)
{
	unsigned int retries;
	int error;

	for (retries = 100; retries; retries--) {
		error = st7123_ts_read_data(ts, REG_STATUS, 1);
		if (!error) {
			switch (ts->read_buf[0]) {
			case STATUS_NORMAL | ERROR_NONE:
			case STATUS_IDLE | ERROR_NONE:
				return 0;
			}
		}

		usleep_range(1000, 2000);
	}

	return -ENXIO;
}

static int st7123_ts_read_resolution(struct st7123_ts_data *ts, u16 *max_x,
				     u16 *max_y)
{
	u8 *buf;
	int error;

	/* get max x,y coord */
	error = st7123_ts_read_data(ts, REG_MAX_X_COORD_H, 4);
	if (error)
		return error;

	buf = ts->read_buf;

	*max_x = (((buf[0] & 0x003f) << 8) | buf[1]) - 1;
	*max_y = (((buf[2] & 0x003f) << 8) | buf[3]) - 1;

	return 0;
}

static int st7123_ts_parse_and_report(struct st7123_ts_data *ts)
{
	struct input_dev *input = ts->input_dev;
	struct input_mt_pos pos[ST_TS_MAX_FINGERS];
	u8 z[ST_TS_MAX_FINGERS];
	int slots[ST_TS_MAX_FINGERS];
	int n_contacts = 0;
	int i;

	for (i = 0; i < ts->chip_info->max_fingers; i++) {
		u8 *buf = &ts->read_buf[i * 8 + 4];

		if (buf[0] & BIT(7)) {
			unsigned int x = ((buf[0] & 0x003f) << 8) | buf[1];
			unsigned int y = ((buf[2] & 0x003f) << 8) | buf[3];

			touchscreen_set_mt_pos(&pos[n_contacts],
					       &ts->prop, x, y);

			if(ts->chip_info->have_z)
				z[n_contacts] = buf[5];

			n_contacts++;
		}
	}

	input_mt_assign_slots(input, slots, pos, n_contacts, 0);
	for (i = 0; i < n_contacts; i++) {
		input_mt_slot(input, slots[i]);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
		input_report_abs(input, ABS_MT_POSITION_X, pos[i].x);
		input_report_abs(input, ABS_MT_POSITION_Y, pos[i].y);
		if (ts->chip_info->have_z)
			input_report_abs(input, ABS_MT_TOUCH_MAJOR, z[i]);
	}

	input_mt_sync_frame(input);
	input_sync(input);

	return n_contacts;
}

static irqreturn_t st7123_ts_irq_handler(int irq, void *dev_id)
{
	struct st7123_ts_data *ts = dev_id;
	int count;
	int error;

	error = st7123_ts_read_data(ts, REG_REPORT_TABLE, ts->read_buf_len);
	if (error)
		goto out;

	count = st7123_ts_parse_and_report(ts);
	if (!count) {
		if (ts->low_latency_req.dev) {
			dev_pm_qos_remove_request(&ts->low_latency_req);
			ts->low_latency_req.dev = NULL;
		}
	} else if (!ts->low_latency_req.dev) {
		/* First contact, request 100 us latency. */
		dev_pm_qos_add_ancestor_request(&ts->client->dev,
						&ts->low_latency_req,
						DEV_PM_QOS_RESUME_LATENCY, 100);
	}

out:
	return IRQ_HANDLED;
}

/*
static void st7123_ts_power(struct st7123_ts_data *ts, bool poweron)
{
	if (ts->reset_gpio)
		gpiod_set_value_cansleep(ts->reset_gpio, poweron);
}
*/

static void st7123_ts_power(struct st7123_ts_data *ts, bool poweron)
{
    if (!ts->reset_gpio)
        return;
    if (poweron) {
        // 先拉低（复位有效）
        gpiod_set_value_cansleep(ts->reset_gpio, 1);
        msleep(10);
        gpiod_set_value_cansleep(ts->reset_gpio, 0);
        msleep(150);   // 等待芯片初始化
    } else {
        gpiod_set_value_cansleep(ts->reset_gpio, 1);  // 断电时保持复位
    }
}

static void st7123_ts_power_off(void *data)
{
	st7123_ts_power(data, false);
}

static const struct st_chip_info st7123_chip_info = {
	.have_z		= true,
	.max_area	= 0xff,
	.max_fingers	= 5,
};

static int st7123_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	const struct st_chip_info *match;
	struct st7123_ts_data *ts;
	struct input_dev *input_dev;
	u16 max_x, max_y;
	int error;

	match = device_get_match_data(&client->dev);
	if (!match && id)
		match = (const void *)id->driver_data;
	if (!match) {
		dev_err(&client->dev, "unknown device model\n");
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "need I2C_FUNC_I2C\n");
		return -EIO;
	}

	if (!client->irq) {
		dev_err(&client->dev, "no IRQ?\n");
		return -EINVAL;
	}

	ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	ts->chip_info = match;

	/* allocate a buffer according to the number of registers to read */
	ts->read_buf_len = ts->chip_info->max_fingers * 8 + 4;
	ts->read_buf = devm_kzalloc(&client->dev, ts->read_buf_len, GFP_KERNEL);
	if (!ts->read_buf)
		return -ENOMEM;

	input_dev = devm_input_allocate_device(&client->dev);
	if (!input_dev)
		return -ENOMEM;

	ts->client = client;
	ts->input_dev = input_dev;

	ts->reset_gpio = devm_gpiod_get_optional(&client->dev, NULL,
						 GPIOD_OUT_HIGH);
	if (IS_ERR(ts->reset_gpio)) {
		error = PTR_ERR(ts->reset_gpio);
		dev_err(&client->dev, "Unable to request GPIO pin: %d.\n",
			error);
		return error;
	}

	st7123_ts_power(ts, true);

	error = devm_add_action_or_reset(&client->dev, st7123_ts_power_off, ts);
	if (error) {
		dev_err(&client->dev,
			"Failed to install power off action: %d\n", error);
		return error;
	}

	input_dev->name = "st7123-touchscreen";
	input_dev->id.bustype = BUS_I2C;

	/* Wait until device is ready */
	error = st7123_ts_wait_ready(ts);
	if (error)
		return error;

	if (ts->chip_info->have_z)
		input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0,
					ts->chip_info->max_area, 0, 0);

	/* Read resolution from the chip */
	error = st7123_ts_read_resolution(ts, &max_x, &max_y);
	if (error) {
		dev_err(&client->dev,
			"Failed to read resolution: %d\n", error);
		return error;
	}

	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			     0, max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			     0, max_y, 0, 0);

	touchscreen_parse_properties(input_dev, true, &ts->prop);

	error = input_mt_init_slots(input_dev, ts->chip_info->max_fingers,
				    INPUT_MT_DIRECT | INPUT_MT_TRACK |
					INPUT_MT_DROP_UNUSED);
	if (error) {
		dev_err(&client->dev, "failed to initialize MT slots\n");
		return error;
	}

	error = devm_request_threaded_irq(&client->dev, client->irq,
					  NULL, st7123_ts_irq_handler,
					  IRQF_ONESHOT,
					  client->name, ts);
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		return error;
	}

	error = input_register_device(ts->input_dev);
	if (error) {
		dev_err(&client->dev, "Unable to register %s input device\n",
			input_dev->name);
		return error;
	}

	i2c_set_clientdata(client, ts);

	return 0;
}

static int __maybe_unused st7123_ts_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st7123_ts_data *ts = i2c_get_clientdata(client);

	disable_irq(client->irq);

	if (!device_may_wakeup(&client->dev))
		st7123_ts_power(ts, false);

	return 0;
}

static int __maybe_unused st7123_ts_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st7123_ts_data *ts = i2c_get_clientdata(client);

	if (!device_may_wakeup(&client->dev))
		st7123_ts_power(ts, true);

	enable_irq(client->irq);

	return 0;
}

static SIMPLE_DEV_PM_OPS(st7123_ts_pm_ops,
			 st7123_ts_suspend, st7123_ts_resume);

static const struct i2c_device_id st7123_ts_id[] = {
	{ ST7123_TS_NAME, (unsigned long)&st7123_chip_info },
	{ }
};
MODULE_DEVICE_TABLE(i2c, st7123_ts_id);

static const struct of_device_id st7123_ts_dt_ids[] = {
	{ .compatible = "sitronix,st7123", .data = &st7123_chip_info },
	{ }
};
MODULE_DEVICE_TABLE(of, st7123_ts_dt_ids);

static struct i2c_driver st7123_ts_driver = {
	.probe		= st7123_ts_probe,
	.id_table	= st7123_ts_id,
	.driver = {
		.name	= ST7123_TS_NAME,
		.of_match_table = st7123_ts_dt_ids,
		.probe_type	= PROBE_PREFER_ASYNCHRONOUS,
		.pm	= &st7123_ts_pm_ops,
	},
};

module_i2c_driver(st7123_ts_driver);

MODULE_AUTHOR("Xenotaxis");
MODULE_DESCRIPTION("SITRONIX ST7123 Touchscreen Controller Driver");
MODULE_LICENSE("GPL v2");
