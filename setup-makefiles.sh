#!/bin/bash
#
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
#

set -e

VENDOR=samsung
DEVICE=a3y17lte

export INITIAL_COPYRIGHT_YEAR=2017

# proprietary-files_device.txt
declare -A PROP_FILES=(
    ["proprietary-files_a3y17lte.txt"]=""
    ["proprietary-files_a7y17lte.txt"]=""
    ["proprietary-files_m10lte.txt"]=""
    ["proprietary-files_a6lte.txt"]=""
    ["proprietary-files_a3y17lte-lineage-19.txt"]=""
)

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}"

# Copyright headers and guards
write_headers "a3y17lte"

mkdir -p "${MY_DIR}/vendor-tools/unified_proprietary"

cat ${MY_DIR}/vendor-tools/*.txt > "${MY_DIR}/vendor-tools/unified_proprietary/proprietary-files.txt"

# The standard blobs
write_makefiles "${MY_DIR}/vendor-tools/unified_proprietary/proprietary-files.txt" true

rm -rf "${MY_DIR}/vendor-tools/unified_proprietary"

# Finish
write_footers
