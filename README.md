# TWRP for the Samsung Galaxy A3 2017

TWRP device tree for the Samsung Galaxy A3 2017 (`a3y17lte`, SM-A320F and
related variants), based on the TWRP AOSP 12.1 manifest. The Exynos 7870 kernel
must be built from its `twrp-12.1` branch so the A3 defconfig and DTBs match.

## Build

```bash
mkdir twrp && cd twrp
repo init --depth=1 \
  -u https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git \
  -b twrp-12.1

mkdir -p .repo/local_manifests
cat > .repo/local_manifests/a3y17lte.xml <<'EOF_MANIFEST'
<?xml version="1.0" encoding="UTF-8"?>
<manifest>
    <remote fetch="https://github.com/samsungexynos7870" name="exynos7870-labs" />
    <project name="android_device_samsung_a3y17lte" path="device/samsung/a3y17lte"
             remote="exynos7870-labs" revision="twrp-12.1" />
    <project name="android_kernel_samsung_exynos7870" path="kernel/samsung/exynos7870"
             remote="exynos7870-labs" revision="twrp-12.1" />
</manifest>
EOF_MANIFEST

repo sync
source build/envsetup.sh
lunch twrp_a3y17lte-eng
mka recoveryimage
```

The recovery image is `out/target/product/a3y17lte/recovery.img`.

## DTBH image handling

Samsung Exynos 7870 boot images use a DTBH table appended after the kernel and
ramdisk. `build/tasks/dtb.mk` builds the A3 kernel DTBs and packages them with
the vendored LineageOS `dtbhtool`; `tools/mkbootimg-dt` appends the DTBH image
and checks the recovery size and LZMA ramdisk before the image is accepted.

`dtbhtool/` is based on the LineageOS `android_hardware_samsung` DTBH tooling
(branch `lineage-19.1`). The A3-specific filter and identifiers are in
`dtbhtool/include/samsung_dtbh.h`.

## Credits
2020 @Astrako
2026 samsungexynos7870

`dtbhtool/` is the DTBH tooling of the LineageOS project
(`LineageOS/android_hardware_samsung`, branch `lineage-19.1`), vendored
unchanged.

## Contact
Telegram support group: https://t.me/joinchat/D1Jk_VbieGBXOWZt2y8O7A
