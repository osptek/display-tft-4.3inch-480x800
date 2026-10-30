#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LCD_H_RES                       480
#define LCD_V_RES                       800

/*
 * ESP32-P4 mini-board pinout from docs/转接板原理图.png.
 * ST7102 commands use 9-bit 3-wire SPI: SDA carries the D/C bit followed by
 * the eight command/parameter bits.
 */
#define LCD_SPI_RST                     GPIO_NUM_24
#define LCD_SPI_SDA                     GPIO_NUM_25
#define LCD_SPI_SCLK                    GPIO_NUM_26
#define LCD_SPI_CS                      GPIO_NUM_8

#define LCD_PCLK                        GPIO_NUM_27
#define LCD_DE                          GPIO_NUM_28
#define LCD_VSYNC                       GPIO_NUM_29
#define LCD_HSYNC                       GPIO_NUM_30

#define LCD_DATA0                       GPIO_NUM_31
#define LCD_DATA1                       GPIO_NUM_33
#define LCD_DATA2                       GPIO_NUM_34
#define LCD_DATA3                       GPIO_NUM_36
#define LCD_DATA4                       GPIO_NUM_45
#define LCD_DATA5                       GPIO_NUM_46
#define LCD_DATA6                       GPIO_NUM_47
#define LCD_DATA7                       GPIO_NUM_48
#define LCD_DATA8                       GPIO_NUM_49
#define LCD_DATA9                       GPIO_NUM_50
#define LCD_DATA10                      GPIO_NUM_51
#define LCD_DATA11                      GPIO_NUM_52
#define LCD_DATA12                      GPIO_NUM_0
#define LCD_DATA13                      GPIO_NUM_1
#define LCD_DATA14                      GPIO_NUM_2
#define LCD_DATA15                      GPIO_NUM_3

/* Timing copied from the reference ST7102 480x800 MIPI video mode. */
#define LCD_PIXEL_CLOCK_HZ              (37800000)
#define LCD_HSYNC_PULSE_WIDTH            2
#define LCD_HSYNC_BACK_PORCH             40
#define LCD_HSYNC_FRONT_PORCH            40
#define LCD_VSYNC_PULSE_WIDTH            2
#define LCD_VSYNC_BACK_PORCH             10
#define LCD_VSYNC_FRONT_PORCH            210

typedef struct {
    uint8_t command;
    const uint8_t *data;
    size_t data_size;
    uint32_t delay_ms;
} lcd_init_cmd_t;

static const char *TAG = "st7102_rgb";
static esp_lcd_panel_handle_t s_panel;
static esp_ldo_channel_handle_t s_ldo_channels[4];

/*
 * Panel-specific ST7102 register sequence from
 * reference/p4-dev-board/.../lcd_init_mipi_dsi.c.
 * DCS commands are independent of the pixel transport and are sent here over
 * the RGB panel's command SPI interface.
 */
static const lcd_init_cmd_t s_st7102_init_commands[] = {
    {0xF0, (uint8_t[]){0x00}, 1, 0},
    {0x36, (uint8_t[]){0x00}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},
    {0x99, (uint8_t[]){0x71, 0x02, 0xA2}, 3, 0},
    {0x99, (uint8_t[]){0x71, 0x02, 0xA3}, 3, 0},
    {0x99, (uint8_t[]){0x71, 0x02, 0xA4}, 3, 0},
    {0xB0, (uint8_t[]){0x22, 0x57, 0x1E, 0x61, 0x2F, 0x57, 0x61}, 7, 0},
    {0xB7, (uint8_t[]){0x64, 0x64}, 2, 0},
    {0xBF, (uint8_t[]){0xB4, 0xB4}, 2, 0},
    {0xC8, (uint8_t[]){
        0x00, 0x00, 0x13, 0x24, 0x44, 0x00, 0x74, 0x03, 0xB8, 0x04,
        0x11, 0x16, 0x08, 0x86, 0x04, 0x21, 0xD3, 0x02, 0x10, 0x0F,
        0x22, 0x4D, 0x0E, 0x90, 0x09, 0x32, 0xF0, 0x0B, 0x40, 0x0E,
        0xF3, 0x7D, 0x0E, 0xA9, 0xBF, 0x03, 0xC4
    }, 37, 0},
    {0xC9, (uint8_t[]){
        0x00, 0x00, 0x13, 0x24, 0x44, 0x00, 0x74, 0x03, 0xB8, 0x04,
        0x11, 0x16, 0x08, 0x86, 0x04, 0x21, 0xD3, 0x02, 0x10, 0x0F,
        0x22, 0x4D, 0x0E, 0x90, 0x09, 0x32, 0xF0, 0x0B, 0x40, 0x0E,
        0xF3, 0x7D, 0x0E, 0xA9, 0xBF, 0x03, 0xC4
    }, 37, 0},
    {0xD7, (uint8_t[]){0x10, 0x0C, 0x36, 0x19, 0x90, 0x90}, 6, 0},
    {0xA3, (uint8_t[]){
        0x51, 0x03, 0x80, 0xCF, 0x44, 0x00, 0x00, 0x00, 0x00, 0x04,
        0x78, 0x78, 0x00, 0x1A, 0x00, 0x45, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x46, 0x00, 0x00, 0x02, 0x20, 0x52, 0x00, 0x05, 0x00,
        0x00, 0xFF
    }, 32, 0},
    {0xA6, (uint8_t[]){
        0x02, 0x00, 0x24, 0x55, 0x35, 0x00, 0x38, 0x00, 0x78, 0x78,
        0x00, 0x24, 0x55, 0x36, 0x00, 0x37, 0x00, 0x78, 0x78, 0x02,
        0xAC, 0x51, 0x3A, 0x00, 0x00, 0x00, 0x78, 0x78, 0x03, 0xAC,
        0x21, 0x00, 0x04, 0x00, 0x00, 0x78, 0x78, 0x3E, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x00
    }, 44, 0},
    {0xA7, (uint8_t[]){
        0x19, 0x19, 0x00, 0x64, 0x40, 0x07, 0x16, 0x40, 0x00, 0x04,
        0x03, 0x78, 0x78, 0x00, 0x64, 0x40, 0x25, 0x34, 0x00, 0x00,
        0x02, 0x01, 0x78, 0x78, 0x00, 0x64, 0x40, 0x4B, 0x5A, 0x00,
        0x00, 0x02, 0x01, 0x78, 0x78, 0x00, 0x24, 0x40, 0x69, 0x78,
        0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x00, 0x44
    }, 48, 0},
    {0xAC, (uint8_t[]){
        0x08, 0x0A, 0x11, 0x00, 0x13, 0x03, 0x1B, 0x18, 0x06, 0x1A,
        0x19, 0x1B, 0x1B, 0x1B, 0x18, 0x1B, 0x09, 0x0B, 0x10, 0x02,
        0x12, 0x01, 0x1B, 0x18, 0x06, 0x1A, 0x19, 0x1B, 0x1B, 0x1B,
        0x18, 0x1B, 0xFF, 0x67, 0xFF, 0x67, 0x00
    }, 37, 0},
    {0xAD, (uint8_t[]){0xCC, 0x40, 0x46, 0x11, 0x04, 0x78, 0x78}, 7, 0},
    {0xE8, (uint8_t[]){
        0x30, 0x07, 0x00, 0x94, 0x94, 0x9C, 0x00, 0xE2, 0x04, 0x00,
        0x00, 0x00, 0x00, 0xEF
    }, 14, 0},
    {0xE7, (uint8_t[]){
        0x8B, 0x3C, 0x00, 0x0C, 0xF0, 0x5D, 0x00, 0x5D, 0x00, 0x5D,
        0x00, 0x5D, 0x00, 0xFF, 0x00, 0x08, 0x7B, 0x00, 0x00, 0xC8,
        0x6A, 0x5A, 0x08, 0x1A, 0x3C, 0x00, 0x81, 0x01, 0xCC, 0x01,
        0x7F, 0xF0, 0x22
    }, 33, 0},
    {0x11, NULL, 0, 600},
    {0x29, NULL, 0, 120},
    {0x35, (uint8_t[]){0x00}, 1, 0},
};

static void enable_p4_ldo_domains(void)
{
    static const struct {
        int channel;
        int voltage_mv;
    } ldo_config[] = {
        {1, 3300},
        {2, 1800},
        {3, 3300},
        {4, 3300},
    };

    for (size_t i = 0; i < sizeof(ldo_config) / sizeof(ldo_config[0]); i++) {
        const esp_ldo_channel_config_t config = {
            .chan_id = ldo_config[i].channel,
            .voltage_mv = ldo_config[i].voltage_mv,
        };
        ESP_ERROR_CHECK(esp_ldo_acquire_channel(&config, &s_ldo_channels[i]));
    }
}

static void lcd_spi_write_9bit(bool is_data, uint8_t value)
{
    const uint16_t word = ((uint16_t)is_data << 8) | value;

    for (int bit = 8; bit >= 0; bit--) {
        gpio_set_level(LCD_SPI_SDA, (word >> bit) & 1);
        esp_rom_delay_us(1);
        gpio_set_level(LCD_SPI_SCLK, 1);
        esp_rom_delay_us(1);
        gpio_set_level(LCD_SPI_SCLK, 0);
    }
}

static void lcd_spi_write_command(uint8_t command,
                                  const uint8_t *data,
                                  size_t data_size)
{
    gpio_set_level(LCD_SPI_CS, 0);
    esp_rom_delay_us(1);
    lcd_spi_write_9bit(false, command);
    for (size_t i = 0; i < data_size; i++) {
        lcd_spi_write_9bit(true, data[i]);
    }
    esp_rom_delay_us(1);
    gpio_set_level(LCD_SPI_CS, 1);
}

static esp_err_t st7102_command_init(void)
{
    const gpio_config_t spi_gpio_config = {
        .pin_bit_mask = (1ULL << LCD_SPI_RST) |
                        (1ULL << LCD_SPI_SDA) |
                        (1ULL << LCD_SPI_SCLK) |
                        (1ULL << LCD_SPI_CS),
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&spi_gpio_config), TAG,
                        "configure 3-wire SPI GPIOs");
    ESP_RETURN_ON_ERROR(gpio_set_level(LCD_SPI_CS, 1), TAG, "set CS high");
    ESP_RETURN_ON_ERROR(gpio_set_level(LCD_SPI_SCLK, 0), TAG, "set SCLK low");
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_RETURN_ON_ERROR(gpio_set_level(LCD_SPI_RST, 1), TAG, "set reset high");
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_RETURN_ON_ERROR(gpio_set_level(LCD_SPI_RST, 0), TAG, "set reset low");
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_RETURN_ON_ERROR(gpio_set_level(LCD_SPI_RST, 1), TAG, "release reset");
    vTaskDelay(pdMS_TO_TICKS(600));

    for (size_t i = 0;
         i < sizeof(s_st7102_init_commands) / sizeof(s_st7102_init_commands[0]);
         i++) {
        const lcd_init_cmd_t *init = &s_st7102_init_commands[i];
        lcd_spi_write_command(init->command, init->data, init->data_size);
        if (init->delay_ms > 0) {
            vTaskDelay(pdMS_TO_TICKS(init->delay_ms));
        }
    }

    return ESP_OK;
}

static void fill_test_pattern(uint16_t *frame_buffer)
{
    static const uint16_t colors[] = {
        0xF800, /* red */
        0xFFE0, /* yellow */
        0x07E0, /* green */
        0x07FF, /* cyan */
        0x001F, /* blue */
        0xF81F, /* magenta */
        0xFFFF, /* white */
        0x0000, /* black */
    };

    for (int y = 0; y < LCD_V_RES; y++) {
        const uint16_t color = colors[(y * 8) / LCD_V_RES];
        for (int x = 0; x < LCD_H_RES; x++) {
            frame_buffer[(size_t)y * LCD_H_RES + x] = color;
        }
    }
}

static esp_err_t rgb_panel_init(void)
{
    static const gpio_num_t rgb_gpios[] = {
        LCD_PCLK, LCD_DE, LCD_VSYNC, LCD_HSYNC,
        LCD_DATA0, LCD_DATA1, LCD_DATA2, LCD_DATA3,
        LCD_DATA4, LCD_DATA5, LCD_DATA6, LCD_DATA7,
        LCD_DATA8, LCD_DATA9, LCD_DATA10, LCD_DATA11,
        LCD_DATA12, LCD_DATA13, LCD_DATA14, LCD_DATA15,
    };
    const esp_lcd_rgb_panel_config_t config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = LCD_PIXEL_CLOCK_HZ,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            .hsync_pulse_width = LCD_HSYNC_PULSE_WIDTH,
            .hsync_back_porch = LCD_HSYNC_BACK_PORCH,
            .hsync_front_porch = LCD_HSYNC_FRONT_PORCH,
            .vsync_pulse_width = LCD_VSYNC_PULSE_WIDTH,
            .vsync_back_porch = LCD_VSYNC_BACK_PORCH,
            .vsync_front_porch = LCD_VSYNC_FRONT_PORCH,
            .flags.pclk_active_neg = true,
        },
        .data_width = 16,
        .in_color_format = LCD_COLOR_FMT_RGB565,
        .out_color_format = LCD_COLOR_FMT_RGB565,
        .num_fbs = 1,
        .bounce_buffer_size_px = LCD_H_RES * 20,
        .dma_burst_size = 128,
        .hsync_gpio_num = LCD_HSYNC,
        .vsync_gpio_num = LCD_VSYNC,
        .de_gpio_num = LCD_DE,
        .pclk_gpio_num = LCD_PCLK,
        .disp_gpio_num = GPIO_NUM_NC,
        .data_gpio_nums = {
            LCD_DATA0, LCD_DATA1, LCD_DATA2, LCD_DATA3,
            LCD_DATA4, LCD_DATA5, LCD_DATA6, LCD_DATA7,
            LCD_DATA8, LCD_DATA9, LCD_DATA10, LCD_DATA11,
            LCD_DATA12, LCD_DATA13, LCD_DATA14, LCD_DATA15,
        },
        .flags.fb_in_psram = true,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_rgb_panel(&config, &s_panel), TAG,
                        "create RGB panel");

    for (size_t i = 0; i < sizeof(rgb_gpios) / sizeof(rgb_gpios[0]); i++) {
        ESP_RETURN_ON_ERROR(gpio_set_drive_capability(rgb_gpios[i],
                                                       GPIO_DRIVE_CAP_3),
                            TAG, "set RGB GPIO drive capability");
    }

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel), TAG, "reset RGB panel");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel), TAG, "initialize RGB panel");

    uint16_t *frame_buffer = NULL;
    ESP_RETURN_ON_ERROR(
        esp_lcd_rgb_panel_get_frame_buffer(s_panel, 1, (void **)&frame_buffer),
        TAG, "get RGB frame buffer");
    fill_test_pattern(frame_buffer);
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Start ST7102 480x800 RGB display bring-up");
    enable_p4_ldo_domains();
    ESP_ERROR_CHECK(st7102_command_init());
    ESP_ERROR_CHECK(rgb_panel_init());
    ESP_LOGI(TAG, "LCD initialized; showing RGB565 color bars");
}
