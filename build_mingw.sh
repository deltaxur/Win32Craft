#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CC=${CC:-clang}
# MinGW-w64 GNU ABI target; no MSVC CRT/toolchain dependency.
LINK=${LINK:-lld-link}
TARGET=i686-w64-windows-gnu
BUILD=${BUILD:-build}
DIST=${DIST:-dist}
rm -rf "$BUILD" "$DIST"
mkdir -p "$BUILD" "$DIST"


CFLAGS=(
  -target "$TARGET" -O2 -fno-stack-protector -fno-builtin
  -I"$BUILD"
  -Wno-incompatible-pointer-types -Wno-microsoft-cast
  -Wno-ignored-attributes
)

"$CC" "${CFLAGS[@]}" -c win32_host.c -o "$BUILD/win32_host.obj"
"$CC" "${CFLAGS[@]}" -DWin32Bootstrap=Win32BootstrapEarly2016 \
  -DDllMain=DllMainEarly2016 -c win32_host_early2016.c \
  -o "$BUILD/win32_host_early2016.obj"
"$CC" "${CFLAGS[@]}" -c api_compat.c -o "$BUILD/api_compat.obj"

SYSTEM_LIB_DIR=${SYSTEM_LIB_DIR:-/usr/lib/wine/i386-windows}

"$LINK" /dll /machine:x86 /entry:DllMain@12 /def:win32craft.def \
  /out:"$DIST/win32craft.dll" /implib:"$BUILD/win32craft.lib" \
  "$BUILD/win32_host.obj" "$BUILD/win32_host_early2016.obj" \
  "$BUILD/api_compat.obj" \
  "$SYSTEM_LIB_DIR/libkernel32.a" "$SYSTEM_LIB_DIR/libuser32.a" \
  "$SYSTEM_LIB_DIR/libshell32.a" "$SYSTEM_LIB_DIR/libcomdlg32.a" \
  "$SYSTEM_LIB_DIR/libmsvcrt.a" \
  /nodefaultlib /subsystem:windows /nxcompat /dynamicbase \
  /safeseh:no /brepro /base:0x62940000 /stack:0x200000,0x1000 \
  /opt:ref /opt:icf

# Windows 7 has no WinRT API-set schema entries. Keep the genuine Microsoft
# vccorlib140.dll untouched and satisfy its imports with tiny forwarder DLLs.
for definition in \
  api-ms-win-core-winrt-error-l1-1-0.def \
  api-ms-win-core-winrt-l1-1-0.def \
  api-ms-win-core-winrt-string-l1-1-0.def
do
  output=${definition%.def}.dll
  "$LINK" /dll /machine:x86 /noentry /def:"$definition" \
    "$BUILD/win32craft.lib" \
    /out:"$DIST/$output" /nodefaultlib /subsystem:windows \
    /nxcompat /dynamicbase /brepro
done

# lld-link decorates x86 PE forwarder module names as _KERNEL32.  Forwarder
# strings are not C symbols, so normalize those strings in place.
python - "$DIST/win32craft.dll" <<'PYFIX'
from pathlib import Path
import sys
p = Path(sys.argv[1])
b = bytearray(p.read_bytes())
old = b"_KERNEL32."
count = 0
start = 0
while True:
    i = b.find(old, start)
    if i < 0:
        break
    end = b.find(b"\0", i)
    if end < 0:
        raise SystemExit("unterminated forwarder string")
    value = bytes(b[i + 1:end])
    original_len = end - i
    b[i:i + len(value)] = value
    b[i + len(value):i + original_len] = b"\0" * (original_len - len(value))
    count += 1
    start = end + 1
normal_count = b.count(b"KERNEL32.")
if count == 0:
    if normal_count == 0:
        raise SystemExit("no KERNEL32 forwarders found")
    print(f"KERNEL32 forwarders already normalized: {normal_count}")
else:
    if old in b:
        raise SystemExit("forwarder normalization incomplete")
    p.write_bytes(b)
    print(f"normalized {count} KERNEL32 forwarders")
PYFIX

# lld-link may spell PE forwarder modules as _KERNEL32. on x86.  Normalize
# every final DLL; already-normalized outputs are accepted.
python3 - "$DIST"/*.dll <<'PYALLFIX'
from pathlib import Path
import sys
for name in sys.argv[1:]:
    p = Path(name)
    b = bytearray(p.read_bytes())
    old = b"_KERNEL32."
    count = 0
    start = 0
    while True:
        i = b.find(old, start)
        if i < 0:
            break
        end = b.find(b"\0", i)
        if end < 0:
            raise SystemExit(f"unterminated forwarder in {p}")
        value = bytes(b[i + 1:end])
        original_len = end - i
        b[i:i + len(value)] = value
        b[i + len(value):i + original_len] = b"\0" * (original_len - len(value))
        count += 1
        start = end + 1
    if count:
        p.write_bytes(b)
        print(f"normalized {count} KERNEL32 forwarders in {p.name}")
PYALLFIX

sha256sum "$DIST/win32craft.dll"
