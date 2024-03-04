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
        int m_bufferCount;
        int m_subSampling;
        int m_exposureStart;
        int m_exposureEnd;
        int m_exposureStep;
        int m_gainStart;
        int m_gainEnd;
        int m_gainStep;
        bool m_writeRowStats;

        void getAllImagesFromBuffer(ImageSource *imageSource);
        Image *measureStats(ImageSource *imageSource, int exposure, int gain, 
                u_int16_t &minValue, u_int16_t &maxValue, u_int16_t &meanValue, float &stdValue, 
                float &slopeValue, u_int16_t &maxValueLimit);
        int measureNoise(ImageSource *imageSource);
};