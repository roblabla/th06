import coff
from pathlib import Path
import struct

SCRIPTS_DIR = Path(__file__).parent


def rename_symbols(filename):
    out_folder = SCRIPTS_DIR.parent / "build" / "objdiff" / "src"
    asm_folder = SCRIPTS_DIR.parent / "build" / "objdiff" / "asm"

    class_name = filename.removesuffix(".obj")
    obj = coff.ObjectModule()
    with open(Path("build") / filename, "rb") as f:
        obj.unpack(f.read(), 0)

    # We filter to only the symbols with the namespace=filename, and we scrape everything but the function name
    # and we save all the renames onto syms.txt
    # TODO: Implement constructors/destructors
    seen = {}
    for sym_obj in obj.symbols:
        sym = sym_obj.get_name(obj.string_table)
        if seen.get(sym, False):
            continue
        seen[sym] = True

        parts = sym.split(b"@")
        if len(parts) > 1:
            func_name = parts[0].replace(b"?", b"", 2)
            namespace = parts[1]
            if class_name.encode("utf8") not in sym:
                continue
            if class_name == func_name[1:]:
                func_name = (b"~" if func_name[0] == b"1" else b"") + func_name[1:]
            elif namespace != class_name.encode("utf8"):
                continue

            offset = obj.string_table.append(func_name)
            sym_obj.name = b"\0\0\0\0" + struct.pack("I", offset)

    if not out_folder.exists():
        out_folder.mkdir(parents=True, exist_ok=True)
        asm_folder.mkdir(parents=True, exist_ok=True)

    with open(out_folder / filename, "wb") as f:
        f.write(obj.get_buffer())
