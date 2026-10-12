#!/bin/bash
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
# Tests for tools/mkbootimg-dt and tools/imginfo. They need bash, python3 and
# coreutils, no Android tree and no device:
#
#   tools/tests/test-mkbootimg-dt.sh

set -u

here=$(cd "$(dirname "$0")/.." && pwd)
mkbootimg_dt="${here}/mkbootimg-dt"
imginfo="${here}/imginfo"

work=$(mktemp -d)
trap 'rm -rf "${work}"' EXIT

page=2048
window=$((0x2000000))
partition=39845888

passed=0
failed=0

fail() {
	echo "FAIL: $*" >&2
	failed=$((failed + 1))
}

pass() {
	passed=$((passed + 1))
}

check() {
	description="$1"
	shift
	if "$@"; then
		pass
	else
		fail "${description}"
	fi
}

check_grep() {
	# check_grep <description> <pattern> <file>
	if grep -q -- "$2" "$3"; then
		pass
	else
		fail "$1 (no '$2' in $3)"
		sed 's/^/    | /' "$3" >&2
	fi
}

check_not_grep() {
	if grep -q -- "$2" "$3"; then
		fail "$1 ('$2' is in $3)"
		sed 's/^/    | /' "$3" >&2
	else
		pass
	fi
}

# ---------------------------------------------------------------------------
# the stub mkbootimg
# ---------------------------------------------------------------------------
cat >"${work}/mkbootimg" <<'STUB'
#!/usr/bin/env python3
"""A minimal stand in for the python mkbootimg of a 12.1 tree (header v0)."""
import argparse
import hashlib
import struct


def align(value, alignment):
    return (value + alignment - 1) // alignment * alignment


parser = argparse.ArgumentParser()
parser.add_argument('--kernel', type=argparse.FileType('rb'))
parser.add_argument('--ramdisk', type=argparse.FileType('rb'))
parser.add_argument('--second', type=argparse.FileType('rb'))
parser.add_argument('--dt', type=argparse.FileType('rb'))
parser.add_argument('--base', type=lambda x: int(x, 0), default=0x10000000)
parser.add_argument('--kernel_offset', type=lambda x: int(x, 0), default=0x8000)
parser.add_argument('--ramdisk_offset', type=lambda x: int(x, 0), default=0x1000000)
parser.add_argument('--second_offset', type=lambda x: int(x, 0), default=0xf00000)
parser.add_argument('--tags_offset', type=lambda x: int(x, 0), default=0x100)
parser.add_argument('--os_version', default='0')
parser.add_argument('--os_patch_level', default='0')
parser.add_argument('--pagesize', type=lambda x: int(x, 0), default=2048)
parser.add_argument('--header_version', type=lambda x: int(x, 0), default=0)
parser.add_argument('--board', default='')
parser.add_argument('--cmdline', default='')
parser.add_argument('-o', '--output', type=argparse.FileType('wb'), required=True)
args = parser.parse_args()

if args.header_version not in (0, 1):
    raise SystemExit('stub: header version %d not supported' % args.header_version)

# kernel, ramdisk, second stage and dt, read once: the id is the sha1 over them
blobs = [f.read() if f else b'' for f in (args.kernel, args.ramdisk, args.second,
                                          args.dt)]
sha = hashlib.sha1()
for f, blob in zip((args.kernel, args.ramdisk, args.second, args.dt), blobs):
    if f is None:
        sha.update(struct.pack('I', 0))
    else:
        sha.update(blob)
        sha.update(struct.pack('I', len(blob)))

header = struct.pack('<8sIIIIIIII', b'ANDROID!',
                     len(blobs[0]), args.base + args.kernel_offset,
                     len(blobs[1]), args.base + args.ramdisk_offset,
                     len(blobs[2]), args.base + args.second_offset,
                     args.base + args.tags_offset, args.pagesize)
header += struct.pack('<I', max(args.header_version, len(blobs[3])))
header += struct.pack('<I', 0)   # os version / patch level
header += b'\0' * 16            # board name
header += b'\0' * 512           # kernel command line
header += struct.pack('32s', sha.digest())
header += b'\0' * 1024          # extra command line, header is 1632 bytes
header += b'\0' * (align(len(header), args.pagesize) - len(header))
args.output.write(header)
for blob in blobs:
    if not blob:
        continue
    args.output.write(blob)
    args.output.write(b'\0' * (align(len(blob), args.pagesize) - len(blob)))
STUB
chmod 755 "${work}/mkbootimg"

# a fake kernel, fake ramdisks (with the magic of the format they claim) and
# fake dt images
head -c 5000 /dev/zero >"${work}/kernel"
for fmt in gzip lzma xz lz4 zstd raw unknown; do
	case "${fmt}" in
	gzip) magic='\x1f\x8b\x08\x00' ;;
	lzma) magic='\x5d\x00\x00\x80\x00' ;;
	xz) magic='\xfd7zXZ\x00' ;;
	lz4) magic='\x02\x21\x4c\x18' ;;
	zstd) magic='\x28\xb5\x2f\xfd' ;;
	raw) magic='070701' ;;
	unknown) magic='\x00\x01\x02\x03' ;;
	esac
	printf "${magic}" >"${work}/ramdisk-${fmt}"
	head -c 9000 /dev/zero >>"${work}/ramdisk-${fmt}"
done
head -c 4096 /dev/zero >"${work}/dt-aligned"
head -c 3000 /dev/zero >"${work}/dt-unaligned"
echo "this is not a boot image" >"${work}/not-an-image"

recovery() {
	# recovery <output> <ramdisk> [extra arguments ...]
	out="$1"
	ramdisk="$2"
	shift 2
	"${mkbootimg_dt}" "${work}/mkbootimg" \
		--kernel "${work}/kernel" --ramdisk "${ramdisk}" \
		--kernel_offset 0x00008000 --ramdisk_offset 0x01000000 \
		--tags_offset 0x00000100 \
		--os_version 16.1.0 --os_patch_level 2099-12-31 \
		"$@" --output "${out}"
}

reference() {
	# what the real mkbootimg would pack for the same arguments
	"${work}/mkbootimg" --kernel "${work}/kernel" "$@"
}

# ---------------------------------------------------------------------------
echo "== an image without a dt section is passed through unchanged"
recovery "${work}/plain.img" "${work}/ramdisk-lzma" --max-image-size none \
	>"${work}/out" 2>&1
rc=$?
check "plain image builds (rc=${rc})" test "${rc}" -eq 0
reference --ramdisk "${work}/ramdisk-lzma" --kernel_offset 0x00008000 \
	--ramdisk_offset 0x01000000 --tags_offset 0x00000100 \
	--os_version 16.1.0 --os_patch_level 2099-12-31 \
	--output "${work}/plain-reference.img"
check "plain image is byte identical to one the real mkbootimg packs" \
	cmp -s "${work}/plain.img" "${work}/plain-reference.img"
check_grep "the report names the compression of the ramdisk" \
	'load 0x11000000, lzma' "${work}/out"

# ---------------------------------------------------------------------------
echo "== a page aligned dt section is appended the way the real mkbootimg does"
recovery "${work}/dt.img" "${work}/ramdisk-lzma" \
	--dt "${work}/dt-aligned" >"${work}/out" 2>&1
rc=$?
check "image with a dt section builds (rc=${rc})" test "${rc}" -eq 0
reference --ramdisk "${work}/ramdisk-lzma" --kernel_offset 0x00008000 \
	--ramdisk_offset 0x01000000 --tags_offset 0x00000100 \
	--os_version 16.1.0 --os_patch_level 2099-12-31 \
	--dt "${work}/dt-aligned" --output "${work}/dt-reference.img"
check "dt image is byte identical to one the real mkbootimg packs" \
	cmp -s "${work}/dt.img" "${work}/dt-reference.img"
expected_id=$(python3 - "${work}" <<'PYID'
# the sha1 mkbootimg puts in the header: the sections in its order, each
# followed by its size as a little endian u32 (four zero bytes when a section
# was not given)
import hashlib
import os
import struct
import sys

work = sys.argv[1]
sha = hashlib.sha1()
for name in ('kernel', 'ramdisk-lzma', None, 'dt-aligned'):
    if name is None:
        sha.update(struct.pack('I', 0))
    else:
        data = open(os.path.join(work, name), 'rb').read()
        sha.update(data)
        sha.update(struct.pack('I', len(data)))
print(sha.hexdigest())
PYID
)
got_id=$(od -An -tx1 -j576 -N20 "${work}/dt.img" | tr -d ' \n')
check "the image id covers the dt section (${got_id}, want ${expected_id})" \
	test "${got_id}" = "${expected_id}"
check_grep "the report shows the dt section" '   dt ' "${work}/out"
check_grep "the report shows the window verdict" '(32 MiB' "${work}/out"

# ---------------------------------------------------------------------------
echo "== an unaligned dt section is padded, its section size is recorded"
recovery "${work}/dt-pad.img" "${work}/ramdisk-lzma" \
	--max-image-size none --dt "${work}/dt-unaligned" >"${work}/out" 2>&1
rc=$?
check "image with an unaligned dt section builds (rc=${rc})" test "${rc}" -eq 0
image=$(stat -c %s "${work}/dt-pad.img")
check "the image is page aligned (${image})" test $((image % page)) -eq 0
field40=$(od -An -tu4 -j40 -N4 "${work}/dt-pad.img" | tr -d ' ')
expected=$(( (3000 + page - 1) / page * page ))
check "offset 40 holds the padded section size (${field40}, want ${expected})" \
	test "${field40}" -eq "${expected}"

# ---------------------------------------------------------------------------
echo "== an image over the 32 MiB window fails the build and is removed"
head -c $((window + 4096)) /dev/zero >"${work}/big-kernel"
"${mkbootimg_dt}" "${work}/mkbootimg" \
	--kernel "${work}/big-kernel" --ramdisk "${work}/ramdisk-lzma" \
	--output "${work}/big.img" >"${work}/stdout" 2>"${work}/stderr"
rc=$?
cat "${work}/stdout" "${work}/stderr" >"${work}/out"
check "an over sized image fails (rc=${rc})" test "${rc}" -ne 0
check "the image is removed" test ! -e "${work}/big.img"
check_grep "the report says why" 'FAIL' "${work}/out"
check_grep "the report goes to stdout" 'FAIL' "${work}/stdout"
check_grep "the failure quotes the sboot message" \
	'INVALID RAMDISK LENGTH' "${work}/stderr"
check_grep "the failure names the ramdisk to repack" \
	"rm -f ${work}/ramdisk-lzma" "${work}/stderr"
check_grep "the failure names the levers" \
	'TW_EXTRA_LANGUAGES' "${work}/stderr"

# ---------------------------------------------------------------------------
echo "== exactly the window is fine, one page over is not"
ramdisk_aligned=$(( (9005 + page - 1) / page * page ))
kernel_fit=$(( window - page - ramdisk_aligned ))
head -c "${kernel_fit}" /dev/zero >"${work}/fits-kernel"
"${mkbootimg_dt}" "${work}/mkbootimg" \
	--kernel "${work}/fits-kernel" --ramdisk "${work}/ramdisk-lzma" \
	--output "${work}/fits.img" >"${work}/out" 2>&1
rc=$?
check "an image of exactly 32 MiB builds (rc=${rc})" test "${rc}" -eq 0
check "the image is $(stat -c %s "${work}/fits.img" 2>/dev/null) bytes, want ${window}" \
	test "$(stat -c %s "${work}/fits.img")" -eq "${window}"
check_grep "the report calls it a fit" 'fits the' "${work}/out"

# the dt section is what pushes this one over: the ramdisk still ends inside
# the window, the sections after it do not
kernel_tail=$(( window - page - ramdisk_aligned - page ))
head -c "${kernel_tail}" /dev/zero >"${work}/tail-kernel"
"${mkbootimg_dt}" "${work}/mkbootimg" \
	--kernel "${work}/tail-kernel" --ramdisk "${work}/ramdisk-lzma" \
	--dt "${work}/dt-aligned" --output "${work}/over.img" \
	>"${work}/out" 2>&1
rc=$?
check "an image a page over the window fails (rc=${rc})" test "${rc}" -ne 0
check "and is removed" test ! -e "${work}/over.img"
check_grep "the report points at what is behind the ramdisk" \
	'inside the window' "${work}/out"
check_grep "the report quotes the sboot message" \
	'INVALID RAMDISK LENGTH' "${work}/out"

# ---------------------------------------------------------------------------
echo "== a ramdisk that is not compressed the way the device needs is caught"
recovery "${work}/gzip.img" "${work}/ramdisk-gzip" \
	--expect-ramdisk-compression lzma >"${work}/out" 2>&1
rc=$?
check "a gzip ramdisk is refused when lzma is expected (rc=${rc})" \
	test "${rc}" -ne 0
check "the image is removed" test ! -e "${work}/gzip.img"
check_grep "the report names what it found" 'ramdisk is gzip' "${work}/out"
check_grep "the failure explains the stale ramdisk" \
	'does not recompress' "${work}/out"

recovery "${work}/lzma.img" "${work}/ramdisk-lzma" \
	--expect-ramdisk-compression lzma >"${work}/out" 2>&1
rc=$?
check "a lzma ramdisk is accepted (rc=${rc})" test "${rc}" -eq 0
check "the image is there" test -e "${work}/lzma.img"

recovery "${work}/any.img" "${work}/ramdisk-gzip" >"${work}/out" 2>&1
rc=$?
check "without an expectation any ramdisk is accepted (rc=${rc})" \
	test "${rc}" -eq 0

# ---------------------------------------------------------------------------
echo "== misuse fails before anything is written"
recovery "${work}/none.img" "${work}/ramdisk-lzma" --dt "${work}/no-such-dt" \
	>"${work}/out" 2>&1
rc=$?
check "a missing dt image fails (rc=${rc})" test "${rc}" -ne 0
check "no image is left behind" test ! -e "${work}/none.img"
check_grep "the failure names the dt image" 'no-such-dt' "${work}/out"

recovery "${work}/none.img" "${work}/ramdisk-lzma" --dt "${work}/dt-aligned" \
	--header_version 1 >"${work}/out" 2>&1
rc=$?
check "a dt section with a v1 header fails (rc=${rc})" test "${rc}" -ne 0
check_grep "the failure explains the header version" \
	'header version 0' "${work}/out"

"${mkbootimg_dt}" "${work}/mkbootimg" --kernel "${work}/kernel" \
	--ramdisk "${work}/ramdisk-lzma" --pagesize 3000 --output "${work}/bad.img" \
	>"${work}/out" 2>&1
rc=$?
check "a page size that is not a power of two fails (rc=${rc})" test "${rc}" -ne 0
check_grep "the failure names the page size" 'not a power of two' "${work}/out"

"${mkbootimg_dt}" "${work}/mkbootimg" --kernel "${work}/kernel" \
	--ramdisk "${work}/ramdisk-lzma" --expect-ramdisk-compression zip \
	--output "${work}/bad.img" >"${work}/out" 2>&1
rc=$?
check "an unknown compression fails (rc=${rc})" test "${rc}" -ne 0
check_grep "the failure names the format" "'zip'" "${work}/out"

echo "== an image that cannot be looked at fails the build instead of passing"
cat >"${work}/mkbootimg-junk" <<'JUNK'
#!/bin/bash
while [ $# -gt 0 ]; do
	case "$1" in
	-o | --output) echo "this is not a boot image" >"$2"; shift 2 ;;
	-o*) echo "this is not a boot image" >"${1#-o}"; shift ;;
	--output=*) echo "this is not a boot image" >"${1#--output=}"; shift ;;
	*) shift ;;
	esac
done
JUNK
chmod 755 "${work}/mkbootimg-junk"
"${mkbootimg_dt}" "${work}/mkbootimg-junk" --kernel "${work}/kernel" \
	--ramdisk "${work}/ramdisk-lzma" --output "${work}/junk.img" \
	>"${work}/out" 2>&1
rc=$?
check "an image that is not a boot image fails the build (rc=${rc})" \
	test "${rc}" -ne 0
check_grep "the failure says it could not be checked" \
	'could not be checked' "${work}/out"
check "the image is left in place to look at" test -e "${work}/junk.img"

echo "== a tree without the checker fails the build instead of passing silently"
mkdir -p "${work}/no-imginfo"
cp "${mkbootimg_dt}" "${work}/no-imginfo/"
"${work}/no-imginfo/mkbootimg-dt" "${work}/mkbootimg" --kernel "${work}/kernel" \
	--ramdisk "${work}/ramdisk-lzma" --output "${work}/none2.img" \
	>"${work}/out" 2>&1
rc=$?
check "a missing imginfo fails the build (rc=${rc})" test "${rc}" -ne 0
check_grep "and says so" 'imginfo is missing' "${work}/out"
check "and no image is left behind" test ! -e "${work}/none2.img"

# ---------------------------------------------------------------------------
echo "== imginfo on its own"
"${imginfo}" --max-size none --expect-ramdisk-compression lzma \
	"${work}/lzma.img" >"${work}/out" 2>&1
check "a good image passes" test $? -eq 0
"${imginfo}" --expect-ramdisk-compression gzip "${work}/lzma.img" \
	>"${work}/out" 2>&1
check "a wrong compression is exit 2" test $? -eq 2
"${imginfo}" --partition-size "${partition}" "${work}/lzma.img" \
	>"${work}/out" 2>&1
check "the partition check passes" test $? -eq 0
check_grep "and is reported" "fits the ${partition} byte partition" "${work}/out"
"${imginfo}" --partition-size 4096 "${work}/lzma.img" >"${work}/out" 2>&1
check "an image larger than the partition is exit 4" test $? -eq 4

# one image whose ramdisk itself ends past the window, one with only the tail
# behind it
head -c $((window + 4096)) /dev/zero >"${work}/huge-ramdisk"
"${work}/mkbootimg" --kernel "${work}/kernel" \
	--ramdisk "${work}/huge-ramdisk" --output "${work}/huge.img"
"${imginfo}" "${work}/huge.img" >"${work}/out" 2>&1
rc=$?
check "an image over the window is exit 1 (rc=${rc})" test "${rc}" -eq 1
check_grep "the report says the ramdisk itself is over the window" \
	'bytes past the' "${work}/out"
"${imginfo}" --max-size none "${work}/huge.img" >"${work}/out" 2>&1
check "'--max-size none' only reports" test $? -eq 0
check_not_grep "and has no verdict" 'FAIL' "${work}/out"

"${imginfo}" "${work}/not-an-image" >"${work}/out" 2>&1
check "something that is not a boot image is exit 8" test $? -eq 8
"${imginfo}" "${work}/does-not-exist" >"${work}/out" 2>&1
check "a missing file is exit 8" test $? -eq 8

for fmt in gzip lzma xz lz4 zstd raw unknown; do
	"${work}/mkbootimg" --kernel "${work}/kernel" \
		--ramdisk "${work}/ramdisk-${fmt}" \
		--output "${work}/${fmt}.img"
	"${imginfo}" --max-size none --expect-ramdisk-compression "${fmt}" \
		"${work}/${fmt}.img" >"${work}/out" 2>&1
	check "imginfo recognises a ${fmt} ramdisk" test $? -eq 0
done

echo
echo "${passed} passed, ${failed} failed"
[ "${failed}" -eq 0 ]
