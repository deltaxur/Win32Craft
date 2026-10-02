#!/usr/bin/env python3
"""Create the Win32-hosted Minecraft 1.2.8 executable."""

from __future__ import annotations

import argparse
import hashlib
import struct
from dataclasses import dataclass
from pathlib import Path


EXPECTED_SHA256 = "dd3766af82b8f62a2a312b6f32c5a09ae898062d8a5c30b399e950d3fa3241f6"

WIN32CRAFT_API_SETS = {
    b"xinputuap.dll",
    b"api-ms-win-core-com-l1-1-0.dll",
    b"api-ms-win-core-winrt-error-l1-1-0.dll",
    b"api-ms-win-core-winrt-error-l1-1-1.dll",
    b"api-ms-win-core-winrt-l1-1-0.dll",
    b"api-ms-win-core-winrt-string-l1-1-0.dll",
    b"api-ms-win-eventing-provider-l1-1-0.dll",
}


def align(value: int, boundary: int) -> int:
    return (value + boundary - 1) & ~(boundary - 1)


def checked_write(image: bytearray, offset: int, expected: bytes,
                  replacement: bytes, label: str) -> None:
    actual = bytes(image[offset:offset + len(expected)])
    if actual != expected:
        raise ValueError(
            f"{label}: file+0x{offset:x} contains {actual.hex()}, "
            f"expected {expected.hex()}"
        )
    if len(expected) != len(replacement):
        raise ValueError(f"{label}: replacement length mismatch")
    image[offset:offset + len(replacement)] = replacement
    print(f"{label}: file+0x{offset:x}")


def replace_all(image: bytearray, old: bytes, new: bytes,
                expected_count: int, label: str) -> None:
    positions: list[int] = []
    start = 0
    while True:
        position = image.find(old, start)
        if position < 0:
            break
        positions.append(position)
        start = position + len(old)
    if len(positions) != expected_count:
        raise ValueError(
            f"{label}: expected {expected_count} occurrences, "
            f"found {len(positions)}"
        )
    if len(new) > len(old):
        raise ValueError(f"{label}: replacement is too long")
    for position in positions:
        image[position:position + len(old)] = new.ljust(len(old), b"\0")
        print(f"{label}: file+0x{position:x}")


@dataclass
class Section:
    virtual_address: int
    virtual_size: int
    raw_offset: int
    raw_size: int


class Pe32:
    def __init__(self, image: bytearray) -> None:
        self.image = image
        self.pe_offset = struct.unpack_from("<I", image, 0x3C)[0]
        if image[self.pe_offset:self.pe_offset + 4] != b"PE\0\0":
            raise ValueError("invalid PE signature")
        self.file_header = self.pe_offset + 4
        self.optional = self.file_header + 20
        if struct.unpack_from("<H", image, self.optional)[0] != 0x10B:
            raise ValueError("expected PE32")
        self.section_count = struct.unpack_from(
            "<H", image, self.file_header + 2
        )[0]
        self.optional_size = struct.unpack_from(
            "<H", image, self.file_header + 16
        )[0]
        self.section_table = self.optional + self.optional_size
        self.file_alignment = struct.unpack_from(
            "<I", image, self.optional + 0x24
        )[0]
        self.section_alignment = struct.unpack_from(
            "<I", image, self.optional + 0x20
        )[0]
        self.sections: list[Section] = []
        for index in range(self.section_count):
            header = self.section_table + index * 40
            virtual_size, virtual_address, raw_size, raw_offset = \
                struct.unpack_from("<4I", image, header + 8)
            self.sections.append(
                Section(virtual_address, virtual_size, raw_offset, raw_size)
            )

    def rva_to_offset(self, rva: int) -> int:
        for section in self.sections:
            extent = max(section.virtual_size, section.raw_size)
            if section.virtual_address <= rva < section.virtual_address + extent:
                return section.raw_offset + rva - section.virtual_address
        headers = struct.unpack_from("<I", self.image, self.optional + 0x3C)[0]
        if rva < headers:
            return rva
        raise ValueError(f"RVA 0x{rva:x} is outside the image")

    def c_string_at_rva(self, rva: int) -> bytes:
        offset = self.rva_to_offset(rva)
        end = self.image.index(0, offset)
        return bytes(self.image[offset:end])


def patch(source: Path, output: Path) -> None:
    original = source.read_bytes()
    digest = hashlib.sha256(original).hexdigest()
    if digest != EXPECTED_SHA256:
        raise ValueError(
            f"unsupported input SHA-256 {digest}; expected {EXPECTED_SHA256}"
        )
    image = bytearray(original)
    pe = Pe32(image)

    checked_write(
        image, pe.file_header + 18, struct.pack("<H", 0x0102),
        struct.pack("<H", 0x0122), "large-address-aware"
    )
    checked_write(image, pe.optional + 0x2A, b"\x02\x00", b"\x01\x00", "OS minor")
    checked_write(
        image, pe.optional + 0x32, b"\x02\x00", b"\x01\x00",
        "subsystem minor"
    )
    checked_write(
        image, pe.optional + 0x46, struct.pack("<H", 0x9140),
        struct.pack("<H", 0x8100), "desktop DLL characteristics"
    )

    replace_all(image, b"vccorlib140_app.DLL", b"vccorlib140.DLL", 1,
                "desktop vccorlib")
    replace_all(image, b"MSVCP140_APP.dll", b"MSVCP140.dll", 1,
                "desktop MSVCP")
    replace_all(image, b"CONCRT140_APP.dll", b"CONCRT140.dll", 1,
                "desktop CONCRT")
    replace_all(image, b"VCRUNTIME140_APP.dll", b"VCRUNTIME140.dll", 1,
                "desktop VCRUNTIME")
    replace_all(image, b"fmod_X86.dll", b"fmod.dll", 2, "desktop FMOD name")

    import_rva, import_size = struct.unpack_from("<2I", image, pe.optional + 0x68)
    cursor = pe.rva_to_offset(import_rva)
    descriptors: list[tuple[bytes, bytes]] = []
    kernel_descriptor: tuple[int, int] | None = None
    while True:
        descriptor = bytes(image[cursor:cursor + 20])
        values = struct.unpack("<5I", descriptor)
        if not any(values):
            break
        name = pe.c_string_at_rva(values[3]).lower()
        descriptors.append((name, descriptor))
        if name == b"kernel32.dll":
            kernel_descriptor = (values[0] or values[4], values[4])
        cursor += 20
    if len(descriptors) != 27 or import_size != 0x230:
        raise ValueError(
            f"unexpected import table: {len(descriptors)} descriptors, "
            f"size 0x{import_size:x}"
        )
    if kernel_descriptor is None:
        raise ValueError("KERNEL32 import descriptor not found")

    kernel_ilt_rva, kernel_iat_rva = kernel_descriptor
    kernel_ilt_offset = pe.rva_to_offset(kernel_ilt_rva)
    kernel_names: dict[str, tuple[int, int]] = {}
    index = 0
    while True:
        hint_rva = struct.unpack_from("<I", image, kernel_ilt_offset + index * 4)[0]
        if not hint_rva:
            break
        if hint_rva & 0x80000000:
            raise ValueError("unexpected ordinal KERNEL32 import")
        kernel_names[pe.c_string_at_rva(hint_rva + 2).decode("ascii")] = \
            (index, hint_rva)
        index += 1

    safe_hint_rva = kernel_names["GetLastError"][1]
    compatibility_names = (
        "ResolveDelayLoadedAPI", "ResolveDelayLoadsFromDll",
        "DelayLoadFailureHook", "CreateFile2",
    )
    compatibility_iats: dict[str, int] = {}
    for name in compatibility_names:
        entry_index, original_hint_rva = kernel_names[name]
        checked_write(
            image, kernel_ilt_offset + entry_index * 4,
            struct.pack("<I", original_hint_rva), struct.pack("<I", safe_hint_rva),
            f"{name} loader placeholder"
        )
        compatibility_iats[name] = kernel_iat_rva + entry_index * 4

    last = pe.sections[-1]
    new_raw_offset = align(len(image), pe.file_alignment)
    new_rva = align(
        last.virtual_address + max(last.virtual_size, last.raw_size),
        pe.section_alignment,
    )
    new_descriptor_count = len(descriptors) + len(compatibility_names) + 2
    section = bytearray(new_descriptor_count * 20)

    def append(data: bytes, alignment: int = 1) -> int:
        section.extend(b"\0" * ((-len(section)) % alignment))
        offset = len(section)
        section.extend(data)
        return offset

    dll_name_offset = append(b"win32craft.dll\0")
    hint_offsets: dict[str, int] = {}
    for name in ("Win32Bootstrap",) + compatibility_names:
        hint_offsets[name] = append(b"\0\0" + name.encode("ascii") + b"\0", 2)
    ilt_offsets: dict[str, int] = {}
    for name in ("Win32Bootstrap",) + compatibility_names:
        ilt_offsets[name] = append(
            struct.pack("<2I", new_rva + hint_offsets[name], 0), 4
        )
    bootstrap_iat_offset = append(
        struct.pack("<2I", new_rva + hint_offsets["Win32Bootstrap"], 0), 4
    )

    relocated: list[bytes] = []
    for dll_name, descriptor in descriptors:
        values = list(struct.unpack("<5I", descriptor))
        if dll_name in (b"dxgi.dll", b"d3d11.dll") or \
                dll_name in WIN32CRAFT_API_SETS:
            values[3] = new_rva + dll_name_offset
        relocated.append(struct.pack("<5I", *values))
    relocated.append(struct.pack(
        "<5I", new_rva + ilt_offsets["Win32Bootstrap"], 0, 0,
        new_rva + dll_name_offset, new_rva + bootstrap_iat_offset
    ))
    for name in compatibility_names:
        relocated.append(struct.pack(
            "<5I", new_rva + ilt_offsets[name], 0, 0,
            new_rva + dll_name_offset, compatibility_iats[name]
        ))
    relocated.append(b"\0" * 20)
    section[:new_descriptor_count * 20] = b"".join(relocated)

    virtual_size = len(section)
    raw_size = align(virtual_size, pe.file_alignment)
    section.extend(b"\0" * (raw_size - virtual_size))
    image.extend(b"\0" * (new_raw_offset - len(image)))
    image.extend(section)

    new_header = pe.section_table + pe.section_count * 40
    size_of_headers = struct.unpack_from("<I", image, pe.optional + 0x3C)[0]
    if new_header + 40 > size_of_headers:
        raise ValueError("no room for an additional section header")
    checked_write(
        image, new_header, b"\0" * 40,
        struct.pack(
            "<8s6I2HI", b".d3imp\0\0", virtual_size, new_rva, raw_size,
            new_raw_offset, 0, 0, 0, 0, 0xC0000040
        ), "new .d3imp section header"
    )
    checked_write(
        image, pe.file_header + 2, struct.pack("<H", pe.section_count),
        struct.pack("<H", pe.section_count + 1), "section count"
    )
    old_initialized = struct.unpack_from("<I", image, pe.optional + 8)[0]
    checked_write(
        image, pe.optional + 8, struct.pack("<I", old_initialized),
        struct.pack("<I", old_initialized + raw_size), "initialized data size"
    )
    old_image_size = struct.unpack_from("<I", image, pe.optional + 0x38)[0]
    new_image_size = align(new_rva + virtual_size, pe.section_alignment)
    checked_write(
        image, pe.optional + 0x38, struct.pack("<I", old_image_size),
        struct.pack("<I", new_image_size), "image size"
    )
    checked_write(
        image, pe.optional + 0x68, struct.pack("<2I", import_rva, import_size),
        struct.pack("<2I", new_rva, new_descriptor_count * 20),
        "relocated import directory"
    )

    bootstrap_iat_va = 0x400000 + new_rva + bootstrap_iat_offset
    checked_write(
        image, 0x10293A2, bytes.fromhex("50576800004000e8580d0000"),
        b"\xff\x15" + struct.pack("<I", bootstrap_iat_va) + b"\x90" * 6,
        "WinMain bootstrap call"
    )
    output.write_bytes(image)
    print(f"wrote {output} ({len(image)} bytes)")
    print(f"SHA-256 {hashlib.sha256(image).hexdigest()}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?", type=Path,
                        default=Path("Minecraft.Windows.exe"))
    parser.add_argument("output", nargs="?", type=Path,
                        default=Path("Win32Craft.1.2.8.exe"))
    args = parser.parse_args()
    patch(args.source, args.output)


if __name__ == "__main__":
    main()
