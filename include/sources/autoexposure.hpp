#pragma once

#include <sources/imagesource.hpp>
#include <list>


class AutoExposure
{
public:
        AutoExposure();

        void setActive(bool active) { m_active = active; }
        void setTest(bool test) { m_test = test; }
        void setSub(u_int16_t sub) { m_sub = sub; }
        void setP(u_int16_t p) { m_p = p; }
        void setD(u_int16_t d) { m_d = d; }
        void setTarget(u_int16_t target) { m_target = target;}
        void setExposureMin(u_int32_t exposure) { m_exposureMin = exposure; }
        void setExposureMax(u_int32_t exposure) { m_exposureMax = exposure; }
        
        int init(u_int32_t exposure);
        int run(ImageSource *imageSource, Image *image);

private:
        typedef enum {
                TestStateInit,
                TestStateInitMin,
                TestStateInitMax,
                TestStateMeasureMin,
                TestStateMeasureMax
        } TestState;

        bool m_active;
        bool m_test;
        u_int16_t m_sub;
        u_int16_t m_p;
        u_int16_t m_d;
        u_int16_t m_target;
        u_int32_t m_exposureMin;
        u_int32_t m_exposureMax;

        ImageSource *m_imageSource;
        u_int32_t m_exposure;
        u_int16_t m_meanLast;
        TestState m_testState;
        u_int16_t m_initCount;
        u_int16_t m_testCount;
        u_int16_t m_measureCount;
        u_int16_t m_meanMin;
        u_int16_t m_meanMax;
        float m_latency;

        int calculateExposure(ImageSource *imageSource, Image *image);
        int testLatency(ImageSource *imageSource, Image *image);
};