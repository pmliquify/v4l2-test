#pragma once

#include <v4l2test.hpp>
#include <framebuffer.hpp>


class StreamTest : public V4L2Test
{
public:
    StreamTest();

    void printArgs();
    int setup(CommandArgs &args);
    int exec(V4L2ImageSource &imageSource, V4L2Image &image);

private:
    FrameBuffer m_frameBuffer;
    bool    m_fb;
    int     m_x;
    int     m_y;
    int     m_width;
    int     m_height;
    int     m_delay;
    int     m_print;
    int     m_shift;
    int     m_imageCount;
};
