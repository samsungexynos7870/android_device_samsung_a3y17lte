#!/bin/bash

# configure some default settings for the build
export ALLOW_MISSING_DEPENDENCIES=true

# lzma
export LZMA_RAMDISK_TARGETS="recovery"

# compile it
for i in $*; do
    TARGET_DEVICE="$i"
    . build/envsetup.sh
    lunch omni_`echo $i`-eng
    mka recoveryimage -j`nproc`
done
