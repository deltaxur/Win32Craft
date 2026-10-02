#!/usr/bin/env python3
"""Patch the known untouched Minecraft 0.13.2 x86 executable for the universal win32craft.dll."""
from __future__ import annotations
import argparse, hashlib, struct
from pathlib import Path

EXPECTED_SHA256 = "fcef7724d6c77017aff79e24f8d5ccd980996d1601a53198dd7a1f8948978026"
OUTPUT_SHA256 = "141dc9c658e3217e2688a5e5c5d361a990499cf61a629f26fc97804b60c4c77d"
PATCHES = [
    # Core Win32 host patches.  The fibers/file/WinRT API-set descriptors
    # remain external; pure KERNEL32-only descriptors are minimized below.
    (0x152, "020000000000060002", "010000000000060001"),
    (0x16E, "4091", "0081"),
    (0x34280, "558bec8b450883380074", "c2040090909090909090"),
    (0x1DFDD0, "558bec6aff68d56c9a00", "c2240090909090909090"),
    (0x213AB0, "558bec6aff689dc89a00", "c3909090909090909090"),
    (0x3C5304, "c745fc01000000e890230000", "e9d100000090909090909090"),
    (0x3D0533, "e8a6821a00", "31c0599090"),
    (0x3D0603, "85f6740583c608eb0233f656e82c400000", "8365dc0033f683cfffe92f020000909090"),
    (0x3D0891, "a15c2bc2", "e95d0000"),
    (0x4B2571, "744e", "9090"),
    (0x538C97, "01", "00"),
    (0x579DCC, "ff7508ff154cf5a000", "b80100000090909090"),
    (0x57A19C, "6a1cb83ae99f00e81b0100008d45ec33db50895dece8be000000", "ff154cf5a000c3e81b0100008d45ec33db50895dec31c0909090"),
    (0x60FAF4, "5f57534138325f5838362e646c6c", "2e646c6c00000000000000000000"),
    (0x66F3E8, "5f57534138325f5838362e646c6c", "2e646c6c00000000000000000000"),
    (0x757A45, "5f6170702e444c4c", "2e444c4c00000000"),
    (0x75A600, "5f4150502e646c6c", "2e646c6c00000000"),
    (0x75A6B5, "5f4150502e646c6c", "2e646c6c00000000"),
    (0x75A882, "5f4150502e646c6c", "2e646c6c00000000"),
    (0x75B5FE, "44697361626c655468726561644c69627261727943616c6c73", "57696e3332426f6f7473747261700000000000000000000000"),
    (0x75B7BC, "6170692d6d732d77696e2d636f72652d6c6962726172796c6f616465722d6c312d322d302e646c6c", "77696e333263726166742e646c6c0000000000000000000000000000000000000000000000000000"),
]

# These API-set DLLs from the old Windows 7 bundle contained only direct
# KERNEL32 forwarders.  Redirect only their import-descriptor names to the
# native Windows 7 kernel32.dll.  Delayload, COM, WinRT, file and especially
# fibers remain external shims.  The fibers descriptor and FlsSetValue path
# are deliberately not modified.
KERNEL32_IMPORT_REDIRECTS = [
    "api-ms-win-core-datetime-l1-1-1.dll",
    "api-ms-win-core-debug-l1-1-1.dll",
    "api-ms-win-core-errorhandling-l1-1-1.dll",
    "api-ms-win-core-handle-l1-1-0.dll",
    "api-ms-win-core-interlocked-l1-2-0.dll",
    "api-ms-win-core-localization-l1-2-1.dll",
    "api-ms-win-core-processthreads-l1-1-2.dll",
    "api-ms-win-core-profile-l1-1-0.dll",
    "api-ms-win-core-string-l1-1-0.dll",
    "api-ms-win-core-synch-l1-2-0.dll",
    "api-ms-win-core-sysinfo-l1-2-1.dll",
    "api-ms-win-core-timezone-l1-1-0.dll",
    "api-ms-win-core-util-l1-1-0.dll",
]

WIN32CRAFT_IMPORT_REDIRECTS = [
    "XINPUTUAP.dll",
    "api-ms-win-core-com-l1-1-1.dll",
    "api-ms-win-core-delayload-l1-1-1.dll",
    "api-ms-win-core-fibers-l1-1-1.dll",
    "api-ms-win-core-file-l1-2-1.dll",
    "api-ms-win-core-file-l2-1-1.dll",
    "api-ms-win-core-winrt-error-l1-1-1.dll",
    "api-ms-win-core-winrt-l1-1-0.dll",
    "api-ms-win-core-winrt-string-l1-1-0.dll",
    "api-ms-win-eventing-provider-l1-1-0.dll",
    "api-ms-win-gaming-tcui-l1-1-0.dll",
]

def patch(source: Path, output: Path) -> None:
    original = source.read_bytes()
    digest = hashlib.sha256(original).hexdigest()
    if digest != EXPECTED_SHA256:
        raise ValueError(f"unsupported input SHA-256 {digest}; expected {EXPECTED_SHA256}")
    image = bytearray(original)
    pe_offset = int.from_bytes(image[0x3C:0x40], "little")
    characteristics_offset = pe_offset + 4 + 18
    characteristics = int.from_bytes(
        image[characteristics_offset:characteristics_offset + 2], "little"
    )
    if characteristics != 0x0102:
        raise ValueError(
            f"unexpected PE characteristics 0x{characteristics:04X}"
        )
    image[characteristics_offset:characteristics_offset + 2] = \
        (characteristics | 0x20).to_bytes(2, "little")
    for offset, old_hex, new_hex in PATCHES:
        old = bytes.fromhex(old_hex)
        new = bytes.fromhex(new_hex)
        actual = bytes(image[offset:offset + len(old)])
        if actual != old:
            raise ValueError(
                f"file+0x{offset:X} contains {actual.hex()}, expected {old.hex()}"
            )
        if len(old) != len(new):
            raise ValueError("internal patch length mismatch")
        image[offset:offset + len(old)] = new

    for descriptor in KERNEL32_IMPORT_REDIRECTS:
        old = descriptor.encode("ascii") + b"\0"
        positions = []
        start = 0
        while True:
            position = image.find(old, start)
            if position < 0:
                break
            positions.append(position)
            start = position + 1
        if len(positions) != 1:
            raise ValueError(
                f"expected one import descriptor {descriptor!r}, found {len(positions)}"
            )
        replacement = b"kernel32.dll\0"
        replacement += b"\0" * (len(old) - len(replacement))
        position = positions[0]
        image[position:position + len(old)] = replacement

    # Redirect import descriptors by changing their Name RVAs.  Replacing
    # strings in-place is unsafe: XINPUTUAP.dll is shorter than our DLL name
    # and sits immediately before another import name in this executable.
    pe = int.from_bytes(image[0x3C:0x40], "little")
    optional = pe + 4 + 20
    section_count = int.from_bytes(image[pe + 6:pe + 8], "little")
    optional_size = int.from_bytes(image[pe + 20:pe + 22], "little")
    section_table = optional + optional_size
    sections = []
    for index in range(section_count):
        section = section_table + index * 40
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from(
            "<4I", image, section + 8
        )
        sections.append((virtual_address, max(virtual_size, raw_size), raw_offset))

    def rva_offset(rva: int) -> int:
        for address, size, raw in sections:
            if address <= rva < address + size:
                return raw + rva - address
        raise ValueError(f"unmapped import RVA 0x{rva:X}")

    import_rva = int.from_bytes(image[optional + 0x68:optional + 0x6C], "little")
    descriptor_offset = rva_offset(import_rva)
    redirects = set(WIN32CRAFT_IMPORT_REDIRECTS)
    target_descriptors = []
    safe_name_rva = None
    for index in range(512):
        offset = descriptor_offset + index * 20
        fields = struct.unpack_from("<5I", image, offset)
        if not any(fields):
            break
        name_offset = rva_offset(fields[3])
        end = image.find(b"\0", name_offset)
        name = bytes(image[name_offset:end]).decode("ascii")
        if name in redirects:
            target_descriptors.append(offset)
            if safe_name_rva is None and len(name) >= len(b"win32craft.dll"):
                safe_name_rva = fields[3]
    if len(target_descriptors) != len(redirects) or safe_name_rva is None:
        raise ValueError("could not resolve all Win32Craft import descriptors")
    safe_name_offset = rva_offset(safe_name_rva)
    image[safe_name_offset:safe_name_offset + 15] = b"win32craft.dll\0"
    for offset in target_descriptors:
        image[offset + 12:offset + 16] = safe_name_rva.to_bytes(4, "little")

    result = hashlib.sha256(image).hexdigest()
    if OUTPUT_SHA256 and result != OUTPUT_SHA256:
        raise ValueError(f"output verification failed: {result} != {OUTPUT_SHA256}")
    output.write_bytes(image)
    print(f"created {output}")
    print(f"SHA-256 {result}")

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?", type=Path, default=Path("Minecraft.Win10.DX11.exe"))
    parser.add_argument("output", nargs="?", type=Path, default=Path("Win32Craft.0.13.2.exe"))
    args = parser.parse_args()
    patch(args.source, args.output)

if __name__ == "__main__":
    main()
