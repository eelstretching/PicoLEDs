#include "GifWriter.h"

#include <algorithm>
#include <unordered_map>

bool GifWriter::open(const std::string& path, int width, int height) {
    f = fopen(path.c_str(), "wb");
    if (f == nullptr) {
        return false;
    }
    this->width = width;
    this->height = height;
    fwrite("GIF89a", 1, 6, f);
    put16(width);
    put16(height);
    // No global color table; every frame brings its own.
    fputc(0x00, f);
    fputc(0, f);
    fputc(0, f);
    // NETSCAPE2.0 extension so the animation loops forever.
    const uint8_t loop[] = {0x21, 0xFF, 0x0B, 'N', 'E', 'T', 'S', 'C', 'A',
                            'P',  'E',  '2',  '.', '0', 0x03, 0x01, 0x00,
                            0x00, 0x00};
    fwrite(loop, 1, sizeof(loop), f);
    return true;
}

void GifWriter::close() {
    if (f != nullptr) {
        fputc(0x3B, f);
        fclose(f);
        f = nullptr;
    }
}

void GifWriter::put16(int v) {
    fputc(v & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);
}

namespace {

struct ColorCount {
    uint32_t rgb;
    uint32_t count;
};

int channel(uint32_t rgb, int c) { return (rgb >> (16 - 8 * c)) & 0xFF; }

/// @brief Median cut: keep splitting the box with the widest channel range at
/// its median until we have n boxes, then average each box.
std::vector<uint32_t> medianCut(std::vector<ColorCount>& colors, size_t n) {
    struct Box {
        size_t start, end;
    };
    std::vector<Box> boxes = {{0, colors.size()}};
    while (boxes.size() < n) {
        int best = -1, bestRange = 0, bestChannel = 0;
        for (size_t i = 0; i < boxes.size(); i++) {
            if (boxes[i].end - boxes[i].start < 2) {
                continue;
            }
            for (int c = 0; c < 3; c++) {
                int lo = 255, hi = 0;
                for (size_t j = boxes[i].start; j < boxes[i].end; j++) {
                    int v = channel(colors[j].rgb, c);
                    lo = std::min(lo, v);
                    hi = std::max(hi, v);
                }
                if (hi - lo > bestRange) {
                    best = i;
                    bestRange = hi - lo;
                    bestChannel = c;
                }
            }
        }
        if (best < 0) {
            break;
        }
        Box b = boxes[best];
        std::sort(colors.begin() + b.start, colors.begin() + b.end,
                  [&](const ColorCount& x, const ColorCount& y) {
                      return channel(x.rgb, bestChannel) <
                             channel(y.rgb, bestChannel);
                  });
        //
        // Split where half the pixels (not half the colors) are on each side.
        uint64_t total = 0;
        for (size_t j = b.start; j < b.end; j++) {
            total += colors[j].count;
        }
        uint64_t sum = 0;
        size_t mid = b.start + 1;
        for (size_t j = b.start; j < b.end - 1; j++) {
            sum += colors[j].count;
            mid = j + 1;
            if (sum * 2 >= total) {
                break;
            }
        }
        boxes[best] = {b.start, mid};
        boxes.push_back({mid, b.end});
    }
    std::vector<uint32_t> palette;
    for (Box& b : boxes) {
        uint64_t s[3] = {0, 0, 0}, total = 0;
        for (size_t j = b.start; j < b.end; j++) {
            for (int c = 0; c < 3; c++) {
                s[c] += (uint64_t)channel(colors[j].rgb, c) * colors[j].count;
            }
            total += colors[j].count;
        }
        palette.push_back(((s[0] / total) << 16) | ((s[1] / total) << 8) |
                          (s[2] / total));
    }
    return palette;
}

}  // namespace

void GifWriter::addFrame(const uint8_t* rgb, int delayCS) {
    size_t n = (size_t)width * height;

    //
    // Count the colors in this frame.
    std::unordered_map<uint32_t, uint32_t> counts;
    for (size_t i = 0; i < n; i++) {
        uint32_t c = (rgb[3 * i] << 16) | (rgb[3 * i + 1] << 8) | rgb[3 * i + 2];
        counts[c]++;
    }

    std::vector<uint32_t> palette;
    std::unordered_map<uint32_t, uint8_t> lookup;
    if (counts.size() <= 256) {
        for (auto& kv : counts) {
            lookup[kv.first] = palette.size();
            palette.push_back(kv.first);
        }
    } else {
        //
        // LED pictures are mostly black, and black that comes out slightly
        // gray looks wrong, so black always gets its own entry.
        std::vector<ColorCount> colors;
        for (auto& kv : counts) {
            if (kv.first != 0) {
                colors.push_back({kv.first, kv.second});
            }
        }
        palette = medianCut(colors, 255);
        palette.push_back(0);
        for (auto& kv : counts) {
            int best = 0, bestDist = 1 << 30;
            for (size_t p = 0; p < palette.size(); p++) {
                int d = 0;
                for (int c = 0; c < 3; c++) {
                    int diff = channel(kv.first, c) - channel(palette[p], c);
                    d += diff * diff;
                }
                if (d < bestDist) {
                    bestDist = d;
                    best = p;
                }
            }
            lookup[kv.first] = best;
        }
    }

    int bits = 1;
    while ((1u << bits) < palette.size()) {
        bits++;
    }
    palette.resize(1 << bits, 0);

    std::vector<uint8_t> indices(n);
    for (size_t i = 0; i < n; i++) {
        uint32_t c = (rgb[3 * i] << 16) | (rgb[3 * i + 1] << 8) | rgb[3 * i + 2];
        indices[i] = lookup[c];
    }

    // Graphic control extension, for the delay.
    fputc(0x21, f);
    fputc(0xF9, f);
    fputc(0x04, f);
    fputc(0x00, f);
    put16(delayCS);
    fputc(0x00, f);
    fputc(0x00, f);

    // Image descriptor with a local color table.
    fputc(0x2C, f);
    put16(0);
    put16(0);
    put16(width);
    put16(height);
    fputc(0x80 | (bits - 1), f);
    for (uint32_t c : palette) {
        fputc((c >> 16) & 0xFF, f);
        fputc((c >> 8) & 0xFF, f);
        fputc(c & 0xFF, f);
    }
    writeLZW(indices, std::max(2, bits));
    frames++;
}

void GifWriter::writeLZW(const std::vector<uint8_t>& indices, int minCodeSize) {
    fputc(minCodeSize, f);

    std::vector<uint8_t> block;
    uint32_t bitBuffer = 0;
    int bitCount = 0;
    auto flushBlock = [&]() {
        if (!block.empty()) {
            fputc(block.size(), f);
            fwrite(block.data(), 1, block.size(), f);
            block.clear();
        }
    };
    auto emit = [&](int code, int codeSize) {
        bitBuffer |= (uint32_t)code << bitCount;
        bitCount += codeSize;
        while (bitCount >= 8) {
            block.push_back(bitBuffer & 0xFF);
            bitBuffer >>= 8;
            bitCount -= 8;
            if (block.size() == 255) {
                flushBlock();
            }
        }
    };

    const int clearCode = 1 << minCodeSize;
    const int endCode = clearCode + 1;
    int codeSize = minCodeSize + 1;
    int nextCode = endCode + 1;
    // The dictionary maps (prefix code, next index) to a code.
    std::unordered_map<uint32_t, int> dict;

    emit(clearCode, codeSize);
    int prefix = -1;
    for (uint8_t k : indices) {
        if (prefix < 0) {
            prefix = k;
            continue;
        }
        uint32_t key = ((uint32_t)prefix << 8) | k;
        auto it = dict.find(key);
        if (it != dict.end()) {
            prefix = it->second;
            continue;
        }
        emit(prefix, codeSize);
        if (nextCode < 4096) {
            dict[key] = nextCode++;
            if (nextCode > (1 << codeSize) && codeSize < 12) {
                codeSize++;
            }
        } else {
            emit(clearCode, codeSize);
            dict.clear();
            codeSize = minCodeSize + 1;
            nextCode = endCode + 1;
        }
        prefix = k;
    }
    if (prefix >= 0) {
        emit(prefix, codeSize);
    }
    emit(endCode, codeSize);
    if (bitCount > 0) {
        block.push_back(bitBuffer & 0xFF);
    }
    flushBlock();
    fputc(0x00, f);
}
