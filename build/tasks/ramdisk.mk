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

# Included from build/make/core/Makefile (device/*/*/build/tasks/*.mk).
#
# COMPRESSION_COMMAND (build/make/core/Makefile, from BOARD_RAMDISK_USE_*) is
# recipe text, not a build input, so flipping the flag does not make an existing
# ramdisk out of date - and a gzip ramdisk does not fit the 32 MiB window sboot
# reads. The stamp below records the command and is a prerequisite of the
# ramdisk, so a change of the compressor rebuilds it.
ifeq ($(TARGET_DEVICE),a3y17lte)

RAMDISK_COMPRESSION_STAMP := $(PRODUCT_OUT)/obj/recovery-ramdisk-compression.txt

ifneq ($(strip $(recovery_ramdisk)),)
$(recovery_ramdisk): $(RAMDISK_COMPRESSION_STAMP)
endif

# $(DEVICE_PATH) comes from this tree's BoardConfig.mk (the build system does
# not define it); wildcard() keeps a path that does not resolve from becoming a
# hard "no rule to make target" error.
$(RAMDISK_COMPRESSION_STAMP): $(wildcard $(DEVICE_PATH)/BoardConfig.mk)
	$(hide) mkdir -p $(dir $@)
	$(hide) echo "$(COMPRESSION_COMMAND)" > $@.new
	$(hide) if cmp -s $@.new $@; then \
	    rm -f $@.new; \
	else \
	    echo "Recovery ramdisk compression: $(COMPRESSION_COMMAND)"; \
	    mv -f $@.new $@; \
	fi

endif
