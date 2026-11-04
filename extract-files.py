#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2017-2026 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import lib_fixups
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)
from extract_utils.module import (
    ProprietaryFile,
)

import os

# The dump driver runs one proprietary-files list per firmware dump. Select a
# single list during extraction so files absent from that dump are not fatal;
# the final setup-makefiles.py pass processes all lists together.
_EXTRACT_UTILS_ONLY_LIST = os.environ.get('EXTRACT_UTILS_ONLY_LIST')


def _proprietary_file(file_list_path: str, vendor_rel_sub_path: str = 'proprietary', **kwargs):
    if _EXTRACT_UTILS_ONLY_LIST and _EXTRACT_UTILS_ONLY_LIST not in file_list_path:
        return None
    return ProprietaryFile(file_list_path, vendor_rel_sub_path, **kwargs)


namespace_imports = [
    'device/samsung/a3y17lte',
    'device/samsung/universal7870-common',
    'hardware/samsung',
    'hardware/lineage/compat',
    'vendor/samsung/a3y17lte',
]


blob_fixups: blob_fixups_user_type = {
    (
        'vendor/lib/libbauthserver.so',
        'vendor/lib64/libbauthserver.so',
    ): blob_fixup()
        .add_needed('libbauthtzcommon_shim.so'),
}  # fmt: skip


module = ExtractUtilsModule(
    'a3y17lte',
    'samsung',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    skip_main_proprietary_file=True,
    proprietary_files=[
        pf for pf in (
            _proprietary_file('vendor-tools/proprietary-files_a3y17lte.txt'),
            _proprietary_file('vendor-tools/proprietary-files_a6lte.txt'),
            _proprietary_file('vendor-tools/proprietary-files_j5y17lte.txt'),
            _proprietary_file('vendor-tools/proprietary-files_m10lte.txt'),
        ) if pf is not None
    ],
)


if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
