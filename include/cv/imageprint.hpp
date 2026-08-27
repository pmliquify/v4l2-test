/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

#include <cv/image.hpp>


class ImagePrint
{
public:
    static void print(const Image *image, unsigned int format, short x, short y, 
        unsigned long lastTimestamp, unsigned char count = 10);
};