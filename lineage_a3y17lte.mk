#
# Copyright (C) 2019 The lineageOS Project
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
#

# Inherit from those products. Most specific first.
# $(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_m.mk)
# this is save to assume since devices like a6lte lauched with o
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_o.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from a3y17lte device
$(call inherit-product, device/samsung/a3y17lte/device.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Device identifier. This must come after all inclusions

PRODUCT_NAME := lineage_a3y17lte
PRODUCT_DEVICE := a3y17lte
PRODUCT_BRAND := samsung
PRODUCT_MODEL := SM-A320FL
PRODUCT_MANUFACTURER := samsung

BUILD_FINGERPRINT := "samsung/a3y17ltexc/a3y17lte:8.0.0/R16NW/A320FLXXS5CSL5:user/release-keys"

PRODUCT_BUILD_PROP_OVERRIDES += \
    DeviceProduct=a3y17ltexc \
    BuildDesc="a3y17ltexc-user 8.0.0 R16NW A320FLXXS5CSL5 release-keys"

PRODUCT_PROPERTY_OVERRIDES += \
    ro.build.PDA=A320FLXXS5CSL5 \
    ro.build.fingerprint=samsung/a3y17ltexc/a3y17lte:8.0.0/R16NW/A320FLXXS5CSL5:user/release-keys

TARGET_SUPPORTS_BLUR := false
TARGET_FACE_UNLOCK_SUPPORTED := false

TARGET_VENDOR := samsung
PRODUCT_GMS_CLIENTID_BASE := android-samsung

TARGET_BOOT_ANIMATION_RES := 720
TARGET_GAPPS_ARCH := arm64

# Updater: use the Exynos7870-labs OTA channel instead of the official
# LineageOS servers. The stock Updater prefers the lineage.updater.uri
# property over its built-in updater_server_url resource and replaces
# {device} with ro.lineage.device, so it fetches:
#   https://raw.githubusercontent.com/Exynos7870-labs/lineage_release/lineage-22.2/a3y17lte.json
# The JSON (Updater v1 format) points at the OTA zip hosted in the
# GitHub Releases of the same repository. OTA only works between
# builds signed with the same (Exynos7870-labs) keys.
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    lineage.updater.uri=https://raw.githubusercontent.com/Exynos7870-labs/lineage_release/22.2/{device}.json
