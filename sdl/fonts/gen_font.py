#!/usr/bin/env python3
# XEvil 2.5 SDL port -- bitmap font generator.
#
# Converts a classic public-domain X11 bitmap font (6x13, 9x15, ...) into a
# compiled-in C array so the SDL frontend needs NO SDL_ttf / freetype / runtime
# font files.  The metrics we emit (cell width, ascent, descent) mirror the
# X11 XFontStruct->max_bounds idiom the panels rely on, so TextPanel::get_unit
# math ports 1:1  (width = max_bounds.width*cols, height = (ascent+descent)*rows).
#
# Pipeline:  *.pcf.gz  --pcf2bdf-->  BDF text  --(this script)-->  C header.
#
# Usage:
#   ./gen_font.py                 # regenerate font6x13.h + font9x15.h from the
#                                 # standard /usr/share/fonts/X11/misc PCFs
#   ./gen_font.py <in.pcf[.gz]|in.bdf> <var_name> <out.h> [firstcp] [lastcp]
#
# The generated header defines a `static const BitmapFont <var_name>` plus its
# glyph-bit blob.  Only font.cpp includes these headers, so the internal-linkage
# `static const` never collides.

import sys
import os
import subprocess
import tempfile

DEFAULT_FIRST = 32    # space
DEFAULT_LAST  = 126   # tilde  (printable ASCII; em/en dashes are remapped to
                      # '-' at draw time, see sdl/font.cpp)


def load_bdf(path):
    """Return raw BDF text, running pcf2bdf if given a .pcf/.pcf.gz file."""
    if path.endswith(".bdf"):
        with open(path, "r", errors="replace") as f:
            return f.read()
    # PCF (optionally gzipped) -> BDF via pcf2bdf.
    out = subprocess.check_output(["pcf2bdf", path])
    return out.decode("latin-1", errors="replace")


def parse_bdf(text):
    """Parse a BDF into (font_meta, {codepoint: glyph}).

    font_meta = dict(bbw,bbh,bbxoff,bbyoff,ascent,descent,name)
    glyph     = dict(bbw,bbh,bbxoff,bbyoff,dwidth,rows=[int,...])  rows MSB-first
    """
    meta = {"ascent": None, "descent": None, "name": ""}
    glyphs = {}
    lines = text.splitlines()
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i].strip()
        if line.startswith("FONT ") and not meta["name"]:
            meta["name"] = line[5:].strip()
        elif line.startswith("FONTBOUNDINGBOX"):
            p = line.split()
            meta["bbw"], meta["bbh"] = int(p[1]), int(p[2])
            meta["bbxoff"], meta["bbyoff"] = int(p[3]), int(p[4])
        elif line.startswith("FONT_ASCENT"):
            meta["ascent"] = int(line.split()[1])
        elif line.startswith("FONT_DESCENT"):
            meta["descent"] = int(line.split()[1])
        elif line.startswith("STARTCHAR"):
            g = {"dwidth": None}
            enc = None
            i += 1
            while i < n and not lines[i].strip().startswith("BITMAP"):
                l = lines[i].strip()
                if l.startswith("ENCODING"):
                    enc = int(l.split()[1])
                elif l.startswith("DWIDTH"):
                    g["dwidth"] = int(l.split()[1])
                elif l.startswith("BBX"):
                    p = l.split()
                    g["bbw"], g["bbh"] = int(p[1]), int(p[2])
                    g["bbxoff"], g["bbyoff"] = int(p[3]), int(p[4])
                i += 1
            # now lines[i] == BITMAP
            i += 1
            rows = []
            while i < n and not lines[i].strip().startswith("ENDCHAR"):
                hexrow = lines[i].strip()
                if hexrow:
                    rows.append(int(hexrow, 16))
                i += 1
            g["rows"] = rows
            if enc is not None and enc >= 0:
                glyphs[enc] = g
        i += 1
    return meta, glyphs


def glyph_cell_bits(meta, g, cellW, cellH, bytes_per_row):
    """Render one glyph into a fixed cellW x cellH grid, honoring BBX offsets.

    Returns a flat list of ints, cellH * bytes_per_row bytes, MSB-first, where
    row 0 is the top of the cell (cell top == baseline - ascent).
    """
    ascent = meta["ascent"]
    # A 2D boolean grid, [row][col].
    grid = [[0] * cellW for _ in range(cellH)]
    if g is not None and g.get("rows"):
        bw, bh = g["bbw"], g["bbh"]
        bxoff, byoff = g["bbxoff"], g["bbyoff"]
        src_bytes_per_row = (bw + 7) // 8
        # Top of glyph within the cell (0 == cell top).
        top = ascent - (bh + byoff)
        left = bxoff - meta["bbxoff"]
        for r, rowval in enumerate(g["rows"]):
            cy = top + r
            if cy < 0 or cy >= cellH:
                continue
            # rowval is MSB-first across src_bytes_per_row*8 bits.
            total_bits = src_bytes_per_row * 8
            for c in range(bw):
                bit = (rowval >> (total_bits - 1 - c)) & 1
                if not bit:
                    continue
                cx = left + c
                if 0 <= cx < cellW:
                    grid[cy][cx] = 1
    # Pack grid into bytes, MSB-first, bytes_per_row per row.
    out = []
    for row in grid:
        for b in range(bytes_per_row):
            val = 0
            for bit in range(8):
                col = b * 8 + bit
                if col < cellW and row[col]:
                    val |= (1 << (7 - bit))
            out.append(val)
    return out


def generate(in_path, var_name, out_path, first_cp, last_cp):
    text = load_bdf(in_path)
    meta, glyphs = parse_bdf(text)
    if meta["ascent"] is None or meta["descent"] is None:
        raise SystemExit("BDF missing FONT_ASCENT/FONT_DESCENT")
    cellW = meta["bbw"]
    cellH = meta["ascent"] + meta["descent"]
    if cellH != meta["bbh"]:
        # Non-standard; trust ascent+descent for the cell so the baseline math
        # stays exact.  (Full-cell fonts like 6x13/9x15 satisfy cellH==bbh.)
        pass
    bytes_per_row = (cellW + 7) // 8
    num = last_cp - first_cp + 1

    blob = []
    missing = []
    for cp in range(first_cp, last_cp + 1):
        g = glyphs.get(cp)
        if g is None:
            missing.append(cp)
        blob.extend(glyph_cell_bits(meta, g, cellW, cellH, bytes_per_row))

    guard = "SDL_" + var_name.upper() + "_H"
    with open(out_path, "w") as f:
        f.write("// Generated by sdl/fonts/gen_font.py -- DO NOT EDIT BY HAND.\n")
        f.write("// Source X11 font: %s\n" % meta["name"])
        f.write("// cell %dx%d  ascent %d  descent %d  codepoints %d..%d\n"
                % (cellW, cellH, meta["ascent"], meta["descent"], first_cp, last_cp))
        f.write("#ifndef %s\n#define %s\n\n" % (guard, guard))
        f.write('#include "font.h"\n\n')
        f.write("static const unsigned char %s_bits[] = {\n" % var_name)
        per_glyph = cellH * bytes_per_row
        for gi in range(num):
            cp = first_cp + gi
            chunk = blob[gi * per_glyph:(gi + 1) * per_glyph]
            ch = chr(cp) if 32 <= cp < 127 else "?"
            f.write("  " + ",".join("0x%02x" % b for b in chunk)
                    + ",  /* %d '%s' */\n" % (cp, ch if ch != "\\" else "\\\\"))
        f.write("};\n\n")
        f.write("static const BitmapFont %s = {\n" % var_name)
        f.write("  %d, %d,  /* cellW, cellH */\n" % (cellW, cellH))
        f.write("  %d, %d,  /* ascent, descent */\n" % (meta["ascent"], meta["descent"]))
        f.write("  %d, %d,  /* firstChar, numChars */\n" % (first_cp, num))
        f.write("  %d,      /* bytesPerRow */\n" % bytes_per_row)
        f.write("  %s_bits\n" % var_name)
        f.write("};\n\n")
        f.write("#endif\n")
    print("wrote %s: cell %dx%d ascent %d descent %d, %d glyphs (%d bytes)%s"
          % (out_path, cellW, cellH, meta["ascent"], meta["descent"], num,
             len(blob), "" if not missing else ", missing cp: %r" % missing))


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    if len(sys.argv) == 1:
        # Regenerate the two shipped fonts from the standard system PCFs.
        jobs = [
            ("/usr/share/fonts/X11/misc/6x13-ISO8859-1.pcf.gz", "font6x13",
             os.path.join(here, "font6x13.h")),
            ("/usr/share/fonts/X11/misc/9x15-ISO8859-1.pcf.gz", "font9x15",
             os.path.join(here, "font9x15.h")),
        ]
        for pcf, var, out in jobs:
            if not os.path.exists(pcf):
                raise SystemExit("missing system font: %s (install xfonts-base)" % pcf)
            generate(pcf, var, out, DEFAULT_FIRST, DEFAULT_LAST)
        return
    if len(sys.argv) < 4:
        raise SystemExit(__doc__)
    in_path = sys.argv[1]
    var_name = sys.argv[2]
    out_path = sys.argv[3]
    first_cp = int(sys.argv[4]) if len(sys.argv) > 4 else DEFAULT_FIRST
    last_cp = int(sys.argv[5]) if len(sys.argv) > 5 else DEFAULT_LAST
    generate(in_path, var_name, out_path, first_cp, last_cp)


if __name__ == "__main__":
    main()
