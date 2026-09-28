use std::env;
use std::fs;
use std::process;


// параметры псевдокода

// база адресного пространства дампа
const FLASH_BASE: u32 = 0x1000_0000;

// адрес прошивки
const ENCRYPTED_ADDR: u32 = 0x1010_0000;

// смещение зашифрованной прошивки
const ENCRYPTED_OFFSET: usize = (ENCRYPTED_ADDR - FLASH_BASE) as usize;

// размер LBA
const LBA_SIZE: usize = 512;

// всего LBA
const LBA_COUNT: usize = 2048;

// размер шифрованной области
const ENCRYPTED_SIZE: usize = LBA_COUNT * LBA_SIZE;


// КОНСТАНТЫ

const LCG_SEED_MUL: u32 = 0x38C9_CDA0;
const LCG_SEED_SUB: u32 = 0x61C8_5616;
const LCG_STEP: u32 = 0x41C6_4E6D;

// таблица масок

#[derive(Clone, Copy)]
struct MaskByte {
    constant: Option<u32>,
    sign: i32,
    shift: u32,
}

// маски для всех четырёх dword'ов одного 16-байтового блока

const MASKS: [[MaskByte; 4]; 4] = [
    // dword 0, для него байты в обратном порядке
    [
        MaskByte { constant: Some(0x3C6E_F362), sign:  1, shift: 24 },
        MaskByte { constant: Some(0x61C8_864F), sign: -1, shift: 16 },
        MaskByte { constant: None,              sign:  0, shift:  8 }, // HIBYTE(v10)
        MaskByte { constant: Some(0x61C8_864F), sign:  1, shift:  0 },
    ],
    // dword 1
    [
        MaskByte { constant: Some(0x2559_92ED), sign: -1, shift:  0 },
        MaskByte { constant: Some(0x78DD_E6C4), sign:  1, shift:  8 },
        MaskByte { constant: Some(0x1715_6075), sign:  1, shift: 16 },
        MaskByte { constant: Some(0x4AC6_E0BA), sign: -1, shift: 24 },
    ],
    // dword 2
    [
        MaskByte { constant: Some(0x538C_4D77), sign:  1, shift:  0 },
        MaskByte { constant: Some(0x0E41_D2B8), sign: -1, shift:  8 },
        MaskByte { constant: Some(0x7003_6D87), sign: -1, shift: 16 },
        MaskByte { constant: Some(0x2E2B_B1DA), sign:  1, shift: 24 },
    ],
    // dword 3
    [
        MaskByte { constant: Some(0x33A0_F7AB), sign: -1, shift:  0 },
        MaskByte { constant: Some(0x6A8E_984C), sign:  1, shift:  8 },
        MaskByte { constant: Some(0x08CE_A50D), sign:  1, shift: 16 },
        MaskByte { constant: Some(0x58F8_4F2E), sign: -1, shift: 24 },
    ],
];

// расшифровка

// 32-битная XOR-маска для одного dword'а по текущему v10.
#[inline]
fn compute_mask(v10: u32, dword_idx: usize) -> u32 {
    let mut mask: u32 = 0;
    for mb in &MASKS[dword_idx] {
        let hi_byte = match mb.constant {
            None => (v10 >> 24) & 0xFF,
            Some(c) => {
                let t = if mb.sign > 0 {
                    v10.wrapping_add(c)
                } else {
                    v10.wrapping_sub(c)
                };
                (t >> 24) & 0xFF
            }
        };
        mask |= (hi_byte & 0xFF) << mb.shift;
    }
    mask
}

// расшифровка одного блока
fn decrypt_lba(lba: u32, encrypted: &[u8]) -> [u8; LBA_SIZE] {
    debug_assert_eq!(encrypted.len(), LBA_SIZE);

    let mut v10 = LCG_SEED_MUL
        .wrapping_mul(lba)
        .wrapping_sub(LCG_SEED_SUB);

    let mut out = [0u8; LBA_SIZE];
    out.copy_from_slice(encrypted);

    // 32 блока по 16 байт, в каждом блоке 4 dword'а
    for blk in 0..32 {
        for d in 0..4 {
            let mask = compute_mask(v10, d);
            let off = blk * 16 + d * 4;
            let dw = u32::from_le_bytes([
                out[off],
                out[off + 1],
                out[off + 2],
                out[off + 3],
            ]) ^ mask;
            out[off..off + 4].copy_from_slice(&dw.to_le_bytes());
        }
        v10 = v10.wrapping_add(LCG_STEP);
    }

    out
}

// расшифровка произвольного куска данных
fn decrypt_region(encrypted: &[u8], start_lba: u32) -> Vec<u8> {
    assert!(
        encrypted.len() % LBA_SIZE == 0,
        "длина должна быть кратна 512 байтам"
    );

    let mut out = Vec::with_capacity(encrypted.len());
    for i in 0..(encrypted.len() / LBA_SIZE) {
        let lba = start_lba + i as u32;
        let block = &encrypted[i * LBA_SIZE..(i + 1) * LBA_SIZE];
        out.extend_from_slice(&decrypt_lba(lba, block));
    }
    out
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let in_path = &args[1];
    let out_path = &args[2];

    let data = fs::read(in_path).unwrap_or_else(|e| {
        eprintln!("Не удалось прочитать {}: {}", in_path, e);
        process::exit(1);
    });

    let enc_start = ENCRYPTED_OFFSET;
    let enc_end = (enc_start + ENCRYPTED_SIZE).min(data.len());
    let enc = &data[enc_start..enc_end];
    let enc = &enc[..(enc.len() / LBA_SIZE) * LBA_SIZE];

    let dec = decrypt_region(enc, 0);

    fs::write(out_path, &dec).unwrap_or_else(|e| {
        eprintln!("Не удалось записать {}: {}", out_path, e);
        process::exit(1);
    });
}