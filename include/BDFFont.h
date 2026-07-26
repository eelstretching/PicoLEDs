// bdf_font.h
//
// Renders text from a BDF-derived font (see bdf_to_header.py / font_common.h)
// onto a pixel Canvas.
//
// Coordinate convention: this class assumes the Canvas has (0,0) at the
// lower-left corner with y increasing upward -- i.e. a standard Cartesian
// plane, matching how BDF itself defines glyph offsets (BBX y-offset is
// "how far above/below the baseline"). Because of that, no y-flip is
// needed anywhere in this code.
//
// The (x, y) passed to render() is the *baseline origin* of the first
// character -- the same reference point BDF/PostScript-style fonts use.
// That's usually a few pixels above the true bottom of descenders (like
// 'g' or 'y') and a few pixels below the top of ascenders/caps. If you'd
// rather position by the top-left corner of the text, add ascent()
// (or bbx_height() if the font has no declared ascent) to your y value
// before calling render().
//
// This header only forward-declares Canvas and RGB. Include your actual
// Canvas/RGB definitions in the .cpp (or before including this header,
// if you prefer a header-only build) -- adjust the placeholder include
// at the top of bdf_font.cpp to match your project.

#pragma once

#include "Font.h"
#include "Canvas.h"
#include "bdf_font_common.h"

// @brief A class for rendering text using a BDF-derived font.
class BDFFont : public Font {
public:
    // meta/data/index are the three arrays generated for a single font,
    // e.g. font_9x15_meta, font_9x15_data, font_9x15_index -- or simply
    // pass a bdf_font_t's members: BDFFont f(font_9x15.meta, font_9x15.data, font_9x15.index);
    BDFFont(const bdf_font_t& fontData);

    // Draws `text` starting with the first glyph's baseline origin at
    // (x, y). Advances left-to-right using each glyph's DWIDTH. A '\n'
    // in the string moves to a new line: x resets to the original x,
    // and y decreases by line_height() (since y increases upward here).
    // Characters not present in the font are drawn as a solid block
    // (the generated ".notdef" glyph) when draw_missing is true (the
    // default) so gaps are visible instead of silent; pass false to
    // skip them instead (still advances the pen either way).
    uint render(Canvas* canvas, const char* text, int x, int y, const RGB& color,
                bool renderMissing = true, RenderAngle angle = RENDER_0) override;

    // Total advance width (pixels) of `text` if rendered on one line
    // (stops measuring at the first '\n', if any). Pass the same
    // draw_missing value you'll use for render() so the measured width
    // matches what will actually be drawn.
    uint getWidth(const char* text, bool renderMissing = true) const override;

    std::pair<uint, uint> getBoundingBox(const char* text, bool renderMissing = true) const override;

    // Font-wide metrics, straight out of the generated meta[] array.
    int bbx_width()   const { return meta[FONT_META_BBX_WIDTH]; }
    int bbx_height()  const { return meta[FONT_META_BBX_HEIGHT]; }
    int bbx_xoff()    const { return meta[FONT_META_BBX_XOFF]; }
    int bbx_yoff()    const { return meta[FONT_META_BBX_YOFF]; }
    int first_char()  const { return meta[FONT_META_FIRST_CHAR]; }
    int last_char()   const { return meta[FONT_META_LAST_CHAR]; }
    int num_chars()   const { return meta[FONT_META_NUM_CHARS]; }

    // Vertical distance from baseline to top of a full-height line
    // (ascent) and from baseline down to the lowest descender
    // (descent). Falls back to deriving something reasonable from the
    // font bounding box if the source BDF didn't specify ASCENT/DESCENT.
    int ascent() const;
    int descent() const;

    // Recommended distance between successive baselines (ascent + descent).
    int line_height() const;

    // True if the font defines a glyph for character code c.
    bool has_char(int c) const;

    // True if this header includes a synthesized .notdef block glyph
    // (older generated headers, from before this feature existed, won't).
    bool has_notdef() const { return meta[FONT_META_NOTDEF_OFFSET] >= 0; }

private:
    const int* meta;
    const int* data;
    const int* index;

    // Returns the offset into data_ for character c's record, or -1.
    int glyph_offset(int c) const;

    // Draws a single glyph whose record starts at data_[offset], with
    // its baseline origin at (pen_x, pen_y).
    void renderGlyph(Canvas* canvas, int offset, const RGB& color,
                       int pen_x, int pen_y) const;
};
