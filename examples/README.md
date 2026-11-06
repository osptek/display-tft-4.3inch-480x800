# 示例工程 / Sample Projects

本目录收录本规格屏幕的示例工程，每个子文件夹是一个独立工程，打开对应文件夹按其中的 README 编译即可。
这些工程是参考代码，引脚、背光、触摸等配置请按你的硬件修改；选用前请对照仓库首页各型号的驱动 IC 和接口确认。

This folder holds sample projects for this display spec. Each sub-folder is a standalone project — open it and follow its own README to build.
They are reference code; adjust pins, backlight, touch and other settings to your hardware. Before choosing one, check the driver IC and interface against the models listed on the repository home page.

| 例程 / Project | 平台 / Platform | 驱动 IC / Driver IC | 接口 / Interface | 说明 / Description |
|---|---|---|---|---|
| [`esp32p4-idf-st7102-mipi-lvgl-common-demo-te`](./esp32p4-idf-st7102-mipi-lvgl-common-demo-te/) | ESP32-P4 · ESP-IDF | ST7102 | MIPI | LVGL 综合演示、TE 防撕裂 / LVGL demo, TE tear-free |
| [`esp32p4-idf5-st7102-mipi-dsi-sc2336-mipi-csi-video-lcd-display`](./esp32p4-idf5-st7102-mipi-dsi-sc2336-mipi-csi-video-lcd-display/) | ESP32-P4 · ESP-IDF 5 | ST7102 | MIPI | 摄像头实时显示 / camera live view |
| [`esp32p4-idf5-st7102-mipi-lvgl-port-lvgl9`](./esp32p4-idf5-st7102-mipi-lvgl-port-lvgl9/) | ESP32-P4 · ESP-IDF 5 | ST7102 | MIPI | LVGL 9 / LVGL 9 |
| [`esp32p4-idf6-st7102-mipi-dsi-sc2336-mipi-csi-video-lcd-display`](./esp32p4-idf6-st7102-mipi-dsi-sc2336-mipi-csi-video-lcd-display/) | ESP32-P4 · ESP-IDF 6 | ST7102 | MIPI | 摄像头实时显示 / camera live view |
| [`esp32p4-idf6-st7102-mipi-lvgl-port-lvgl9-ppa`](./esp32p4-idf6-st7102-mipi-lvgl-port-lvgl9-ppa/) | ESP32-P4 · ESP-IDF 6 | ST7102 | MIPI | LVGL 9、PPA 硬件加速 / LVGL 9, PPA acceleration |
| [`esp32p4-tft-4.3in-480x800-st7102-rgb-bringup`](./esp32p4-tft-4.3in-480x800-st7102-rgb-bringup/) | ESP32-P4 · ESP-IDF | ST7102 | RGB | 点亮示例 / bring-up example |
| [`rk3566-linux-st7102-mipi`](./rk3566-linux-st7102-mipi/) | RK3566 (Linux) | ST7102 + ST7123 | MIPI | Linux 设备树与触摸驱动参考 / Linux device tree and touch driver reference |
| [`rpi5-lvgl-st7102-st7123-480x800`](./rpi5-lvgl-st7102-st7123-480x800/) | Raspberry Pi 5 | ST7102 + ST7123 | — | LVGL、触摸 / LVGL, touch |
| [`rpi5-panel-st7102-480x800`](./rpi5-panel-st7102-480x800/) | Raspberry Pi 5 | ST7102 | — | 面板驱动 / panel driver |
| [`rpi5-panel-st7102-st7123-480x800`](./rpi5-panel-st7102-st7123-480x800/) | Raspberry Pi 5 | ST7102 + ST7123 | — | 面板驱动、触摸 / panel driver, touch |

如有问题请联系技术支持。 / Contact technical support if you have questions.
