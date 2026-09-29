#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define FLASH_XIP_DISK_BASE 0x10100000u
#define SECTOR_SIZE         512u
#define SECTOR_COUNT        0x800u

/* Candidate reconstruction of the decompiled callback.
 * Confirm the four STRB instructions in the original assembly. */
int32_t tud_msc_read10_cb_candidate(uint8_t lun, uint32_t lba,
                                    uint32_t offset, void *buffer,
                                    uint32_t bufsize)
{
    uint8_t *out = (uint8_t *)buffer;
    uint32_t pos = offset;
    uint32_t remaining = bufsize;

    (void)lun;

    if (offset >= SECTOR_SIZE || lba >= SECTOR_COUNT ||
        (uint64_t)offset + bufsize >
            (uint64_t)(SECTOR_COUNT - lba) * SECTOR_SIZE) {
        return -1;
    }

    while (remaining != 0) {
        uint8_t sector[SECTOR_SIZE];
        uint32_t state = 0x38C9CDA0u * lba - 0x61C85616u;
        size_t copy_size;

        memcpy(sector,
               (const void *)(FLASH_XIP_DISK_BASE + lba * SECTOR_SIZE),
               sizeof(sector));

        for (size_t block = 0; block < SECTOR_SIZE; block += 16) {
            uint32_t key1 = ((state - 0x255992EDu) >> 24)
                          | (((state + 0x78DDE6C4u) >> 24) << 8)
                          | (((state + 0x17156075u) >> 24) << 16)
                          | (((state - 0x4AB325DAu) >> 24) << 24);
            uint32_t key2 = ((state + 0x538453D7u) >> 24)
                          | (((state - 0x0E443278u) >> 24) << 8)
                          | (((state - 0x700CB8C7u) >> 24) << 16)
                          | (((state + 0x2E2AC0EAu) >> 24) << 24);
            uint32_t key3 = ((state - 0x339DC565u) >> 24)
                          | (((state + 0x6A99B44Cu) >> 24) << 8)
                          | (((state + 0x08D12DFDu) >> 24) << 16)
                          | (((state - 0x58F75852u) >> 24) << 24);
            uint32_t key4 = (((state + 0x3C6EF362u) >> 24) << 24)
                          | (((state - 0x61C8864Fu) >> 24) << 16)
                          | ((state >> 24) << 8)
                          | ((state + 0x61C8864Fu) >> 24);

            *(uint32_t *)&sector[block + 0] ^= key4;
            *(uint32_t *)&sector[block + 4] ^= key1;
            *(uint32_t *)&sector[block + 8] ^= key2;
            *(uint32_t *)&sector[block + 12] ^= key3;
            state += 0x41C64E6Du;
        }

        copy_size = SECTOR_SIZE - pos;
        if (copy_size > remaining) {
            copy_size = remaining;
        }

        memcpy(out, sector + pos, copy_size);
        out += copy_size;
        remaining -= (uint32_t)copy_size;
        pos = 0;
        ++lba;
    }

    return (int32_t)bufsize;
}
