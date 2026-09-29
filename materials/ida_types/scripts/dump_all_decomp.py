import os

import idaapi
import ida_hexrays
import ida_funcs
import idautils
import idc


if not ida_hexrays.init_hexrays_plugin():
    raise RuntimeError("Hex-Rays decompiler is unavailable")

database = idaapi.get_input_file_path()
output = os.path.splitext(database)[0] + ".decomp.c"

count = 0
failed = 0
with open(output, "w", encoding="utf-8") as stream:
    stream.write("/* IDA Hex-Rays decompilation dump */\n\n")

    for ea in idautils.Functions():
        name = idc.get_func_name(ea)
        stream.write("/* ------------------------------------------------------------------ */\n")
        stream.write("/* %s @ 0x%X */\n" % (name, ea))
        stream.write("/* ------------------------------------------------------------------ */\n")

        try:
            cfunc = ida_hexrays.decompile(ea)
            stream.write(str(cfunc))
            stream.write("\n\n")
            count += 1
        except Exception as error:
            stream.write("/* DECOMPILATION FAILED: %s */\n\n" % error)
            failed += 1

print("Wrote %d functions to %s (%d failed)" % (count, output, failed))
