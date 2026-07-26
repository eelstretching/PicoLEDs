#!/usr/bin/env python3
"""
bdf_to_header.py

Convert a BDF (Bitmap Distribution Format) font file into a C header
file suitable for embedding in a microcontroller project (e.g. Raspberry
Pi Pico / Pico 2W).

Usage:
    python3 bdf_to_header.py input.bdf output.h [--name FONT_NAME]

This also writes (or overwrites) a small "font_common.h" next to the
output file. It defines the shared layout used by every font this tool
generates: an enum of indices into each font's metadata array, and a
`bdf_font_t` struct that bundles a font's three arrays together so
multiple fonts can be handled generically (e.g. in an array of
`const bdf_font_t *`) without any macros to look up. Every generated
font header #includes it, and it only needs to be generated once even
if you convert several fonts into the same project.

The generated font-specific header defines:

  - `static const int <prefix>_meta[FONT_META_COUNT]`
        The font's overall metrics, indexed using the enum from
        font_common.h (FONT_META_BBX_WIDTH, FONT_META_BBX_HEIGHT,
        FONT_META_BBX_XOFF, FONT_META_BBX_YOFF, FONT_META_ASCENT,
        FONT_META_DESCENT, FONT_META_FIRST_CHAR, FONT_META_LAST_CHAR,
        FONT_META_NUM_CHARS). ASCENT/DESCENT are -1 if the source BDF
        didn't specify them.

  - `static const int <prefix>_data[]`
        A flat array containing, for every glyph (in ascending order of
        character code), a small fixed-size header followed by one int
        per bitmap row:

            [0] encoding        (character code, e.g. ASCII value)
            [1] bbx width       (glyph bounding box width, pixels)
            [2] bbx height      (glyph bounding box height, pixels)
            [3] bbx x-offset    (glyph bounding box x offset, pixels)
            [4] bbx y-offset    (glyph bounding box y offset, pixels)
            [5] dwidth x        (advance width, pixels)
            [6] dwidth y        (advance height, pixels)
            [7 .. 7+height-1]   one int per bitmap row, packed MSB-first,
                                padded to a whole number of bytes
                                (i.e. each row is ceil(width/8) bytes,
                                matching the padding already used by BDF)

  - `static const int <prefix>_index[]`
        Indexed by (character_code - first_char). Each entry is either
        the offset into <prefix>_data[] where that character's record
        begins, or -1 if the font does not define that character.

  - `static const bdf_font_t <prefix> = { <prefix>_meta, <prefix>_data, <prefix>_index };`
        A ready-to-use struct tying the three arrays together, so code
        that works with multiple fonts can just pass around
        `const bdf_font_t *` values instead of needing a distinct set
        of macros/arrays per font.

This layout supports variable-width / variable-height glyphs (proportional
fonts) as well as fixed-width fonts, without wasting space on unused
per-glyph fields.
"""

import argparse
import re
import sys


class Glyph:
    __slots__ = ("name", "encoding", "bbx_w", "bbx_h", "bbx_xoff", "bbx_yoff",
                 "dwidth_x", "dwidth_y", "rows")

    def __init__(self):
        self.name = None
        self.encoding = None
        self.bbx_w = self.bbx_h = self.bbx_xoff = self.bbx_yoff = 0
        self.dwidth_x = self.dwidth_y = 0
        self.rows = []  # list of ints, one per bitmap row


class BdfFont:
    def __init__(self):
        self.font_name = ""
        self.point_size = None
        self.res_x = None
        self.res_y = None
        self.bbx_w = self.bbx_h = self.bbx_xoff = self.bbx_yoff = 0
        self.font_ascent = None
        self.font_descent = None
        self.default_char = None
        self.glyphs = []  # list of Glyph, in file order


def parse_bdf(path):
    font = BdfFont()
    glyph = None
    in_bitmap = False
    in_properties = False

    with open(path, "r", encoding="latin-1") as f:
        for raw_line in f:
            line = raw_line.strip()
            if not line:
                continue
            tokens = line.split()
            keyword = tokens[0]

            if keyword == "STARTPROPERTIES":
                in_properties = True
                continue
            if keyword == "ENDPROPERTIES":
                in_properties = False
                continue

            if keyword == "FONT" and not in_properties:
                font.font_name = line[len("FONT"):].strip()
                continue

            if keyword == "SIZE" and not in_properties:
                if len(tokens) >= 4:
                    font.point_size = int(tokens[1])
                    font.res_x = int(tokens[2])
                    font.res_y = int(tokens[3])
                continue

            if keyword == "FONTBOUNDINGBOX":
                font.bbx_w = int(tokens[1])
                font.bbx_h = int(tokens[2])
                font.bbx_xoff = int(tokens[3])
                font.bbx_yoff = int(tokens[4])
                continue

            if keyword == "FONT_ASCENT":
                font.font_ascent = int(tokens[1])
                continue
            if keyword == "FONT_DESCENT":
                font.font_descent = int(tokens[1])
                continue
            if keyword == "DEFAULT_CHAR":
                font.default_char = int(tokens[1])
                continue

            if keyword == "STARTCHAR":
                glyph = Glyph()
                glyph.name = tokens[1] if len(tokens) > 1 else ""
                continue

            if glyph is None:
                # Not currently inside a glyph definition; nothing else
                # here matters for our purposes.
                continue

            if keyword == "ENCODING":
                glyph.encoding = int(tokens[1])
                continue

            if keyword == "DWIDTH":
                glyph.dwidth_x = int(tokens[1])
                glyph.dwidth_y = int(tokens[2]) if len(tokens) > 2 else 0
                continue

            if keyword == "BBX":
                glyph.bbx_w = int(tokens[1])
                glyph.bbx_h = int(tokens[2])
                glyph.bbx_xoff = int(tokens[3])
                glyph.bbx_yoff = int(tokens[4])
                continue

            if keyword == "BITMAP":
                in_bitmap = True
                glyph.rows = []
                continue

            if keyword == "ENDCHAR":
                in_bitmap = False
                if glyph.encoding is not None and glyph.encoding >= 0:
                    font.glyphs.append(glyph)
                glyph = None
                continue

            if in_bitmap:
                # A bitmap row: hex digits, one nibble per 4 pixels,
                # padded to a byte boundary (as BDF already does).
                hex_str = tokens[0]
                glyph.rows.append(int(hex_str, 16) if hex_str else 0)
                continue

    return font


def bytes_per_row(bbx_width):
    return (bbx_width + 7) // 8


# Shared metadata layout used by every generated font header. Keeping this
# in one place (and mirroring it into font_common.h) means the meaning of
# each <prefix>_meta[] slot never has to be re-declared per font.
META_FIELDS = [
    "FONT_META_BBX_WIDTH",
    "FONT_META_BBX_HEIGHT",
    "FONT_META_BBX_XOFF",
    "FONT_META_BBX_YOFF",
    "FONT_META_ASCENT",
    "FONT_META_DESCENT",
    "FONT_META_FIRST_CHAR",
    "FONT_META_LAST_CHAR",
    "FONT_META_NUM_CHARS",
    "FONT_META_NOTDEF_OFFSET",
]


def generate_common_header():
    lines = []
    lines.append("// font_common.h")
    lines.append("// Shared definitions used by every font header generated by")
    lines.append("// bdf_to_header.py. Generate/include this once per project even")
    lines.append("// if you have multiple fonts.")
    lines.append("#ifndef FONT_COMMON_H")
    lines.append("#define FONT_COMMON_H")
    lines.append("")
    lines.append("// Indices into every font's <prefix>_meta[] array.")
    lines.append("enum {")
    for i, name in enumerate(META_FIELDS):
        suffix = "," if i < len(META_FIELDS) - 1 else ","
        lines.append(f"    {name}{' = 0' if i == 0 else ''}{suffix}")
    lines.append("    FONT_META_COUNT")
    lines.append("};")
    lines.append("")
    lines.append("// Bundles a font's three generated arrays together so multiple")
    lines.append("// fonts can be handled generically (e.g. an array of")
    lines.append("// `const bdf_font_t *`) without any per-font macros.")
    lines.append("typedef struct {")
    lines.append("    const int *meta;   // FONT_META_COUNT ints, see enum above")
    lines.append("    const int *data;   // packed glyph records (see font header comment)")
    lines.append("    const int *index;  // [code - meta[FONT_META_FIRST_CHAR]] -> offset")
    lines.append("                       // into data, or -1 if undefined")
    lines.append("} bdf_font_t;")
    lines.append("")
    lines.append("#endif // FONT_COMMON_H")
    lines.append("")
    return "\n".join(lines)


def generate_header(font, prefix, source_name):
    glyphs = sorted(font.glyphs, key=lambda g: g.encoding)
    if not glyphs:
        raise ValueError("No glyphs found in font")

    first_char = glyphs[0].encoding
    last_char = glyphs[-1].encoding
    num_chars = len(glyphs)

    data_words = []      # flat list of ints for font_data[]
    index = [-1] * (last_char - first_char + 1)

    for g in glyphs:
        start_offset = len(data_words)
        index[g.encoding - first_char] = start_offset

        data_words.append(g.encoding)
        data_words.append(g.bbx_w)
        data_words.append(g.bbx_h)
        data_words.append(g.bbx_xoff)
        data_words.append(g.bbx_yoff)
        data_words.append(g.dwidth_x)
        data_words.append(g.dwidth_y)

        expected_rows = g.bbx_h
        rows = list(g.rows)
        # Defensive padding in case a glyph's BITMAP had fewer rows than
        # its BBX height claims (shouldn't happen in well-formed BDF).
        while len(rows) < expected_rows:
            rows.append(0)
        data_words.extend(rows[:expected_rows])

    # Synthesize a ".notdef" replacement glyph: a solid block sized to the
    # font's overall bounding box. It isn't tied to any character code (no
    # index[] entry points to it on its own) -- instead its offset is
    # recorded in meta[FONT_META_NOTDEF_OFFSET] so rendering code can draw
    # it in place of any character the font doesn't define, rather than
    # silently skipping it.
    notdef_w, notdef_h = font.bbx_w, font.bbx_h
    notdef_offset = len(data_words)
    data_words.append(-1)          # encoding: not a real character code
    data_words.append(notdef_w)
    data_words.append(notdef_h)
    data_words.append(font.bbx_xoff)
    data_words.append(font.bbx_yoff)
    data_words.append(notdef_w)    # dwidth x: advance by the full block width
    data_words.append(0)           # dwidth y
    notdef_bytes_per_row = bytes_per_row(notdef_w)
    notdef_bits_per_row = notdef_bytes_per_row * 8
    solid_row = ((1 << notdef_w) - 1) << (notdef_bits_per_row - notdef_w)
    data_words.extend([solid_row] * notdef_h)

    guard = re.sub(r"[^A-Za-z0-9]", "_", prefix).upper() + "_H"

    lines = []
    lines.append(f"// Auto-generated from {source_name} by bdf_to_header.py")
    lines.append("// Do not edit by hand -- regenerate from the source .bdf file instead.")
    if font.font_name:
        lines.append(f"// Source font: {font.font_name}")
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")
    lines.append('#include "font_common.h"')
    lines.append("")
    ascent = font.font_ascent if font.font_ascent is not None else -1
    descent = font.font_descent if font.font_descent is not None else -1
    meta_values = [font.bbx_w, font.bbx_h, font.bbx_xoff, font.bbx_yoff,
                   ascent, descent, first_char, last_char, num_chars,
                   notdef_offset]
    lines.append(f"static const int {prefix}_meta[FONT_META_COUNT] = {{")
    for name, val in zip(META_FIELDS, meta_values):
        comment = name
        if name == "FONT_META_ASCENT" and font.font_ascent is None:
            comment += " (not specified in source BDF)"
        if name == "FONT_META_DESCENT" and font.font_descent is None:
            comment += " (not specified in source BDF)"
        lines.append(f"    {val},  // {comment}")
    lines.append("};")
    lines.append("")
    lines.append("// Per-glyph record layout within " + prefix + "_data[]:")
    lines.append("//   [0] encoding (character code)")
    lines.append("//   [1] bbx width  (pixels)")
    lines.append("//   [2] bbx height (pixels, also the number of row words that follow)")
    lines.append("//   [3] bbx x-offset (pixels)")
    lines.append("//   [4] bbx y-offset (pixels)")
    lines.append("//   [5] dwidth x (advance width, pixels)")
    lines.append("//   [6] dwidth y (advance height, pixels)")
    lines.append("//   [7..] one int per bitmap row, MSB-first, "
                  "ceil(width/8) bytes per row (BDF padding)")
    lines.append("static const int " + prefix + "_data[] = {")

    # Emit the data array, one glyph per group of lines for readability.
    idx = 0
    for g in glyphs:
        header_vals = data_words[idx:idx + 7]
        idx += 7
        nrows = g.bbx_h
        row_vals = data_words[idx:idx + nrows]
        idx += nrows

        lines.append(
            f"    /* '{g.name}' (code {g.encoding}) */"
        )
        header_str = ", ".join(str(v) for v in header_vals)
        lines.append(f"    {header_str},")
        row_width_hex = bytes_per_row(g.bbx_w) * 2  # hex digits per row
        row_strs = [f"0x{v:0{row_width_hex}X}" for v in row_vals]
        # Wrap rows at a reasonable line length.
        chunk = []
        cur_len = 4
        out_row_lines = []
        for rs in row_strs:
            piece = rs + ", "
            if cur_len + len(piece) > 96 and chunk:
                out_row_lines.append("    " + "".join(chunk))
                chunk = []
                cur_len = 4
            chunk.append(piece)
            cur_len += len(piece)
        if chunk:
            out_row_lines.append("    " + "".join(chunk))
        lines.extend(out_row_lines)

    # Print the synthesized .notdef (missing-character) block glyph.
    notdef_header_vals = data_words[idx:idx + 7]
    idx += 7
    notdef_row_vals = data_words[idx:idx + notdef_h]
    idx += notdef_h
    lines.append(f"    /* '.notdef' -- solid block used for missing characters "
                  f"(see FONT_META_NOTDEF_OFFSET) */")
    lines.append("    " + ", ".join(str(v) for v in notdef_header_vals) + ",")
    notdef_row_hex_digits = notdef_bytes_per_row * 2
    notdef_row_strs = [f"0x{v:0{notdef_row_hex_digits}X}" for v in notdef_row_vals]
    chunk = []
    cur_len = 4
    out_row_lines = []
    for rs in notdef_row_strs:
        piece = rs + ", "
        if cur_len + len(piece) > 96 and chunk:
            out_row_lines.append("    " + "".join(chunk))
            chunk = []
            cur_len = 4
        chunk.append(piece)
        cur_len += len(piece)
    if chunk:
        out_row_lines.append("    " + "".join(chunk))
    lines.extend(out_row_lines)

    lines.append("};")
    lines.append("")
    lines.append(f"// {prefix}_index[code - {prefix.upper()}_FIRST_CHAR] gives the offset")
    lines.append(f"// into {prefix}_data[] where that character's record starts,")
    lines.append("// or -1 if the font does not define that character code.")
    lines.append(f"static const int {prefix}_index[] = {{")
    idx_strs = [str(v) for v in index]
    chunk = []
    cur_len = 4
    out_lines = []
    for s in idx_strs:
        piece = s + ", "
        if cur_len + len(piece) > 96 and chunk:
            out_lines.append("    " + "".join(chunk))
            chunk = []
            cur_len = 4
        chunk.append(piece)
        cur_len += len(piece)
    if chunk:
        out_lines.append("    " + "".join(chunk))
    lines.extend(out_lines)
    lines.append("};")
    lines.append("")
    lines.append(f"static const bdf_font_t {prefix} = {{ {prefix}_meta, {prefix}_data, {prefix}_index }};")
    lines.append("")
    lines.append(f"#endif // {guard}")
    lines.append("")

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="Convert a BDF font to a C header.")
    parser.add_argument("input", help="Path to input .bdf file")
    parser.add_argument("output", help="Path to output .h file")
    parser.add_argument(
        "--name",
        dest="prefix",
        default=None,
        help="Identifier prefix for arrays/#defines (default: derived from filename)",
    )
    args = parser.parse_args()

    if args.prefix:
        prefix = args.prefix
    else:
        import os
        base = os.path.splitext(os.path.basename(args.input))[0]
        prefix = re.sub(r"[^A-Za-z0-9_]", "_", base).lower()
        if not prefix or prefix[0].isdigit():
            prefix = "font_" + prefix

    font = parse_bdf(args.input)
    header = generate_header(font, prefix, args.input)

    with open(args.output, "w") as f:
        f.write(header)

    import os
    common_path = os.path.join(os.path.dirname(os.path.abspath(args.output)), "font_common.h")
    with open(common_path, "w") as f:
        f.write(generate_common_header())

    print(f"Wrote {args.output}: {len(font.glyphs)} glyphs, prefix '{prefix}'")
    print(f"Wrote {common_path} (shared by all fonts; safe to regenerate/overwrite)")


if __name__ == "__main__":
    sys.exit(main())
