#
#	This file is part of the OrangeFox Recovery Project
# 	Copyright (C) 2019-2020 The OrangeFox Recovery Project
#	
#	OrangeFox is free software: you can redistribute it and/or modifyblock
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	any later version.
#
#	OrangeFox is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
# 	This software is released under GPL version 3 or any later version.
#	See <http://www.gnu.org/licenses/>.
# 	
# 	Please maintain this if you use this script or any part of it
#

FDEVICE="a3y17lte"

# a3y17lte specific
export TARGET_DEVICE_ALT="a3y17lte, a3y17ltexc, a3y17ltexx, a3y17ltelk"

# R11
export FOX_R11="1"
export FOX_BUILD_TYPE="Stable"
export FOX_VERSION="R11.0_1"
export FOX_ADVANCED_SECURITY="1"
export OF_USE_TWRP_SAR_DETECT="1"
export OF_DISABLE_MIUI_OTA_BY_DEFAULT="1"
export OF_QUICK_BACKUP_LIST="/boot;/data;/system_image;/vendor_image;"

# Common
export TARGET_ARCH="arm64"
export ALLOW_MISSING_DEPENDENCIES=true
export LC_ALL="C"
export FOX_RECOVERY_INSTALL_PARTITION="/dev/block/platform/13540000.dwmmc0/by-name/RECOVERY"
export FOX_RECOVERY_SYSTEM_PARTITION="/dev/block/platform/13540000.dwmmc0/by-name/SYSTEM"
export FOX_RECOVERY_VENDOR_PARTITION="/dev/block/platform/13540000.dwmmc0/by-name/VENDOR"

export FOX_ASH_IS_BASH="1"
export FOX_REPLACE_BUSYBOX_PS="1"
export FOX_USE_BASH_SHELL="1"
export FOX_USE_LZMA_COMPRESSION="1"
export FOX_USE_NANO_EDITOR="1"
export FOX_USE_TAR_BINARY="1"
export FOX_USE_TWRP_RECOVERY_IMAGE_BUILDER="1"
export FOX_USE_ZIP_BINARY="1"
export LZMA_RAMDISK_TARGETS="recovery"
export OF_AB_DEVICE="0"
export OF_DISABLE_MIUI_SPECIFIC_FEATURES="1"
export OF_DONT_PATCH_ENCRYPTED_DEVICE="1"
export OF_FLASHLIGHT_ENABLE="0"
export OF_KEEP_FORCED_ENCRYPTION="1"
export OF_MAINTAINER="Astrako"
export OF_NO_TREBLE_COMPATIBILITY_CHECK="1"
export OF_SCREEN_H="1920"
export OF_SKIP_MULTIUSER_FOLDERS_BACKUP="1"
export OF_TWRP_COMPATIBILITY_MODE="1"
export OF_USE_MAGISKBOOT="1"
export OF_USE_MAGISKBOOT_FOR_ALL_PATCHES="1"
export OF_USE_NEW_MAGISKBOOT="1"
export PLATFORM_SECURITY_PATCH="2099-12-31"
export PLATFORM_VERSION="16.1.0"
export TW_DEFAULT_LANGUAGE="en"

# use system (ROM) fingerprint where available
export OF_USE_SYSTEM_FINGERPRINT="1"

# OTA for custom ROMs
export OF_SUPPORT_ALL_BLOCK_OTA_UPDATES="1"
export OF_FIX_OTA_UPDATE_MANUAL_FLASH_ERROR="1"

add_lunch_combo omni_"$FDEVICE"-eng
