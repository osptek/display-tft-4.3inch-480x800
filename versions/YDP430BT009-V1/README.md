<p align="left"><img alt="OSPTEK" src="../../images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 4.3″ TFT 显示屏 480×800（YDP430BT009-V1）</h1>

<p align="center"><b>TFT 模组 · MIPI/I2C · ST7102</b></p>

<p align="center"><a href="./README_EN.md">English</a> | 简体中文 · <a href="../../README.md">规格族索引</a></p>

<p align="center">
  <img alt="Interface: MIPI/I2C" src="https://img.shields.io/badge/Interface-MIPI%20%C2%B7%20I2C-27AE60?style=flat-square" />
  <img alt="Driver: ST7102" src="https://img.shields.io/badge/Driver-ST7102-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 4.3 寸 480×800 TFT 模组宣传图" src="./images/recommend-optimized%20%281%29.png" width="640" /></p>

## 目录

- [规格参数](#规格参数)
- [相关资料](#相关资料)
- [仓库结构](#仓库结构)
- [购买链接](#购买链接)
- [技术支持](#技术支持)

---

## 规格参数

| 项目 | 规格 |
| ---- | ---- |
| 尺寸 | 4.3 英寸 |
| 类型 | TFT |
| 分辨率 | 480×800 |
| 外形尺寸 | 66.8 x 114.4 x 3.12 mm |
| 有效显示区域 | 56.16 x 93.60 mm |
| 接口 | MIPI/I2C |
| 驱动 IC | ST7102 |
| 触摸驱动 | — |

> 完整外形尺寸、FPC 定义、供电与时序以产品规格书 / 驱动手册为准。

## 相关资料

| 分类 | 文件 |
| ---- | ---- |
| 规格文档 | [`ST7102_Datasheet_V0.1.pdf`](./datasheets/ST7102_Datasheet_V0.1.pdf) |
| 规格文档 | [`YDP430BT009-V1.pdf`](./datasheets/YDP430BT009-V1.pdf) |
| CAD 图纸 | [`YDP430BT009-V1 Model (1).pdf`](./cad/YDP430BT009-V1%20Model%20%281%29.pdf) |
| CAD 图纸 | [`YDP430BT009-V1.dwg`](./cad/YDP430BT009-V1.dwg) |
| 转接板资料 | [`转接板.png`](./adapter/%E8%BD%AC%E6%8E%A5%E6%9D%BF.png) |
| 转接板资料 | [`PCB-4.3寸TFT转接板_YDP430BT009-V1.pdf`](./adapter/PCB-4.3%E5%AF%B8TFT%E8%BD%AC%E6%8E%A5%E6%9D%BF_YDP430BT009-V1.pdf) |
| 其他资料 | [`RGB_Output_Timing_90Hz.jpg`](./misc/RGB_Output_Timing_90Hz.jpg) |

## 仓库结构

```text
display-tft-4.3inch-480x800/ # 仓库根（导航见 ../../README.md，示例工程见根目录 examples/）
└── versions/
    └── YDP430BT009-V1/        # 本料号完整资料
        ├── README.md
        ├── README_EN.md
        ├── osptek.yml
        ├── images/
        ├── datasheets/       # 产品规格书 + 驱动/触摸 IC 数据手册
        ├── cad/              # 外形/盖板 CAD
        ├── adapter/          # 转接板资料 + 转接板图片
        └── misc/             # 其他资料
```

## 购买链接

见[仓库首页](../../README.md#购买链接)。

## 技术支持

见[仓库首页](../../README.md#技术支持)。

---

<p align="center"><sub>© 2026 OSPTEK 鱼鹰光电 · 本仓库资料采用 CC BY 4.0 许可</sub></p>
