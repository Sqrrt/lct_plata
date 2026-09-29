from pathlib import Path
import struct
import sys


FLASH_DISK_OFFSET = 0x100000
DEFAULT_DISK_SIZE = 0xB0000
SECTOR_SIZE = 512


def decrypt_sector(data, lba):
    out = bytearray(data)
    state = (0x38C9CDA0 * lba - 0x61C85616) & 0xFFFFFFFF

    for block in range(0, SECTOR_SIZE, 16):
        key1 = (
            ((state - 0x255992ED) >> 24)
            | (((state + 0x78DDE6C4) >> 24) << 8)
            | (((state + 0x17156075) >> 24) << 16)
            | (((state - 0x4AB325DA) >> 24) << 24)
        ) & 0xFFFFFFFF
        key2 = (
            ((state + 0x538453D7) >> 24)
            | (((state - 0x0E443278) >> 24) << 8)
            | (((state - 0x700CB8C7) >> 24) << 16)
            | (((state + 0x2E2AC0EA) >> 24) << 24)
        ) & 0xFFFFFFFF
        key3 = (
            ((state - 0x339DC565) >> 24)
            | (((state + 0x6A99B44C) >> 24) << 8)
            | (((state + 0x08D12DFD) >> 24) << 16)
            | (((state - 0x58F75852) >> 24) << 24)
        ) & 0xFFFFFFFF
        key4 = (
            (((state + 0x3C6EF362) >> 24) << 24)
            | (((state - 0x61C8864F) >> 24) << 16)
            | ((state >> 24) << 8)
            | ((state + 0x61C8864F) >> 24)
        ) & 0xFFFFFFFF

        for offset, key in ((0, key4), (4, key1), (8, key2), (12, key3)):
            value = struct.unpack_from("<I", out, block + offset)[0]
            struct.pack_into("<I", out, block + offset, value ^ key)

        state = (state + 0x41C64E6D) & 0xFFFFFFFF

    return out


def main():
    source = Path(sys.argv[1] if len(sys.argv) > 1 else "pt/full_flash.bin")
    output = Path(sys.argv[2] if len(sys.argv) > 2 else "pt/attack/memory/decrypted_blob.bin")
    disk_size = int(sys.argv[3], 0) if len(sys.argv) > 3 else DEFAULT_DISK_SIZE
    blob = source.read_bytes()[FLASH_DISK_OFFSET:FLASH_DISK_OFFSET + disk_size]

    if len(blob) != disk_size or disk_size % SECTOR_SIZE:
        raise SystemExit("input does not contain the complete encrypted blob")

    decrypted = bytearray()
    for offset in range(0, len(blob), SECTOR_SIZE):
        lba = offset // SECTOR_SIZE
        decrypted.extend(decrypt_sector(blob[offset:offset + SECTOR_SIZE], lba))

    output.write_bytes(decrypted)
    print(f"wrote {len(decrypted)} bytes to {output}")


if __name__ == "__main__":
    main()
