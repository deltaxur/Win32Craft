#!/usr/bin/env python3
"""Patch the untouched Minecraft 0.14.2 x86 EXE for win32craft.dll."""
from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SHA256 = '2299405fa247b558a15adff9185e761f377964b24e6b79c788457c41e9b84405'
OUTPUT_SHA256 = '6e2c030b924955f29e134db6f85399f030055c0c7444914c67cd4960dd7bb870'

# RVAs, original bytes, replacement bytes. The DLL owns all game hooks.
PATCHES = [
    # Replace the UWP main with the imported Win32 bootstrap.
    (0x006d08f0, '6a1cb80afdb600', 'ff150466b800c3'),
    # The same import was previously used by CRT DllMain.
    (0x006d04dc, 'ff7508ff150466b800', 'b80100000090909090'),
]

HOST_IMPORTS = {
    'xinputuap.dll',
    'api-ms-win-core-com-l1-1-1.dll',
    'api-ms-win-core-delayload-l1-1-1.dll',
    'api-ms-win-core-fibers-l1-1-1.dll',
    'api-ms-win-core-file-l1-2-1.dll',
    'api-ms-win-core-file-l2-1-1.dll',
    'api-ms-win-core-winrt-error-l1-1-1.dll',
    'api-ms-win-core-winrt-l1-1-0.dll',
    'api-ms-win-core-winrt-string-l1-1-0.dll',
    'api-ms-win-eventing-provider-l1-1-0.dll',
    'api-ms-win-gaming-tcui-l1-1-0.dll',
    'api-ms-win-core-libraryloader-l1-2-0.dll',
}


def patch(source: Path, output: Path) -> None:
    if source.resolve() == output.resolve():
        raise ValueError('source and output must be different; preserve the vanilla EXE')
    original = source.read_bytes()
    digest = hashlib.sha256(original).hexdigest()
    if digest != EXPECTED_SHA256:
        raise ValueError(f'unsupported source SHA-256: {digest}')
    image = bytearray(original)
    pe = struct.unpack_from('<I', image, 0x3c)[0]
    optional = pe + 24
    section_count = struct.unpack_from('<H', image, pe + 6)[0]
    optional_size = struct.unpack_from('<H', image, pe + 20)[0]
    sections = [struct.unpack_from('<4I', image, optional + optional_size + i * 40 + 8)
                for i in range(section_count)]

    def offset(rva):
        for vsize, va, raw_size, raw in sections:
            if va <= rva < va + raw_size:
                return raw + rva - va
        raise ValueError(f'unmapped RVA {rva:#x}')

    def replace(rva, before, after):
        before = bytes.fromhex(before)
        after = bytes.fromhex(after)
        pos = offset(rva)
        if len(before) != len(after) or image[pos:pos + len(before)] != before:
            raise ValueError(f'patch signature mismatch at RVA {rva:#x}')
        image[pos:pos + len(after)] = after

    # Desktop subsystem versions; fixed image base is required by the new call.
    struct.pack_into('<HH', image, optional + 0x28, 6, 1)
    struct.pack_into('<HH', image, optional + 0x30, 6, 1)
    struct.pack_into('<H', image, optional + 0x46, 0x8100)
    characteristics = struct.unpack_from('<H', image, pe + 22)[0]
    struct.pack_into('<H', image, pe + 22, characteristics | 0x20)
    for rva, before, after in PATCHES:
        replace(rva, before, after)

    # Share an existing long import name; XINPUTUAP has no space for a rename.
    directory = offset(struct.unpack_from('<I', image, optional + 0x68)[0])
    descriptors = []
    while any(image[directory:directory + 20]):
        fields = struct.unpack_from('<5I', image, directory)
        name_pos = offset(fields[3])
        end = image.index(0, name_pos)
        name = bytes(image[name_pos:end]).decode('ascii').lower()
        descriptors.append((directory, fields[3], name))
        directory += 20
    shared_rva = next(rva for _, rva, name in descriptors
                      if name == 'api-ms-win-core-libraryloader-l1-2-0.dll')
    seen = set()
    for pos, name_rva, name in descriptors:
        if name in HOST_IMPORTS:
            struct.pack_into('<I', image, pos + 12, shared_rva)
            seen.add(name)
        elif name.startswith('api-ms-win-core-'):
            name_pos = offset(name_rva)
            image[name_pos:name_pos + len(name) + 1] = b'kernel32.dll\0'.ljust(len(name) + 1, b'\0')
        elif name.endswith('_app.dll'):
            value = name.replace('_app.dll', '.dll').encode() + b'\0'
            name_pos = offset(name_rva)
            image[name_pos:name_pos + len(name) + 1] = value.ljust(len(name) + 1, b'\0')
    if seen != HOST_IMPORTS:
        raise ValueError(f'missing imports: {HOST_IMPORTS - seen}')
    image[offset(shared_rva):offset(shared_rva) + 15] = b'win32craft.dll\0'

    for old_name, new_name in [(b'DisableThreadLibraryCalls\0', b'Win32Bootstrap\0'),
                               (b'fmod_WSA82_X86.dll\0', b'fmod.dll\0')]:
        positions = [i for i in range(len(image)) if image.startswith(old_name, i)]
        if not positions:
            raise ValueError(f'missing name: {old_name!r}')
        for pos in positions:
            image[pos:pos + len(old_name)] = new_name.ljust(len(old_name), b'\0')

    result = hashlib.sha256(image).hexdigest()
    if OUTPUT_SHA256 and result != OUTPUT_SHA256:
        raise ValueError(f'output SHA-256 mismatch: {result}')
    output.write_bytes(image)
    print(f'created {output}\nSHA-256 {result}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    patch(args.source, args.output)
