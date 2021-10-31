# SkyHawk Recovery Project for the Samsung Galaxy A3 2017

### How to build ###

```bash
# Create dirs
$ mkdir shrp ; cd shrp

# Init repo
$ repo init -u git://github.com/SHRP/platform_manifest_twrp_omni.git -b v3_9.0

# Clone my local repo
$ git clone https://github.com/samsungexynos7870/android_manifest_samsung_a3y17lte.git -b skyhawk .repo/local_manifests

# Sync
$ repo sync --no-repo-verify -c --force-sync --no-clone-bundle --no-tags --optimized-fetch --prune -j`nproc`

# Build
$ mv device/samsung/a3y17lte/build_skyhawk.sh .
$ . build_skyhawk.sh a3y17lte
```
## Credits
2019 @Astrako

## Contact
Telegram support group: https://t.me/joinchat/D1Jk_VbieGBXOWZt2y8O7A
