#pragma once

#include <v4l2test.hpp>


class IspTest : public V4L2Test
{
public:
    IspTest();

    void printArgs();
    int setup(CommandArgs &args);
    int exec(V4L2ImageSource &imageSource, V4L2Image &image);
};
