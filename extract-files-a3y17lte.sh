MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ORIGINAL_DIR="${PWD}"

ANDROID_ROOT="${MY_DIR}/../../.."
DEVICE_COMMON=universal7870-common
VENDOR=samsung
VENDOR_MK_ROOT="${ANDROID_ROOT}"/vendor/"${VENDOR}"
DEVICE_COMMON_ROOT="${ANDROID_ROOT}"/device/"${VENDOR}"/"${DEVICE_COMMON}"

TARGET_SOURCES_DIR="${VENDOR_MK_ROOT}/tmp/sources"
mkdir -p "$TARGET_SOURCES_DIR"

REPO_URLS=(
    "https://github.com/Exynos7870-labs/samsung_a3y17lte_dump.git -b a3y17ltexc-user-8.0.0-R16NW-A320FLXXS9CTK1-release-keys A320FLXXS9CTK1"
    "https://github.com/Exynos7870-labs/samsung_a6lte_dump.git -b a6ltejx-user-10-QP1A.190711.020-A600FJXU9CVB1-release-keys A600FJXU9CVB1"
    "https://github.com/Exynos7870-labs/samsung_a7y17lteskt_dump.git -b a7y17lteskt-user-9-PPR1.180610.011-A720SKSU5CUJ2-release-keys A720SKSU5CUJ2"
    "https://github.com/Exynos7870-labs/samsung_j5y17lte_dump.git -b j5y17ltexx-user-9-PPR1.180610.011-J530FXXS8CUE4-release-keys J530FXXS8CUE4"
)

cd "$TARGET_SOURCES_DIR"
for i in "${!REPO_URLS[@]}"; do
    # Extract target directory name (last word in the string)
    repo_info="${REPO_URLS[$i]}"
    target_dir="${repo_info##* }"

    echo "Checking: $target_dir"

    if [[ -d "$target_dir" ]]; then
        echo "  Directory $target_dir already exists. Skipping."
    else
        echo "  Cloning: ${REPO_URLS[$i]}"
        git clone ${REPO_URLS[$i]}
    fi
done
ls
cd "$ORIGINAL_DIR"

COMMON_O_A3Y17LTE_PATH="${TARGET_SOURCES_DIR}/A320FLXXS9CTK1"
COMMON_Q_A6LTE_PATH="${TARGET_SOURCES_DIR}/A600FJXU9CVB1"
COMMON_P_A7Y17LTE_PATH="${TARGET_SOURCES_DIR}/A720SKSU5CUJ2"
COMMON_P_J5Y17LTE_PATH="${TARGET_SOURCES_DIR}/J530FXXS8CUE4"

# a3y17lte files (Oreo)
./extract-files.sh a3y17lte vendor-tools/proprietary-files_a3y17lte.txt -n -k $COMMON_O_A3Y17LTE_PATH

# a6lte files (Biometrics - ET5XX)
./extract-files.sh a3y17lte vendor-tools/proprietary-files_a6lte.txt -n -k $COMMON_Q_A6LTE_PATH

# a7y17lte files (Bluetooth + ANT)
./extract-files.sh a3y17lte vendor-tools/proprietary-files_a7y17lte.txt -n -k $COMMON_P_A7Y17LTE_PATH

# j5y17lte files (NFC)
./extract-files.sh a3y17lte vendor-tools/proprietary-files_j5y17lte.txt -n -k $COMMON_P_J5Y17LTE_PATH

# lineage-19 vendor dump (camera + NFC blobs for the cam-34xx setup).
# Point A3Y17LTE_LINEAGE19_DUMP at an extracted lineage-19 vendor tree
# (e.g. the vendor partition of a running lineage-19 build) to pull in
# proprietary-files_a3y17lte-lineage-19.txt as well.
LINEAGE19_DUMP="${A3Y17LTE_LINEAGE19_DUMP:-}"
if [ -n "${LINEAGE19_DUMP}" ] && [ -d "${LINEAGE19_DUMP}" ]; then
    ./extract-files.sh a3y17lte vendor-tools/proprietary-files_a3y17lte-lineage-19.txt -n -k "${LINEAGE19_DUMP}"
else
    echo "NOTE: A3Y17LTE_LINEAGE19_DUMP is not set or does not exist."
    echo "      Skipping vendor-tools/proprietary-files_a3y17lte-lineage-19.txt (camera/NFC blobs)."
fi
