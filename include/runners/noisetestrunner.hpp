#pragma once

#include <runners/imagesourcerunner.hpp>


class NoiseTestRunner : public ImageSourceRunner
{
public:
        NoiseTestRunner();

        void printArgs();
        int setup(CommandArgs &args);
        int run(ImageSource *imageSource);

private:
        int  m_bufferCount;
        int  m_subSampling;
        int  m_exposureStart;
        int  m_exposureEnd;
        int  m_exposureStep;
        int  m_gainStart;
        int  m_gainEnd;
        int  m_gainStep;
        bool m_writeRowStats;

        void getAllImagesFromBuffer(ImageSource *imageSource);
        Image *measureStats(ImageSource *imageSource, int exposure, int gain, 
                unsigned short &minValue, unsigned short &maxValue, unsigned short &meanValue, float &stdValue, 
                float &slopeValue, unsigned short &maxValueLimit);
        int measureNoise(ImageSource *imageSource);
};