# Linux reference files

**简体中文：** [`README.md`](README.md)

---

Reference files for the **4.3″ 480×800 MIPI module (ST7102)** on **Linux 6.1 / RK3566 LubanCat**. **Display and touch are separate; both are required.**

| File | What it is |
|:--|:--|
| [`rk3566-lubancat-dsi0-vp0-4.3inch.dtsi`](rk3566-lubancat-dsi0-vp0-4.3inch.dtsi) | **Device tree fragment**: MIPI DSI panel (simple-panel-dsi), ST7102 init sequence, 480×800 timings |
| [`st7123.c`](st7123.c) | **Touch driver source**: ST7123 I2C touch (`sitronix,st7123`), build as an out-of-tree kernel module |

Panel display uses in-kernel **simple-panel-dsi**; no separate ST7102 display driver `.c` file is included.
