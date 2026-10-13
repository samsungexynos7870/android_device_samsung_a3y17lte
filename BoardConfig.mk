#
# Copyright 2017 The Android Open Source Project
#
# Copyright (C) 2018-2020 OrangeFox Recovery Project
# Copyright (C) 2026 samsungexynos7870
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

DEVICE_PATH := device/samsung/a3y17lte

# Architecture
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := cortex-a53
TARGET_CPU_VARIANT_RUNTIME := cortex-a53

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a53
TARGET_2ND_CPU_VARIANT_RUNTIME := cortex-a53

TARGET_USES_64_BIT_BINDER := true

# Bootloader
BOARD_VENDOR := samsung
TARGET_BOARD_PLATFORM := exynos5
TARGET_SOC := exynos7870
TARGET_BOOTLOADER_BOARD_NAME := universal7870
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

# Crypto (FBE v1)
TW_INCLUDE_CRYPTO := true
TW_INCLUDE_CRYPTO_FBE := true
TW_INCLUDE_FBE := true
TW_USE_FSCRYPT_POLICY := 1
TW_INCLUDE_LIBRESETPROP := true
TW_INCLUDE_RESETPROP := true

# Kernel
TARGET_KERNEL_ARCH := arm64
TARGET_KERNEL_HEADER_ARCH := arm64
TARGET_KERNEL_CONFIG := exynos7870-a3y17lte_defconfig
TARGET_KERNEL_SOURCE := kernel/samsung/exynos7870
# GCC 4.9 for the 3.18 kernel, clang for the host tools (dtc, kconfig)
TARGET_KERNEL_CLANG_COMPILE := false
TARGET_KERNEL_ADDITIONAL_FLAGS := HOSTCFLAGS="-fuse-ld=lld -Wno-unused-command-line-argument"

# Recovery is 77,824 512-byte blocks in the A3 stock PIT (39,845,888 bytes).
# Setting this also makes the build system set BUILDING_RECOVERY_IMAGE.
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 39845888

# Extracted with libbootimg; --dt is the Samsung DTBH image packed by
# build/tasks/dtb.mk (the bootloader picks the dtb by chip / hw_rev).
BOARD_MKBOOTIMG_ARGS := --kernel_offset 0x00008000 --ramdisk_offset 0x01000000 --tags_offset 0x00000100 --dt $(OUT_DIR)/target/product/a3y17lte/obj/KERNEL_OBJ/arch/$(TARGET_ARCH)/boot/dtb.img
BOARD_MKBOOTIMG_ARGS += --max-image-size $(BOARD_RECOVERYIMAGE_PARTITION_SIZE) --expect-ramdisk-compression lzma
# Recursively expanded on purpose: BoardConfig.mk is read before
# HOST_OUT_EXECUTABLES exists.
BOARD_CUSTOM_MKBOOTIMG = $(DEVICE_PATH)/tools/mkbootimg-dt $(HOST_OUT_EXECUTABLES)/mkbootimg
BOARD_KERNEL_BASE := 0x10000000
BOARD_KERNEL_PAGESIZE := 2048
BOARD_KERNEL_IMAGE_NAME := Image

# File systems
BOARD_HAS_LARGE_FILESYSTEM := true
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true

# Ramdisk compression (CONFIG_RD_LZMA, and ~25% smaller than gzip)
BOARD_RAMDISK_USE_LZMA := true

TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/recovery/root/system/etc/recovery.fstab

# Recovery
TARGET_RECOVERY_PIXEL_FORMAT := "ABGR_8888"

# System as root
BOARD_BUILD_SYSTEM_ROOT_IMAGE := true
AB_OTA_UPDATER := false

# TWRP specific build flags
TARGET_OTA_ASSERT_DEVICE := a3y17lte,a3y17ltexc,a3y17ltexx,a3y17ltelk
RECOVERY_SDCARD_ON_DATA := true
TW_BRIGHTNESS_PATH := "/sys/class/backlight/panel/brightness"
TW_EXCLUDE_DEFAULT_USB_INIT := true
# Keep optional fonts and translations out to leave space in the recovery image.
TW_EXTRA_LANGUAGES := false
TW_DEFAULT_LANGUAGE := en
TW_INCLUDE_NTFS_3G := true
TW_INPUT_BLACKLIST := "hbtp_vm"
TW_MAX_BRIGHTNESS := 255
TW_DEFAULT_BRIGHTNESS := 162
TW_THEME := portrait_hdpi
TW_SCREEN_BLANK_ON_BOOT := true
TWRP_INCLUDE_LOGCAT := true
TARGET_USES_LOGD := true
TW_USE_TOOLBOX := true
TW_NO_REBOOT_BOOTLOADER := true

# Vendor separation
TARGET_COPY_OUT_VENDOR := vendor
