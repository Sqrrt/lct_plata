import ida_auto
import ida_funcs
import ida_hexrays
import ida_name
import idaapi
import idautils

ida_auto.auto_wait()

print("=== names containing unlock/open/carousel ===")
for ea, name in idautils.Names():
    low = name.lower()
    if any(word in low for word in ("open", "sesame", "carousel", "code")):
        print(hex(ea), name)

print("=== functions mentioning unlock/state/USB ===")
if ida_hexrays.init_hexrays_plugin():
    for func_ea in idautils.Functions():
        try:
            cfunc = ida_hexrays.decompile(func_ea)
            text = str(cfunc)
        except Exception:
            continue
        low = text.lower()
        if any(word in low for word in ("open_sesame", "open_sasame", "0x2000118c", "50110000", "tud_init")):
            print("---", hex(func_ea), ida_name.get_name(func_ea), "---")
            print(text)

print("=== xrefs to state address ===")
state_ea = 0x2000118C
for xref in idautils.XrefsTo(state_ea, 0):
    print(hex(xref.frm), ida_name.get_name(xref.frm), xref.type)

idaapi.qexit(0)
