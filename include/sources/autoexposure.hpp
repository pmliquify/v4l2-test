#pragma once

#include <sources/imagesource.hpp>
#include <list>


class AutoExposure
{
public:
        AutoExposure();

        void setActive(bool active) { m_active = active; }
        void setTest(bool test) { m_test = test; }
        void setSub(unsigned short sub) { m_sub = sub; }
        void setP(unsigned short p) { m_p = p; }
        void setD(unsigned short d) { m_d = d; }
        void setTarget(unsigned short target) { m_target = target;}
        void setExposureMin(unsigned int exposure) { m_exposureMin = exposure; }
        void setExposureMax(unsigned int exposure) { m_exposureMax = exposure; }
        
        int init(unsigned int exposure);
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
        unsigned short m_sub;
        unsigned short m_p;
        unsigned short m_d;
        unsigned short m_target;
        unsigned int m_exposureMin;
        unsigned int m_exposureMax;

        ImageSource *m_imageSource;
        unsigned int m_exposure;
        unsigned short m_meanLast;
        TestState m_testState;
        unsigned short m_initCount;
        unsigned short m_testCount;
        unsigned short m_measureCount;
        unsigned short m_meanMin;
        unsigned short m_meanMax;
        float m_latency;

        int calculateExposure(ImageSource *imageSource, Image *image);
        int testLatency(ImageSource *imageSource, Image *image);
};