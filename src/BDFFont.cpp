// bdf_font.cpp
//
// TODO: point this at your project's real Canvas/RGB declarations, e.g.:
//   #include "canvas.h"
#include "Canvas.h"

#include "BDFFont.h"

// Per-glyph record layout, matching bdf_to_header.py's font_*_data[]:
//   [0] encoding
//   [1] bbx width
//   [2] bbx height
//   [3] bbx x-offset
//   [4] bbx y-offset
//   [5] dwidth x (advance)
//   [6] dwidth y
//   [7..] one int per bitmap row, MSB-first, ceil(width/8) bytes per row
namespace {
constexpr int kEncoding = 0;
constexpr int kBbxW     = 1;
constexpr int kBbxH     = 2;
constexpr int kBbxXoff  = 3;
constexpr int kBbxYoff  = 4;
constexpr int kDwidthX  = 5;
constexpr int kDwidthY  = 6;
constexpr int kRowsStart = 7;
}  // namespace

BDFFont::BDFFont(const bdf_font_t& fontData)
    : meta(fontData.meta), data(fontData.data), index(fontData.index) {}

int BDFFont::glyph_offset(int c) const {
    int first = meta[FONT_META_FIRST_CHAR];
    int last  = meta[FONT_META_LAST_CHAR];
    if (c < first || c > last) return -1;
    return index[c - first];
}

bool BDFFont::has_char(int c) const {
    return glyph_offset(c) >= 0;
}

int BDFFont::ascent() const {
    int a = meta[FONT_META_ASCENT];
    if (a >= 0) return a;
    // Fall back to "top of the font bounding box, relative to the
    // baseline" -- BDF's bounding box is [yoff, yoff + height), so its
    // top edge sits (yoff + height) pixels above the baseline.
    return meta[FONT_META_BBX_YOFF] + meta[FONT_META_BBX_HEIGHT];
}

int BDFFont::descent() const {
    int d = meta[FONT_META_DESCENT];
    if (d >= 0) return d;
    // Fall back to "how far the bounding box extends below the
    // baseline" (0 if it doesn't).
    int yoff = meta[FONT_META_BBX_YOFF];
    return yoff < 0 ? -yoff : 0;
}

int BDFFont::line_height() const {
    return ascent() + descent();
}

void BDFFont::renderGlyph(Canvas* canvas, int offset, const RGB& color,
                            int pen_x, int pen_y) const {
    int w    = data[offset + kBbxW];
    int h    = data[offset + kBbxH];
    int xoff = data[offset + kBbxXoff];
    int yoff = data[offset + kBbxYoff];

    int bytes_per_row = (w + 7) / 8;
    int bits_per_row = bytes_per_row * 8;

    for (int r = 0; r < h; ++r) {
        int row = data[offset + kRowsStart + r];
        // Row r=0 is the topmost row of the glyph's bitmap. Its distance
        // above the baseline is yoff + (h - 1 - r).
        int canvas_y = pen_y + yoff + (h - 1 - r);
        for (int c = 0; c < w; ++c) {
            int bit_pos = bits_per_row - 1 - c;
            if (row & (1 << bit_pos)) {
                int canvas_x = pen_x + xoff + c;
                canvas->set(canvas_x, canvas_y, color);
            }
        }
    }
}

uint BDFFont::render(Canvas* canvas, const char* text, int x, int y, const RGB& color,
                       bool renderMissing, RenderAngle angle) {
    int pen_x = x;
    int pen_y = y;
    int lh = line_height();
    int tw = 0;
    int notdef_off = meta[FONT_META_NOTDEF_OFFSET];
    int fallback_advance = meta[FONT_META_BBX_WIDTH];

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text);
         *p; ++p) {
        unsigned char c = *p;

        if (c == '\n') {
            pen_x = x;
            pen_y -= lh;  // next line is *below* since y increases upward
            continue;
        }

        int offset = glyph_offset(c);
        if (offset < 0) {
            if (renderMissing && notdef_off >= 0) {
                renderGlyph(canvas, notdef_off, color, pen_x, pen_y);
                pen_x += data[notdef_off + kDwidthX];
                pen_y += data[notdef_off + kDwidthY];
                tw += data[notdef_off + kDwidthX];
            } else {
                // No notdef glyph available (or caller opted out): just
                // advance so text after it doesn't collide/overlap.
                pen_x += fallback_advance;
                tw += fallback_advance;
            }
            continue;
        }

        renderGlyph(canvas, offset, color, pen_x, pen_y);
        pen_x += data[offset + kDwidthX];
        pen_y += data[offset + kDwidthY];
    }
    return tw;
}

uint BDFFont::getWidth(const char* text, bool renderMissing) const {
    int width = 0;
    int notdef_off = meta[FONT_META_NOTDEF_OFFSET];
    int fallback_advance = meta[FONT_META_BBX_WIDTH];

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text);
         *p && *p != '\n'; ++p) {
        int offset = glyph_offset(*p);
        if (offset < 0) {
            width += (renderMissing && notdef_off >= 0)
                         ? data[notdef_off + kDwidthX]
                         : fallback_advance;
        } else {
            width += data[offset + kDwidthX];
        }
    }
    return width;
}

std::pair<uint, uint> BDFFont::getBoundingBox(const char* text, bool renderMissing) const {
    int lh = line_height();
    int width = 0;
    int height = lh;
    int notdef_off = meta[FONT_META_NOTDEF_OFFSET];
    int fallback_advance = meta[FONT_META_BBX_WIDTH];

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        unsigned char c = *p;
        //
        // Add a line height if there's a newline.
        if (c == '\n') {
            height += lh;
            continue;
        }
        int offset = glyph_offset(*p);
        if (offset < 0) {
            width += (renderMissing && notdef_off >= 0)
                         ? data[notdef_off + kDwidthX]
                         : fallback_advance;
        } else {
            width += data[offset + kDwidthX];
        }
    }
    return std::pair<int, int>(width, height);
}
