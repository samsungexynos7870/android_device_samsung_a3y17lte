#
# Copyright (C) 2019 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DEVICE_PATH := device/samsung/a3y17lte

# Audio

# audio type guard
TARGET_BOARD_HAS_TFA_SEC_AUDIO_HAL := false
TARGET_BOARD_HAS_SEC_AUDIO_HAL := false

ifeq ($(TARGET_BOARD_HAS_TFA_SEC_AUDIO_HAL),true)
TARGET_BOARD_HAS_TFA_AMP := true
endif

# sec audio hal
TARGET_BOARD_HAS_EXYNOS7870_SEC_AUDIOHAL := false

TARGET_AUDIOHAL_VARIANT := samsung-exynos7870

# Audiohal
BOARD_USE_SPKAMP := true

# TFA
TARGET_BOARD_TFA_MODEL := 9896

# libhwjpeg
TARGET_USES_EXYNOS7870_LIBHWJPEG_O := true

# FM Radio
TARGET_BOARD_HAS_SILAB_FM := true

# Assert
TARGET_OTA_ASSERT_DEVICE := a3y17lte,a3y17ltexc,a3y17ltexx,a3y17ltelk

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth

# Kernel
TARGET_KERNEL_CONFIG := exynos7870-a3y17lte_defconfig

# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/configs/manifest.xml

# Init
TARGET_INIT_VENDOR_LIB := //$(DEVICE_PATH):libinit_a3y17lte
TARGET_RECOVERY_DEVICE_MODULES := libinit_a3y17lte

# Build fingerprint
BUILD_FINGERPRINT := "samsung/a7y17lteskt/a7y17lteskt:9/PPR1.180610.011/A720SKSU5CUJ2:user/release-keys"
PRIVATE_BUILD_DESC := "a7y17lteskt-user 9 PPR1.180610.011 A720SKSU5CUJ2 release-keys"

# Releasetools
TARGET_RELEASETOOLS_EXTENSIONS := $(DEVICE_PATH)/releasetools

# CAMERA
BOARD_BACK_CAMERA_ROTATION := 90
BOARD_FRONT_CAMERA_ROTATION := 270
BOARD_BACK_CAMERA_SENSOR := SENSOR_NAME_IMX258
BOARD_FRONT_CAMERA_SENSOR := SENSOR_NAME_IMX219
BOARD_SECURE_CAMERA_SENSOR := SENSOR_NAME_NOTHING
BOARD_SECURE_CAMERA_ROTATION := 0

BOARD_SECURE_CAMERA_SUPPORT := false
BOARD_CAMERA_DUAL_SUPPORT := false
BOARD_CAMERA2_API_SUPPORT := true
BOARD_CAMERA_SAMSUNG_TN_FEATURE := true
BOARD_CAMERA_HAL3_FEATURE := true

#ANT+
#Broadcom : BOARD_ANT_WIRELESS_DEVICE -> BCM
#Qualcomm : BOARD_ANT_WIRELESS_DEVICE -> "vfs-prerelease"
BOARD_HAVE_ANT_WIRELESS := true
BOARD_ANT_WIRELESS_DEVICE := "vfs-prerelease"

# Wifi
BOARD_HAS_QCOM_WLAN := true
BOARD_WLAN_DEVICE := qcwcn
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_qcwcn
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_qcwcn
WIFI_DRIVER_FW_PATH_AP := "ap"
WIFI_DRIVER_FW_PATH_STA := "sta"
WIFI_DRIVER_FW_PATH_P2P := "p2p"
WPA_SUPPLICANT_VERSION := VER_0_8_X
WPA_SUPPLICANT_USE_HIDL := true
WIFI_AVOID_IFACE_RESET_MAC_CHANGE := true
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true

# inherit from common
-include device/samsung/universal7870-common/BoardConfigCommon.mk

# inherit from the proprietary version
-include vendor/samsung/a3y17lte/BoardConfigVendor.mk
