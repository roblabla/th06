# Somewhat bafflingly, link.exe does not provide a (documented) way to
# override the PE header timestamp. So this script does exactly that.

import sys


def patch_timestamp(exe_path, timestamp_value):
    timestamp = timestamp_value.to_bytes(4, byteorder="little")
    with open(exe_path, "r+b") as exe:
        # Offset for pointer to PE header
        exe.seek(0x3C)
        pe_pointer = exe.read(4)
        if len(pe_pointer) != 4:
            raise ValueError("Missing PE header pointer")
        pe_offset = int.from_bytes(pe_pointer, byteorder="little")
        exe.seek(pe_offset)
        header = exe.read(12)
        if len(header) != 12 or header[:4] != b"PE\0\0":
            raise ValueError("Invalid PE header")
        # TimeDateStamp is +0x8 from base of PE header
        exe.seek(pe_offset + 0x8)
        exe.write(timestamp)


if __name__ == "__main__":
    patch_timestamp(sys.argv[1], int(sys.argv[2]))
