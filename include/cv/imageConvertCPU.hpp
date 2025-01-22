#pragma once

#include <cv/image.hpp>
#include <cstdint>

struct ImageData {
    unsigned char* data;
    size_t size;
};
class ImageConvertCPU
{
    public:
        uint16_t grey10BitToRGB565 (const uint16_t &grey) const;
        uint16_t grey8BitToRGB565 (const char &grey) const;
        unsigned long grey8BitToRGB888(const uint16_t &grey) const;
        unsigned long grey10BitToRGB888(const uint16_t &grey) const;

        ImageData convert10BitGreyToRGB888(const Image *image) const;
        ImageData convert10BitPackedGreyToRGB888(const Image *image) const;
        ImageData convert10BitPackedBayerToRGB888(const Image *image) const;
        ImageData convert8BitBayerToRGB888(const Image *image) const;


};