#pragma once

#include <cv/image.hpp>


class ImagePrint
{
public:
    static void print(const Image *image, u_int32_t format, int16_t x, int16_t y, 
        u_int64_t lastTimestamp, u_int8_t count = 10);
};