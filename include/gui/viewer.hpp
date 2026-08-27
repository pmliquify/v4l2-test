/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

#include <cv/image.hpp>
#include <sources/imagesource.hpp>


class Viewer
{
public:
    Viewer();
    ~Viewer();

    void init(ImageSource *imageSource);
    int show(Image *image);
    void hide();
};