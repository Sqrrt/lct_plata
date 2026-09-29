int __fastcall decrypt(int a1, unsigned int a2, unsigned int a3, int a4, unsigned int a5)
{
  unsigned int v5; // r4
  int v6; // r11
  int v7; // r5
  unsigned int v8; // r6
  char *v9; // r7
  unsigned int v10; // r3
  unsigned int v11; // r7
  _BYTE v14[512]; // [sp+8h] [bp-204h] BYREF
  char v15; // [sp+208h] [bp-4h] BYREF

  v5 = a3;
  if ( a3 >= 0x200 || a2 >= 0x800 || a3 + a5 > (2048 - a2) << 9 )
    return -1;
  if ( a5 )
  {
    v6 = 0x38C9CDA0 * a2 - 0x61C85616;
    v7 = 32 * a2;
    v8 = a5;
    do
    {
      sub_10002E1C(v14);
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
        *v9 ^= ((v10 + 1013904226) >> 24 << 24)
             | ((v10 - 1640531535) >> 24 << 16)
             | (HIBYTE(v10) << 8)
             | ((v10 + 1640531535) >> 24);
        v9 += 16;
        v10 += 0x41C64E6D;
      }
      while ( v9 != &v15 );
      v11 = 512 - v5;
      if ( 512 - v5 > v8 )
        v11 = v8;
      sub_10002E1C(a4);
      a4 += v11;
      v6 += 0x38C9CDA0;
      v5 = 0;
      v8 -= v11;
      v7 += 32;
    }
    while ( v8 );
  }
  return a5;
}
