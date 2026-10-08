// A small animated GIF writer, so the simulator doesn't need any libraries to
// record. Each frame gets its own 256 color palette, picked by median cut.
#ifndef PICOLEDS_SIM_GIF_WRITER_H
#define PICOLEDS_SIM_GIF_WRITER_H

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <vector>

class GifWriter {
   public:
    ~GifWriter() { close(); }

    /// @brief Starts a GIF that loops forever.
    /// @return false if the file couldn't be opened.
    bool open(const std::string& path, int width, int height);

    /// @brief Adds a frame.
    /// @param rgb width * height RGB triples, top row first.
    /// @param delayCS how long to show the frame, in hundredths of a second.
    void addFrame(const uint8_t* rgb, int delayCS);

    void close();

    int getFrameCount() { return frames; }

   private:
    FILE* f = nullptr;
    int width = 0;
    int height = 0;
    int frames = 0;

    void put16(int v);
    void writeLZW(const std::vector<uint8_t>& indices, int minCodeSize);
};

#endif
