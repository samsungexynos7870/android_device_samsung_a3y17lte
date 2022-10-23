# SkyHawk Recovery Project for the Samsung Galaxy A3 2017

### How to build ###

```bash
# Create dirs
$ mkdir shrp ; cd shrp

# Init repo
$ repo init -u https://github.com/SHRP/manifest -b v3_9.0

# Clone my device tree
$ git clone https://github.com/samsungexynos7870/android_device_samsung_a3y17lte.git -b skyhawk device/samsung/a3y17lte

# Sync
$ repo sync --no-repo-verify -c --force-sync --no-clone-bundle --no-tags --optimized-fetch --prune -j`nproc`

# Build
$ . build/envsetup.sh && lunch omni_a3y17lte-eng && export ALLOW_MISSING_DEPENDENCIES=true && mka recoveryimage
```
## Credits
2019 @Astrako
2022 @Batuhantrkgl

## Contact
Telegram support group: https://t.me/joinchat/D1Jk_VbieGBXOWZt2y8O7A
