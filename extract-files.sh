#!/bin/bash
#
# Copyright (C) 2018-2026 The LineageOS Project
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

set -e

# Default values (can be overridden by arguments)
DEVICE=${1:-???}
PROPRIETARY_FILES=${2:-???}
SRC=${3:-???}
VENDOR=samsung

MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."
HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"

if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi

source "${HELPER}"

function blob_fixup() {
    case "${1}" in
        vendor/lib/hw/android.hardware.bluetooth@1.0-impl-qti.so | vendor/lib64/hw/android.hardware.bluetooth@1.0-impl-qti.so)
            # Fix bluetooth HAL configuration path
            sed -i 's|/system/etc/|/vendor/etc/|g' "${2}"
        ;;

        vendor/lib/libbauthserver.so | vendor/lib64/libbauthserver.so)
            # Add shim for libbauthserver.so
            "${PATCHELF}" --add-needed "libbauthtzcommon_shim.so" "${2}"
        ;;

        system/lib/libantradio.so | system/lib64/libantradio.so | vendor/lib/libantradio.so | vendor/lib64/libantradio.so)
            # ANT JNI library: provide the JNI helpers it was built against
            # (jniThrowException is no longer exported by libnativehelper)
            "${PATCHELF}" --add-needed "libnativehelper_shim.so" "${2}"
        ;;
    esac
}

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

SECTION=
KANG=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

extract "${MY_DIR}/${PROPRIETARY_FILES}" "${SRC}" "${KANG}" --section "${SECTION}"

"${MY_DIR}/setup-makefiles.sh"
