import sys
import argparse

def parse_char_arg(arg_str):
    """Helper to parse an argument as either an integer code or a single character."""
    try:
        # Try parsing as integer (e.g., "32" or "0x20")
        if arg_str.lower().startswith("0x"):
            return int(arg_str, 16)
        return int(arg_str)
    except ValueError:
        # If it's a single character string (e.g., "A")
        if len(arg_str) == 1:
            return ord(arg_str)
        raise argparse.ArgumentTypeError(
            f"'{arg_str}' must be an integer ASCII code (e.g., 32, 0x20) or a single character (e.g., A)."
        )

def bdf_to_c_header(bdf_path, output_path, start_char, end_char, array_name="font_9x15"):
    with open(bdf_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    out = []
    out.append("#ifndef FONT_9X15_H")
    out.append("#define FONT_9X15_H")
    out.append("")
    out.append(f"// Generated from {bdf_path} (Range: {start_char} to {end_char})")
    out.append(f"const unsigned char {array_name}[][15] = {{")

    # Dictionary to temporarily hold our parsed glyphs
    glyphs = {}
    
    in_char = False
    in_bitmap = False
    current_encoding = -1
    current_bitmap = []

    for line in lines:
        line = line.strip()
        
        if line.startswith("STARTCHAR"):
            in_char = True
            current_bitmap = []
        elif line.startswith("ENCODING"):
            try:
                current_encoding = int(line.split()[1])
            except (IndexError, ValueError):
                current_encoding = -1
        elif line.startswith("BITMAP"):
            in_bitmap = True
        elif line.startswith("ENDCHAR"):
            # Only process if the character falls within our specified window
            if start_char <= current_encoding <= end_char:
                while len(current_bitmap) < 15:
                    current_bitmap.append("0x00")
                glyphs[current_encoding] = current_bitmap[:15]
            
            in_char = False
            in_bitmap = False
        elif in_bitmap:
            # Grab the first byte (first 2 hex chars) for 8-bit packing
            hex_val = line[:2] if len(line) >= 2 else "00"
            current_bitmap.append(f"0x{hex_val.upper()}")

    # Loop through the exact contiguous range requested by the user
    for code in range(start_char, end_char + 1):
        if code in glyphs:
            bytes_str = ", ".join(glyphs[code])
            char_repr = chr(code) if 32 <= code <= 126 else '\\'
            out.append(f"    {{{bytes_str}}}, // ASCII {code} ('{char_repr}')")
        else:
            # Fallback padding if a specific character code is missing in the BDF file
            empty_rows = ", ".join(["0x00"] * 15)
            out.append(f"    {{{empty_rows}}}, // ASCII {code} (Missing/Empty)")

    out.append("};")
    out.append("")
    out.append("#endif // FONT_9X15_H")

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("\n".join(out))
    print(f"Successfully generated {output_path} with {end_char - start_char + 1} characters.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert a BDF font file into an embedded C header array.")
    parser.add_argument("bdf_file", help="Path to the input .bdf file")
    parser.add_argument("output_file", help="Path to the output .h file")
    parser.add_argument("start_char", type=parse_char_arg, help="Start character (e.g., 32, 0x20, or 'A')")
    parser.add_argument("end_char", type=parse_char_arg, help="End character (e.g., 127, 0x7F, or 'Z')")
    parser.add_argument("--name", default="font_9x15", help="Name of the C array variable (default: font_9x15)")

    args = parser.parse_args()

    # Enforce logical bounding ranges
    if args.start_char > args.end_char:
        print("Error: Start character cannot be greater than the end character.", file=sys.stderr)
        sys.exit(1)

    bdf_to_c_header(args.bdf_file, args.output_file, args.start_char, args.end_char, args.name)
