# OrangeFox Recovery Project for the Samsung Galaxy A3 2017

### How to build ###

```bash
# Create dirs
$ mkdir ofox ; cd ofox

# Init repo
$ repo init --depth=1 -u https://gitlab.com/OrangeFox/Manifest.git -b fox_9.0

# Clone my local repo
$ git clone https://gitlab.com/android_samsung_universal7870/manifest/android_manifest_samsung_a3y17lte.git -b orangefox .repo/local_manifests

# Sync
$ repo sync --no-repo-verify -c --force-sync --no-clone-bundle --no-tags --optimized-fetch --prune -j`nproc`

# Build
$ mv device/samsung/a3y17lte/build_ofox.sh .
$ . build_ofox.sh a3y17lte
```
## Credits
2019 @Astrako

## Contact
Telegram support group: https://t.me/joinchat/D1Jk_VbieGBXOWZt2y8O7A

# Device info
The Samsung A3 2017 (codenamed _"a3y17lte"_) are a mid-range smartphone from Samsung.

It was announced and released in January 2017.

## Device specifications

| Device       | Samsung Galaxy A3 2017                             |
| -----------: | :----------------------------------------------    |
| SoC          | Samsung Exynos 7870                                |
| CPU          | Octa-core, 1586 MHz, ARM Cortex-A53, 64-bit, 14 nm |
| GPU          | Mali-T830                                          |
| Memory       | 2GB (LPDDR3X)                                      |
| Shipped Android version | 6.0.1                                   |
| Storage      | 16GB eMMC 5.1 flash storage                        |
| Battery      | Non-removable Li-Po 2350 mAh                       |
| Dimensions   | 135.4 x 66.2 x 7.9 mm                              |
| Display      | 1280 x 720 (16:9), 4.7  inch                       |
| Rear camera 1 | 13MP, f/1.9                                       |
| Front camera  | 8MP                                               |

## Device picture

![Samsung A3 2017](https://www.maxmovil.com/media/catalog/product/cache/1/thumbnail/600x/17f82f742ffe127f42dca9de82fb58b1/c/o/comprar_samsung_galaxy_a3_2017_negro_2.jpg)
