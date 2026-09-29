int __fastcall tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  uint32_t v5; // r4
  uint32_t v6; // r11
  uint32_t v7; // r5
  uint32_t v8; // r6
  char *v9; // r7
  uint32_t v10; // r3
  int size; // r7
  _BYTE v14[512]; // [sp+8h] [bp-204h] BYREF
  char v15; // [sp+208h] [bp-4h] BYREF

  v5 = offset;
  if ( offset >= 0x200 || lba >= 0x800 || offset + bufsize > (2048 - lba) << 9 )
    return -1;
  if ( bufsize )
  {
    v6 = 0x38C9CDA0 * lba - 0x61C85616;
    v7 = 32 * lba;
    v8 = bufsize;
    do
    {
      memcpy(v14, (16 * v7 + 269484032), sizeof(v14));
      v9 = v14;
      v10 = v6;
      do
      {
        *(v9 + 1) ^= ((v10 - 0x255992ED) >> 24)
                   | ((v10 + 0x78DDE6C4) >> 24 << 8)
                   | ((v10 + 0x17156075) >> 24 << 16)
                   | ((v10 - 1253254618) >> 24 << 24);
        *(v9 + 2) ^= ((v10 + 1401181143) >> 24)
                   | ((v10 - 239350392) >> 24 << 8)
                   | ((v10 - 1879881927) >> 24 << 16)
                   | ((v10 + 774553834) >> 24 << 24);
        *(v9 + 3) ^= ((v10 - 865977701) >> 24)
                   | ((v10 + 1788458060) >> 24 << 8)
                   | ((v10 + 147926525) >> 24 << 16)
                   | ((v10 - 1492605010) >> 24 << 24);
        *v9 ^= ((v10 + 0x3C6EF362) >> 24 << 24)
             | ((v10 - 0x61C8864F) >> 24 << 16)
             | (HIBYTE(v10) << 8)
             | ((v10 + 0x61C8864F) >> 24);
        v9 += 16;
        v10 += 0x41C64E6D;
      }
      while ( v9 != &v15 );
      size = 512 - v5;
      if ( 512 - v5 > v8 )
        size = v8;
      memcpy(buffer, &v14[v5], size);
      buffer = buffer + size;
      v6 += 0x38C9CDA0;
      v5 = 0;
      v8 -= size;
      v7 += 32;
    }
    while ( v8 );
  }
  return bufsize;
}
