"""Reapply known IDA types after a database crash.

Run this with IDA's File -> Script file command. It changes IDA metadata only;
it does not modify firmware bytes.
"""

from pathlib import Path

import ida_bytes
import ida_funcs
import idaapi
import ida_kernwin
import ida_name
import ida_segment
import ida_typeinf
import idc


SRAM_START = 0x20000000
SRAM_END = 0x20042000
EPBUF = 0x200015B8
MSCD_ITF = 0x200017B8
XFER_CB = 0x1000467C
SCSI_CALLBACK = 0x10000894
CODE_LEDS = 0x10005504


def log(message):
    print("[recover_ida_state] " + message)


def ensure_sram_segment():
    seg = ida_segment.getseg(SRAM_START)
    if seg is None:
        new_seg = ida_segment.segment_t()
        new_seg.start_ea = SRAM_START
        new_seg.end_ea = SRAM_END
        new_seg.bitness = 1  # 32-bit
        new_seg.perm = ida_segment.SEGPERM_READ | ida_segment.SEGPERM_WRITE
        if not ida_segment.add_segm_ex(new_seg, "SRAM", "DATA", 0):
            raise RuntimeError("could not create SRAM segment")
    elif seg.end_ea < SRAM_END:
        try:
            ida_segment.set_segm_end(seg.start_ea, SRAM_END, ida_segment.SEGMOD_KEEP)
        except TypeError:
            ida_segment.set_segm_end(seg.start_ea, SRAM_END)


def parse_headers():
    root = Path(__file__).resolve().parent
    paths = [
        root / "ida_tinyusb_types.h",
        root / "ida_tinyusb_msc.h",
        root / "ida_tinyusb_api.h",
    ]
    text = "\n".join(path.read_text(encoding="ascii") for path in paths)
    parse_types = getattr(ida_typeinf, "idc_parse_types", None)
    if parse_types is None:
        parse_types = idaapi.idc_parse_types
    parse_flags = getattr(idaapi, "PT_SILENT", 0)
    result = parse_types(text, parse_flags)
    if result not in (None, 0):
        log("type parser returned %r" % result)


def named_type(name):
    tif = ida_typeinf.tinfo_t()
    if not tif.get_named_type(idaapi.get_idati(), name):
        raise RuntimeError("type not found: " + name)
    return tif


def apply_struct(ea, type_name, size, name):
    ida_bytes.del_items(ea, ida_bytes.DELIT_SIMPLE, size)
    if not ida_typeinf.apply_tinfo(ea, named_type(type_name), ida_typeinf.TINFO_DEFINITE):
        raise RuntimeError("could not apply %s at %#x" % (type_name, ea))
    ida_name.set_name(ea, name, ida_name.SN_FORCE)


def apply_function(ea, declaration, name):
    ida_funcs.add_func(ea)
    if not idc.SetType(ea, declaration):
        log("could not apply function type at %#x" % ea)
    ida_name.set_name(ea, name, ida_name.SN_FORCE)


def apply_code_led_table():
    ida_bytes.del_items(CODE_LEDS, ida_bytes.DELIT_SIMPLE, 4)
    if not idc.SetType(CODE_LEDS, "unsigned char code_leds[4];"):
        log("could not apply code_leds array type")
    ida_name.set_name(CODE_LEDS, "code_leds", ida_name.SN_FORCE)


def main():
    ensure_sram_segment()
    parse_headers()

    # Separate endpoint buffer and MSC interface state object.
    apply_struct(EPBUF, "mscd_ep_buffer_t", 512, "_mscd_epbuf")
    apply_struct(MSCD_ITF, "mscd_interface_t", 64, "_mscd_itf")
    apply_code_led_table()

    apply_function(
        XFER_CB,
        "bool __fastcall mscd_xfer_cb(uint8_t rhport, uint8_t ep_addr, "
        "xfer_result_t event, uint32_t xferred_bytes);",
        "mscd_xfer_cb",
    )
    apply_function(
        SCSI_CALLBACK,
        "int32_t __fastcall tud_msc_scsi_cb(uint8_t lun, "
        "const uint8_t scsi_command[16], void *buffer, uint16_t buffer_size);",
        "tud_msc_scsi_cb",
    )

    ida_kernwin.refresh_idaview_anyway()
    log("SRAM, TinyUSB structures, names, and prototypes reapplied")


if __name__ == "__main__":
    main()
