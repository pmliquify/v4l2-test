#pragma once

#include <v4l2test.hpp>


class NoiseTest : public V4L2Test
{
public:
    NoiseTest();

    void printArgs();
    int setup(CommandArgs &args);
    int exec(V4L2ImageSource &imageSource, V4L2Image &image);

private:
    int m_subSampling;
    int m_exposureStart;
    int m_exposureEnd;
    int m_exposureStep;
    int m_gainStart;
    int m_gainEnd;
    int m_gainStep;
};