#include <runners/noisetestrunner.hpp>
#include <cv/imagestats.hpp>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <linux/videodev2.h>


NoiseTestRunner::NoiseTestRunner() :
        m_bufferCount(3),
        m_subSampling(32),
        m_exposureStart(0),
        m_exposureEnd(10000),
        m_exposureStep(2000),
        m_gainStart(0),
        m_gainEnd(48000),
        m_gainStep(4000),
        m_writeRowStats(false)
{
}

void NoiseTestRunner::printArgs()
{
        printArgSection("Noise test runner");
        printArg("--ntSub",             "-");
        printArg("--ntExposureStart",   "-");
        printArg("--ntExposureEnd",     "-");
        printArg("--ntExposureStep",    "-");
        printArg("--ntGainStart",       "-");
        printArg("--ntGainEnd",         "-");
        printArg("--ntGainStep",        "-");
        printArg("--ntWriteRowStats",   "-");
}

int NoiseTestRunner::setup(CommandArgs &args)
{
        m_subSampling   = args.optionInt("--ntSub", 32);
        m_exposureStart = args.optionInt("--ntExposureStart", 0);
        m_exposureEnd   = args.optionInt("--ntExposureEnd", 20000);
        m_exposureStep  = args.optionInt("--ntExposureStep", 1000);
        m_gainStart     = args.optionInt("--ntGainStart", 0);
        m_gainEnd       = args.optionInt("--ntGainEnd", 48000);
        m_gainStep      = args.optionInt("--ntGainStep", 4000);
        m_writeRowStats = args.exists("--ntWriteRowStats");

        return 0;
}

void NoiseTestRunner::getAllImagesFromBuffer(ImageSource *imageSource)
{
        for (int i=0; i<m_bufferCount; i++) {
                Image *image = imageSource->getNextImage(1000000);
                imageSource->releaseImage(image);
        }
}

float dBtoFactor(float dB)
{
        return powf(10.0, dB / 20.0);
}

Image *NoiseTestRunner::measureStats(ImageSource *imageSource, int exposure, int gain, 
        u_int16_t &minValue, u_int16_t &maxValue, u_int16_t &meanValue, float &stdValue, 
        float &slopeValue, u_int16_t &maxValueLimit)
{
        imageSource->setBlackLevel(0);
        imageSource->setExposure(exposure);
        imageSource->setGain(gain);

        getAllImagesFromBuffer(imageSource);

        Image *image = imageSource->getNextImage(1000000);
        if (image == NULL) {
                return NULL;
        }

        std::ofstream file;
        if (m_writeRowStats) {
                std::locale locale("de_DE.UTF-8");
                file.imbue(locale);
                char fileName[256];
                sprintf(fileName, "rows_%uus_%umdB.csv", exposure, gain);
                file.open(fileName);
        }

        u_int16_t x = image->width()/2;
        std::vector<u_int16_t> min, max, mean;
        std::vector<float> std;
        ImageStats::rowStats(image, min, max, mean, m_subSampling);
        ImageStats::rowStd(image, mean, std, m_subSampling);
        
        minValue = -1;
        maxValue = 0;
        meanValue = 0;
        stdValue = 0;
        u_int64_t meanSum = 0;
        for (u_int16_t index = 0; index < mean.size(); index++) {
                if (m_writeRowStats) {
                        file << index*m_subSampling << ";" << min[index] << ";" << max[index] << ";"
                                << mean[index] << ";" << std[index] << std::endl;
                }

                if (min[index] < minValue) {
                        minValue = min[index];
                }
                if (max[index] > maxValue) {
                        maxValue = max[index];
                }
                meanSum += mean[index];
                stdValue += std[index];
        }
        meanValue = meanSum / mean.size();
        stdValue /= mean.size();
        u_int16_t height = (image->height()/m_subSampling)*m_subSampling;
        slopeValue = (float)(mean[mean.size()] - mean[0])/height;

        if (m_writeRowStats) {
                file.close();
        }

        maxValueLimit = 0;
        switch (image->pixelformat()) {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8: 
                maxValueLimit = 255;
                break;
        case V4L2_PIX_FMT_Y10:
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SBGGR10:      
                maxValueLimit = 1023;
                break;
        case V4L2_PIX_FMT_Y12:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SBGGR12:
                maxValueLimit = 2095;
                break;
        }

        return image;
}

int NoiseTestRunner::measureNoise(ImageSource *imageSource)
{
        std::locale locale("de_DE.UTF-8");
        std::ofstream fileMean;
        fileMean.imbue(locale);
        std::ofstream fileStd;
        fileStd.imbue(locale);
        std::ofstream fileSlope;
        fileSlope.imbue(locale);

        fileMean.open("mean.csv");
        fileStd.open("noise.csv");
        fileSlope.open("slope.csv");

        // Write two lines of table header
        for (int gain=m_gainStart; gain<=m_gainEnd; gain+=m_gainStep) {
                fileMean << ";" << gain;
                fileStd << ";" << gain;
                fileSlope << ";" << gain;
        }
        fileMean << std::endl;
        fileStd << std::endl;
        fileSlope << std::endl;
        for (int gain=m_gainStart; gain<=m_gainEnd; gain+=m_gainStep) {
                fileMean << ";" << dBtoFactor(gain/1000);
                fileStd << ";" << dBtoFactor(gain/1000);
                fileSlope << ";" << dBtoFactor(gain/1000);
        
        }
        fileMean << std::endl;
        fileStd << std::endl;
        fileSlope << std::endl;

        for (int exposure=m_exposureStart; exposure<=m_exposureEnd; exposure+=m_exposureStep) {
                fileMean << exposure;
                fileStd << exposure;
                fileSlope << exposure;
                for (int gain=m_gainStart; gain<=m_gainEnd; gain+=m_gainStep) {
                        u_int16_t minValue = 0;
                        u_int16_t maxValue = 0;
                        u_int16_t meanValue = 0;
                        float stdValue = 0.0;
                        float slopeValue = 0.0;
                        u_int16_t maxValueLimit = 0;
                        Image *image = measureStats(imageSource, exposure, gain, 
                                minValue, maxValue, meanValue, stdValue, slopeValue, maxValueLimit);
                        
                        if (minValue > 0 && maxValue < maxValueLimit) {
                                printf("[#%04d, exposure: %6u, gain: %6u] -> "
                                        "(min: %6u, max: %6u, mean: %6u, std: %6.2f, slope: %6.2f)\n",
                                                image->sequence(), exposure, gain, 
                                                minValue, maxValue, meanValue, stdValue, slopeValue);
                                fileMean << ";" << meanValue;
                                fileStd << ";" << stdValue;
                                fileSlope << ";" << slopeValue;
                        } else {
                                fileMean << ";";
                                fileStd << ";";
                                fileSlope << ";";
                        }

                        imageSource->releaseImage(image);
                }
                fileMean << std::endl;
                fileStd << std::endl;
                fileSlope << std::endl;
        }

        fileMean.close();
        fileStd.close();
        fileSlope.close();
        return 0;
}

int NoiseTestRunner::run(ImageSource *imageSource)
{
        std::cout << "Start noise test ..." << std::endl;

        if (0 != imageSource->streamOn(m_bufferCount)) {
                return -1;
        }

        measureNoise(imageSource);
        
        imageSource->streamOff();

        return 0;
}