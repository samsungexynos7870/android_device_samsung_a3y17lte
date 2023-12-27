#!/bin/bash
#
# Copyright (C) 2018-2023 The LineageOS Project
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

VENDOR=samsung
DEVICE=a3y17lte

MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."
HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"

if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi

SECTION=
KANG=
CLEAN_VENDOR=true

source "${HELPER}"

# proprietary-files_device.txt
declare -A PROP_FILES=(
    ["proprietary-files_a3y17lte.txt"]=""
    ["proprietary-files_a7y17lte.txt"]=""
    ["proprietary-files_m10lte.txt"]=""
    ["proprietary-files_a6lte.txt"]=""
    ["proprietary-files_a3y17lte-lineage-19.txt"]=""
)

function usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -pfs | --proprietary-files-list-and-source <proprietary-files_device.txt> <source_dir> , proprietary-files_somedevice.txt file list with its source directory."
    echo "Currenty need <source_dir> for files listed in following .txt:"
    for key in "${!PROP_FILES[@]}"; do
        echo "$key ${PROP_FILES[$key]}"
    done
    echo ""
    echo "  -n   | --no-cleanup                        Do not clean the vendor directory."
    echo "  -k   | --kang                              Kang (rebrand) proprietary files from another device."
    echo "  -s   | --section                           helper "
    echo "  -h   | --help                              Show this help message."
    echo ""
    echo "example Usage: look at vendor-tools\a3y17lte-common-extract-files-example.sh"
    echo ""
    exit 1
}

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -n|--no-cleanup)
            CLEAN_VENDOR=false
            ;;
        -k|--kang)
            KANG="--kang"
            ;;
        -s|--section)
            SECTION="${2}"; shift
            CLEAN_VENDOR=false
            ;;
        -pfs|--proprietary-files-list-and-source)
            PROP_FILES["$2"]="${3}"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            usage
            ;;
    esac
    shift
done

# Debug: Outputs current state of PROP_FILES
#for key in "${!PROP_FILES[@]}"; do
#    echo "$key=${PROP_FILES[$key]}"
#done

# vendor setup helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

for PROP_FILE in "${!PROP_FILES[@]}"; do
    SOURCE_DIR=${PROP_FILES[$PROP_FILE]}
    # Check if any source directory with proprietary-files is empty.
    if [ -z "${SOURCE_DIR}" ]; then
        echo "Error: Source directory not specified for ${PROP_FILE}. Provide it with --file ${PROP_FILE} <source_dir>"
        usage
    fi
    
    # Check if provided source directory exists
    if [ ! -d "${SOURCE_DIR}" ]; then
        echo "Error: Source directory ${SOURCE_DIR} does not exist."
        exit 1
    fi
    
    # helper needs to be in loop too to always get relauched with correct options
    source "${HELPER}"

    extract "${MY_DIR}/vendor-tools/${PROP_FILE}" "${SOURCE_DIR}" "${KANG}" --section "${SECTION}"
    
done

# BLOB_ROOT
BLOB_ROOT="${ANDROID_ROOT}"/vendor/"${VENDOR}"/"${DEVICE}"/proprietary/vendor

# Fix proprietary blobs
for lib in libexynoscamera.so libexynoscamera3.so; do
    if [[ -f "${BLOB_ROOT}/lib/${lib}" ]]; then
        "${PATCHELF}" --replace-needed "libcamera_client.so" "libcamera_metadata_helper.so" "${BLOB_ROOT}/lib/${lib}"
        "${PATCHELF}" --replace-needed "libgui.so" "libgui_vendor.so" "${BLOB_ROOT}/lib/${lib}"
        "${PATCHELF}" --add-needed "libexynoscamera_shim.so" "${BLOB_ROOT}/lib/${lib}"
        "${PATCHELF}" --add-needed "libgui_vendor_shim_exynos7870.so" "${BLOB_ROOT}/lib/${lib}"
    fi
done

for lib in android.hardware.bluetooth@1.0-impl-qti.so; do
    if [[ -f "${BLOB_ROOT}/lib/hw/${lib}" ]]; then
        sed -i 's|/system/etc/|/vendor/etc/|g' "${BLOB_ROOT}/lib/hw/${lib}"
    fi
    
    if [[ -f "${BLOB_ROOT}/lib64/hw/${lib}" ]]; then
        sed -i 's|/system/etc/|/vendor/etc/|g' "${BLOB_ROOT}/lib64/hw/${lib}"
    fi
done

for lib in camera.vendor.exynos7870.so; do
    if [[ -f "${BLOB_ROOT}/lib/hw/${lib}" ]]; then
        "${PATCHELF}" --replace-needed "libcamera_client.so" "libcamera_metadata_helper.so" "${BLOB_ROOT}/lib/hw/${lib}"
        "${PATCHELF}" --replace-needed "libgui.so" "libgui_vendor.so" "${BLOB_ROOT}/lib/hw/${lib}"
        "${PATCHELF}" --add-needed "libexynoscamera_shim.so" "${BLOB_ROOT}/lib/hw/${lib}"
        "${PATCHELF}" --add-needed "libgui_vendor_shim_exynos7870.so" "${BLOB_ROOT}/lib/hw/${lib}"
    fi
done

"${MY_DIR}/setup-makefiles.sh"
