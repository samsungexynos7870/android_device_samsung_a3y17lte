# Copyright (C) 2009 The Android Open Source Project
# Copyright (c) 2011, The Linux Foundation. All rights reserved.
# Copyright (C) 2017-2019 The LineageOS Project
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

import common

def FullOTA_InstallEnd(info):
  info.script.AppendExtra('ifelse(is_mounted("/vendor"), unmount("/vendor"));')
  info.script.AppendExtra('ifelse(is_mounted("/vendor"), "", mount("ext4", "EMMC", "/dev/block/platform/13540000.dwmmc0/by-name/VENDOR", "/vendor"));')
  info.script.AppendExtra('ifelse(is_substring("A320FL", getprop("ro.bootloader")), run_program("/sbin/sh", "-c", "mv -f /vendor/etc/nfc/sec_s3nrn81_rfreg.fl /vendor/etc/nfc/sec_s3nrn81_rfreg.bin"));')
  info.script.AppendExtra('ifelse(is_substring("A320FL", getprop("ro.bootloader")), run_program("/sbin/sh", "-c", "mv -f /vendor/firmware/nfc/sec_s3nrn81_firmware.fl /vendor/firmware/nfc/sec_s3nrn81_firmware.bin"));')
  info.script.AppendExtra('ifelse(is_substring("A320FL", getprop("ro.bootloader")), run_program("/sbin/sh", "-c", "sed -i \'s:ro.multisim.simslotcount=.*:ro.multisim.simslotcount=1:\' /vendor/build.prop"));')
