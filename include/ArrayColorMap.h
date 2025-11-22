#ifndef ARRAYCOLORMAP_H
#define ARRAYCOLORMAP_H
#pragma once
#include <initializer_list>

#include "ColorMap.h"

class ArrayColorMap : public ColorMap {
   protected:
    uint8_t size;
    uint8_t p;
    RGB* entries;

   public:
    ArrayColorMap();
    ArrayColorMap(uint8_t size);
    ArrayColorMap(std::initializer_list<RGB> rhs);
    ArrayColorMap(std::initializer_list<HSV> rhs);
    ArrayColorMap(const ArrayColorMap& rhs);
    ~ArrayColorMap();
    virtual uint8_t getSize() { return size; };
    virtual uint8_t getUsed() { return p; };

    virtual RGB operator[](uint8_t index) override;
    virtual uint8_t addColor(const RGB& color) override;
    virtual uint8_t addColor(const HSV& color) override;
    virtual void setColor(uint8_t index, const RGB& color) override;
    virtual void setColor(uint8_t index, const HSV& color) override;
    virtual uint8_t addColors(std::initializer_list<RGB> colors) override;
    virtual RGB getColor(uint8_t index) override;
    virtual uint8_t getIndex(RGB& color) override;
    virtual uint8_t getIndex(HSV& color) override;
    virtual void setBrightness(uint8_t value) override;
};

#endif