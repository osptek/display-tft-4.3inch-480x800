# Linux 参考文件说明

**English：** [`README_EN.md`](README_EN.md)

---

本目录为 **4.3 寸 480×800 MIPI 模组（ST7102）** 在 **Linux 6.1 / RK3566 LubanCat（野火）** 上的参考资料，**显示与触摸各一份，需配合使用**。

| 文件 | 是什么 |
|:--|:--|
| [`rk3566-lubancat-dsi0-vp0-4.3inch.dtsi`](rk3566-lubancat-dsi0-vp0-4.3inch.dtsi) | **设备树片段**：MIPI DSI 面板（simple-panel-dsi），含 ST7102 初始化序列与 480×800 时序 |
| [`st7123.c`](st7123.c) | **触摸驱动源码**：ST7123 I2C 电容触摸（`sitronix,st7123`），需单独编译为内核模块 |

面板显示走内核 **simple-panel-dsi**，不另附 ST7102 显示驱动 C 文件。
