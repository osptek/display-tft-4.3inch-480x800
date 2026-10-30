<p align="left"><img alt="OSPTEK" src="../../images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 4.3″ TFT Display 480×800 (YDP430BT009-V1)</h1>

<p align="center"><b>TFT module · MIPI/I2C · ST7102</b></p>

<p align="center"><a href="./README.md">简体中文</a> | English · <a href="../../README_EN.md">Family index</a></p>

<p align="center">
  <img alt="Interface: MIPI/I2C" src="https://img.shields.io/badge/Interface-MIPI%20%C2%B7%20I2C-27AE60?style=flat-square" />
  <img alt="Driver: ST7102" src="https://img.shields.io/badge/Driver-ST7102-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 4.3&quot; 480×800 TFT module product image" src="./images/recommend-optimized%20%281%29.png" width="640" /></p>

## Contents

- [Specifications](#specifications)
- [Resources](#resources)
- [Repository layout](#repository-layout)
- [Buy](#buy)
- [Support](#support)

---

## Specifications

| Item | Spec |
| ---- | ---- |
| Size | 4.3 inch |
| Type | TFT |
| Resolution | 480×800 |
| Outline | 66.8 x 114.4 x 3.12 mm |
| Active area | 56.16 x 93.60 mm |
| Interface | MIPI/I2C |
| Driver IC | ST7102 |
| Touch IC | — |

> Full outline, FPC definition, power, and timing follow the product datasheet / driver IC datasheet.

## Resources

| Category | File |
| ---- | ---- |
| Datasheet | [`ST7102_Datasheet_V0.1.pdf`](./datasheets/ST7102_Datasheet_V0.1.pdf) |
| Datasheet | [`YDP430BT009-V1.pdf`](./datasheets/YDP430BT009-V1.pdf) |
| CAD file | [`YDP430BT009-V1 Model (1).pdf`](./cad/YDP430BT009-V1%20Model%20%281%29.pdf) |
| CAD file | [`YDP430BT009-V1.dwg`](./cad/YDP430BT009-V1.dwg) |
| Adapter materials | [`转接板.png`](./adapter/%E8%BD%AC%E6%8E%A5%E6%9D%BF.png) |
| Adapter materials | [`PCB-4.3寸TFT转接板_YDP430BT009-V1.pdf`](./adapter/PCB-4.3%E5%AF%B8TFT%E8%BD%AC%E6%8E%A5%E6%9D%BF_YDP430BT009-V1.pdf) |
| Other | [`RGB_Output_Timing_90Hz.jpg`](./misc/RGB_Output_Timing_90Hz.jpg) |

## Repository layout

```text
display-tft-4.3inch-480x800/ # repo root (nav: ../../README_EN.md; sample projects under root examples/)
└── versions/
    └── YDP430BT009-V1/        # full materials for this part number
        ├── README.md
        ├── README_EN.md
        ├── osptek.yml
        ├── images/
        ├── datasheets/       # product datasheet + driver/touch IC datasheets
        ├── cad/              # outline/cover CAD
        ├── adapter/          # adapter board materials + photos
        └── misc/             # other materials
```

## Buy

See the [repo home page](../../README_EN.md#where-to-buy).

## Support

See the [repo home page](../../README_EN.md#support).

---

<p align="center"><sub>© 2026 OSPTEK · Materials in this repository are licensed under CC BY 4.0</sub></p>
