#pragma once

#include <commandargclass.hpp>
#include <v4l2imagesource.hpp>


class V4L2Test : public CommandArgClass
{
public:
    virtual int exec(V4L2ImageSource &imageSource, V4L2Image &image) = 0;
};