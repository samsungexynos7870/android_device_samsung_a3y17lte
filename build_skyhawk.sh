#!/bin/bash

# what device are we building for?
TARGET_DEVICE="$1"

# configure some default settings for the build
Default_Settings() {

    export SHRP_PATH="device/samsung/a3y17lte"
    export SHRP_MAINTAINER=Astrako
    export SHRP_DEVICE_CODE=a3y17lte
    export SHRP_EDL_MODE=0
    export SHRP_EXTERNAL="/external_sd"
    export SHRP_INTERNAL="/sdcard"
    export SHRP_OTG="/usb_otg"
    export SHRP_FLASH=1
    export SHRP_CUSTOM_FLASHLIGHT=true
    export SHRP_FONP_1="/sys/class/camera/flash/rear_torch_flash"
    export SHRP_FLASH_MAX_BRIGHTNESS=255
    export SHRP_REC="/dev/block/platform/13540000.dwmmc0/by-name/RECOVERY"
    export SHRP_AB=true
    export SHRP_REC_TYPE=Treble
    export SHRP_DEVICE_TYPE=A_Only
    export SHRP_STATUSBAR_RIGHT_PADDING=40
    export SHRP_STATUSBAR_LEFT_PADDING=40
    export SHRP_EXPRESS=true
    export LZMA_RAMDISK_TARGETS="recovery"

}

# build the project
do_build() {
  Default_Settings

  # compile it
  . build/envsetup.sh
  
  lunch omni_"$TARGET_DEVICE"-eng
  
  mka recoveryimage -j`nproc`
}

# --- main --- #
do_build
#
