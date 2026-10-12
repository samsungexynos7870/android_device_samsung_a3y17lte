#
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

# Included from build/make/core/Makefile (device/*/*/build/tasks/*.mk), which
# happens after vendor/twrp/build/tasks/kernel.mk, so KERNEL_OUT, KERNEL_CONFIG,
# TARGET_PREBUILT_INT_KERNEL and make-kernel-target are defined here.

# Samsung DTBH image ("dt" section of the recovery image, see BoardConfig.mk):
# the kernel has no dtb.img target of its own, so "make dtbs" builds the dtbs
# listed in CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE_NAMES and dtbhtoolExynos of
# dtbhtool/ (the LineageOS DTBH tool, called like the LineageOS mkbootimg.mk
# calls it) packs them into the DTBH container the bootloader reads.
ifeq ($(TARGET_DEVICE),a3y17lte)
ifeq ($(FULL_KERNEL_BUILD),true)
KERNEL_DTB_DIR := $(KERNEL_OUT)/arch/$(KERNEL_ARCH)/boot/dts
KERNEL_DTBIMG := $(KERNEL_OUT)/arch/$(KERNEL_ARCH)/boot/dtb.img
DTBTOOL := $(HOST_OUT_EXECUTABLES)/dtbhtoolExynos

# any device tree source change has to repack the image
KERNEL_DTS_SRCS := $(shell find -L $(TARGET_KERNEL_SOURCE)/arch/$(KERNEL_ARCH)/boot/dts \
	-maxdepth 1 -type f \( -name '*.dts' -o -name '*.dtsi' -o -name 'Makefile' \) 2>/dev/null)

$(TARGET_PREBUILT_INT_KERNEL): $(KERNEL_DTBIMG)

$(KERNEL_DTBIMG): $(KERNEL_CONFIG) $(DTBTOOL) $(KERNEL_DTS_SRCS)
	@echo "Building Exynos DTBH image (dtb.img)"
	$(call make-kernel-target,dtbs)
	$(hide) $(DTBTOOL) $(BOARD_DTBTOOL_ARGS) -o $@ -s $(BOARD_KERNEL_PAGESIZE) -p $(KERNEL_OUT)/scripts/dtc/ "$(KERNEL_DTB_DIR)"

# The recovery image has to wait for the dt image: its path only appears inside
# $(BOARD_MKBOOTIMG_ARGS), which the build system does not turn into a
# dependency.
ifneq ($(INSTALLED_RECOVERYIMAGE_TARGET),)
$(INSTALLED_RECOVERYIMAGE_TARGET): $(KERNEL_DTBIMG)
endif
endif
endif
