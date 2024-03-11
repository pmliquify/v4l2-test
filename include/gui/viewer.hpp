#pragma once

#include <cv/image.hpp>
#include <sources/imagesource.hpp>


class Viewer
{
public:
    Viewer();
    ~Viewer();

    void init(ImageSource *imageSource);
    void show(Image *image);
    void hide();
};