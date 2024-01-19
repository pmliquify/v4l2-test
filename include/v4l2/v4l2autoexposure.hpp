#pragma once

#include <v4l2imagesource.hpp>
#include <v4l2image.hpp>
#include <list>

class V4L2AutoExposure
{
public:
        bool m_test;
        u_int16_t m_sub;
        u_int16_t m_aep;
        u_int16_t m_aed;
        u_int16_t m_aeTarget;
        u_int32_t m_exposureMin;
        u_int32_t m_exposureMax;

        V4L2AutoExposure(V4L2ImageSource *imageSource);
        
        int init(u_int32_t exposure);
        int exec(V4L2Image &image);

private:
        typedef enum {
                TestStateInit,
                TestStateInitMin,
                TestStateInitMax,
                TestStateMeasureMin,
                TestStateMeasureMax
        } TestState;

        V4L2ImageSource *m_imageSource;
        u_int32_t m_exposure;
        u_int16_t m_meanLast;
        TestState m_testState;
        u_int16_t m_initCount;
        u_int16_t m_testCount;
        u_int16_t m_measureCount;
        u_int16_t m_meanMin;
        u_int16_t m_meanMax;
        float m_latency;

        int calculateExposure(V4L2Image &image);
        int testLatency(V4L2Image &image);
};