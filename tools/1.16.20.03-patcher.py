#!/usr/bin/env python3
"""Create the Win32-hosted Minecraft 1.16.20.0.3 executable."""

from __future__ import annotations

import argparse
import hashlib
import struct
from dataclasses import dataclass
from pathlib import Path


EXPECTED_SHA256 = "9ab7e10904d5acf1afe83ab0b666c1426aa61d71f6b15d782d1ae7062703ec63"

WIN32CRAFT_API_SETS = {
    b"xinputuap.dll",
    b"api-ms-win-core-com-l1-1-0.dll",
    b"api-ms-win-core-memory-l1-1-3.dll",
    b"api-ms-win-core-winrt-error-l1-1-0.dll",
    b"api-ms-win-core-winrt-error-l1-1-1.dll",
    b"api-ms-win-core-winrt-l1-1-0.dll",
    b"api-ms-win-core-winrt-string-l1-1-0.dll",
    b"api-ms-win-eventing-provider-l1-1-0.dll",
}

RENOIR_WINDOWS7_API_SETS = {
    b"api-ms-win-core-synch-l1-2-0.dll",
    b"api-ms-win-core-processthreads-l1-1-2.dll",
    b"api-ms-win-core-profile-l1-1-0.dll",
    b"api-ms-win-core-sysinfo-l1-2-1.dll",
    b"api-ms-win-core-libraryloader-l1-2-0.dll",
    b"api-ms-win-core-interlocked-l1-2-0.dll",
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
        image, pe.file_header + 18, struct.pack("<H", 0x0122),
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

    # The Win32 PnP fallback intentionally supplies no WinRT hardware object.
    # This template specialization otherwise dereferences that null operation
    # and its synchronous caller subsequently waits forever.  Mark its task
    # implementation Completed (state 2); the continuation consumes the null
    # PnpObject as the supported "metadata unavailable" result.
    checked_write(
        image, pe.rva_to_offset(0x016edd4f),
        bytes.fromhex("8b0b5653ff511885c0782b"),
        bytes.fromhex("8b07c74064020000009090"),
        "complete unavailable PnP task"
    )
    checked_write(
        image, pe.rva_to_offset(0x016e30d7),
        bytes.fromhex("e8544fb8fe"), b"\x90" * 5,
        "skip unavailable startup metadata wait"
    )
    # PointerVisualizationSettings only disables touch/contact feedback.  It
    # has no desktop equivalent and its C++/CX projection relies on an agile
    # WinRT factory identity that is unavailable in the unpackaged process.
    # Skip this self-contained optional block and continue with CoreWindow
    # event registration at 0x01AE3FAD.  Keep the temporary delegate alive:
    # the function's common epilogue releases EDI unconditionally.
    checked_write(
        image, pe.rva_to_offset(0x016e3f5d),
        bytes.fromhex("ff5008"), b"\x90" * 3,
        "retain skipped pointer visualization delegate"
    )
    checked_write(
        image, pe.rva_to_offset(0x016e3f60),
        bytes.fromhex("e8db340000"), bytes.fromhex("e948000000"),
        "skip pointer visualization setup"
    )
    # A late optional startup notification can have an empty std::function
    # in the unpackaged host.  The UWP event source normally installs this
    # callback; calling it empty enters std::_Xbad_function_call and Wine's
    # unavailable _invoke_watson implementation.  Treat an empty callback as
    # the intended no-op and return from the two-argument wrapper.
    checked_write(
        image, pe.rva_to_offset(0x016d3002),
        bytes.fromhex("ff1528642a02"), bytes.fromhex("33c05dc20800"),
        "ignore absent optional startup callback"
    )
    # The same optional notification is invoked from the loading-state
    # transition as well.  When its std::function is empty, skip the invoke
    # sequence and continue at the existing local cleanup path.
    checked_write(
        image, pe.rva_to_offset(0x016d42fb),
        bytes.fromhex("ff1528642a02"), bytes.fromhex("e92a00000090"),
        "skip absent loading-state callback"
    )
    # Activation and window-state diagnostics stringify boxed WinRT enums.
    # Wine has an aborting Enum::ToString export. A null HSTRING is the
    # supported empty diagnostic value; retain the surrounding COM releases.
    for rva in (0x016e57bf, 0x016e5bae):
        checked_write(image, pe.rva_to_offset(rva),
                      bytes.fromhex("ff15e06b2a02"),
                      bytes.fromhex("33c090909090"),
                      "omit optional WinRT enum diagnostic")
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
    chakra_descriptor: tuple[int, int] | None = None
    while True:
        descriptor = bytes(image[cursor:cursor + 20])
        values = struct.unpack("<5I", descriptor)
        if not any(values):
            break
        name = pe.c_string_at_rva(values[3]).lower()
        descriptors.append((name, descriptor))
        if name == b"kernel32.dll":
            kernel_descriptor = (values[0] or values[4], values[4])
        elif name == b"chakra.dll":
            chakra_descriptor = (values[0] or values[4], values[4])
        cursor += 20
    if len(descriptors) != 32 or import_size != 0x294:
        raise ValueError(
            f"unexpected import table: {len(descriptors)} descriptors, "
            f"size 0x{import_size:x}"
        )
    if kernel_descriptor is None:
        raise ValueError("KERNEL32 import descriptor not found")
    if chakra_descriptor is None:
        raise ValueError("chakra.dll import descriptor not found")

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
        "DelayLoadFailureHook",
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

    chakra_ilt_rva, chakra_iat_rva = chakra_descriptor
    chakra_ilt_offset = pe.rva_to_offset(chakra_ilt_rva)
    chakra_names: dict[str, tuple[int, int]] = {}
    index = 0
    while True:
        hint_rva = struct.unpack_from(
            "<I", image, chakra_ilt_offset + index * 4
        )[0]
        if not hint_rva:
            break
        if hint_rva & 0x80000000:
            raise ValueError("unexpected ordinal chakra.dll import")
        chakra_names[pe.c_string_at_rva(hint_rva + 2).decode("ascii")] = \
            (index, hint_rva)
        index += 1
    debug_index, debug_hint_rva = chakra_names["JsStartDebugging"]
    chakra_safe_hint_rva = chakra_names["JsAddRef"][1]
    checked_write(
        image, chakra_ilt_offset + debug_index * 4,
        struct.pack("<I", debug_hint_rva),
        struct.pack("<I", chakra_safe_hint_rva),
        "JsStartDebugging loader placeholder"
    )
    debug_iat_rva = chakra_iat_rva + debug_index * 4

    last = pe.sections[-1]
    new_raw_offset = align(len(image), pe.file_alignment)
    new_rva = align(
        last.virtual_address + max(last.virtual_size, last.raw_size),
        pe.section_alignment,
    )
    new_descriptor_count = len(descriptors) + len(compatibility_names) + 3
    section = bytearray(new_descriptor_count * 20)

    def append(data: bytes, alignment: int = 1) -> int:
        section.extend(b"\0" * ((-len(section)) % alignment))
        offset = len(section)
        section.extend(data)
        return offset

    dll_name_offset = append(b"win32craft.dll\0")
    chakra_name_offset = append(b"ChakraCore.dll\0")
    hint_offsets: dict[str, int] = {}
    host_names = ("Win32Bootstrap", "JsStartDebugging") + compatibility_names
    for name in host_names:
        hint_offsets[name] = append(b"\0\0" + name.encode("ascii") + b"\0", 2)
    ilt_offsets: dict[str, int] = {}
    for name in host_names:
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
        elif dll_name == b"chakra.dll":
            values[3] = new_rva + chakra_name_offset
        relocated.append(struct.pack("<5I", *values))
    relocated.append(struct.pack(
        "<5I", new_rva + ilt_offsets["Win32Bootstrap"], 0, 0,
        new_rva + dll_name_offset, new_rva + bootstrap_iat_offset
    ))
    relocated.append(struct.pack(
        "<5I", new_rva + ilt_offsets["JsStartDebugging"], 0, 0,
        new_rva + dll_name_offset, debug_iat_rva
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
        image, pe.rva_to_offset(0x1c471af), bytes.fromhex("50576800004000e83e1b0000"),
        b"\xff\x15" + struct.pack("<I", bootstrap_iat_va) + b"\x90" * 6,
        "WinMain bootstrap call"
    )
    output.write_bytes(image)
    print(f"wrote {output} ({len(image)} bytes)")
    print(f"SHA-256 {hashlib.sha256(image).hexdigest()}")


def patch_renoir_windows7(source: Path, output: Path) -> None:
    """Redirect Renoir's unavailable Windows 8+ API sets to the host."""
    original = source.read_bytes()
    image = bytearray(original)
    pe = Pe32(image)
    import_rva, import_size = struct.unpack_from(
        "<2I", image, pe.optional + 0x68
    )
    if not import_rva or import_size < 20:
        raise ValueError("Renoir image has no import directory")
    found: set[bytes] = set()
    cursor = pe.rva_to_offset(import_rva)
    while True:
        descriptor = struct.unpack_from("<5I", image, cursor)
        if not any(descriptor):
            break
        name_offset = pe.rva_to_offset(descriptor[3])
        name = pe.c_string_at_rva(descriptor[3])
        normalized = name.lower()
        if normalized in RENOIR_WINDOWS7_API_SETS:
            if normalized in found:
                raise ValueError(f"duplicate Renoir import descriptor {name!r}")
            image[name_offset:name_offset + len(name)] = \
                b"win32craft.dll".ljust(len(name), b"\0")
            found.add(normalized)
            print(f"{name.decode()} -> win32craft.dll")
        cursor += 20
    if found != RENOIR_WINDOWS7_API_SETS:
        names = ", ".join(sorted(name.decode() for name in found)) or "none"
        raise ValueError(f"unexpected Renoir API-set group: {names}")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(image)
    print(f"input SHA-256  {hashlib.sha256(original).hexdigest()}")
    print(f"output SHA-256 {hashlib.sha256(image).hexdigest()}")


def patch_cohtml_windows7(source: Path, output: Path) -> None:
    """Move cohtml's LoadPackagedLibrary import to win32craft.dll."""
    original = source.read_bytes()
    image = bytearray(original)
    pe = Pe32(image)
    import_rva, import_size = struct.unpack_from(
        "<2I", image, pe.optional + 0x68
    )
    cursor = pe.rva_to_offset(import_rva)
    descriptors: list[bytes] = []
    target_iat_rva = None
    while True:
        descriptor = bytes(image[cursor:cursor + 20])
        values = struct.unpack("<5I", descriptor)
        if not any(values):
            break
        descriptors.append(descriptor)
        if pe.c_string_at_rva(values[3]).lower() == b"kernel32.dll":
            ilt_rva = values[0] or values[4]
            ilt_offset = pe.rva_to_offset(ilt_rva)
            imports: dict[str, tuple[int, int]] = {}
            index = 0
            while True:
                hint_rva = struct.unpack_from(
                    "<I", image, ilt_offset + index * 4
                )[0]
                if not hint_rva:
                    break
                if hint_rva & 0x80000000:
                    raise ValueError("unexpected ordinal KERNEL32 import")
                imports[pe.c_string_at_rva(hint_rva + 2).decode("ascii")] = \
                    (index, hint_rva)
                index += 1
            target_index, target_hint = imports["LoadPackagedLibrary"]
            safe_hint = imports["FreeLibrary"][1]
            position = ilt_offset + target_index * 4
            if struct.unpack_from("<I", image, position)[0] != target_hint:
                raise ValueError("LoadPackagedLibrary thunk changed unexpectedly")
            struct.pack_into("<I", image, position, safe_hint)
            target_iat_rva = values[4] + target_index * 4
        cursor += 20
    if target_iat_rva is None:
        raise ValueError("LoadPackagedLibrary import was not found")

    last = pe.sections[-1]
    new_raw_offset = align(len(image), pe.file_alignment)
    new_rva = align(
        last.virtual_address + max(last.virtual_size, last.raw_size),
        pe.section_alignment,
    )
    descriptor_size = (len(descriptors) + 2) * 20
    section = bytearray(descriptor_size)

    def append(data: bytes, alignment: int = 1) -> int:
        section.extend(b"\0" * ((-len(section)) % alignment))
        offset = len(section)
        section.extend(data)
        return offset

    dll_offset = append(b"win32craft.dll\0")
    hint_offset = append(b"\0\0LoadPackagedLibrary\0", 2)
    ilt_offset = append(struct.pack("<2I", new_rva + hint_offset, 0), 4)
    relocated = descriptors + [struct.pack(
        "<5I", new_rva + ilt_offset, 0, 0,
        new_rva + dll_offset, target_iat_rva
    ), b"\0" * 20]
    section[:descriptor_size] = b"".join(relocated)
    virtual_size = len(section)
    raw_size = align(virtual_size, pe.file_alignment)
    section.extend(b"\0" * (raw_size - virtual_size))
    image.extend(b"\0" * (new_raw_offset - len(image)))
    image.extend(section)

    new_header = pe.section_table + pe.section_count * 40
    size_of_headers = struct.unpack_from("<I", image, pe.optional + 0x3C)[0]
    if new_header + 40 > size_of_headers:
        raise ValueError("no room for an additional section header")
    struct.pack_into(
        "<8s6I2HI", image, new_header, b".w7imp\0\0", virtual_size,
        new_rva, raw_size, new_raw_offset, 0, 0, 0, 0, 0xC0000040
    )
    struct.pack_into("<H", image, pe.file_header + 2, pe.section_count + 1)
    initialized = struct.unpack_from("<I", image, pe.optional + 8)[0]
    struct.pack_into("<I", image, pe.optional + 8, initialized + raw_size)
    struct.pack_into(
        "<I", image, pe.optional + 0x38,
        align(new_rva + virtual_size, pe.section_alignment)
    )
    struct.pack_into(
        "<2I", image, pe.optional + 0x68,
        new_rva, (len(descriptors) + 2) * 20
    )
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(image)
    print("LoadPackagedLibrary -> win32craft.dll")
    print(f"input SHA-256  {hashlib.sha256(original).hexdigest()}")
    print(f"output SHA-256 {hashlib.sha256(image).hexdigest()}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?", type=Path,
                        default=Path("Minecraft.Windows.exe"))
    parser.add_argument("output", nargs="?", type=Path,
                        default=Path("Win32Craft.1.16.20.0.3.exe"))
    args = parser.parse_args()
    patch(args.source, args.output)


if __name__ == "__main__":
    main()
