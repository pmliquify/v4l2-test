
#include "noisetest.hpp"
#include <iostream>
#include <fstream>


NoiseTest::NoiseTest() :
        m_subSampling(32),
        m_exposureStart(0),
        m_exposureEnd(10000),
        m_exposureStep(1000),
        m_gainStart(0),
        m_gainEnd(48000),
        m_gainStep(2000)
{
}

void NoiseTest::printArgs()
{
        printArgSection("Noise test");
        printArg("--ntSub",             "-");
        printArg("--ntExposureStart",   "-");
        printArg("--ntExposureEnd",     "-");
        printArg("--ntExposureStep",    "-");
        printArg("--ntGainStart",       "-");
        printArg("--ntGainEnd",         "-");
        printArg("--ntGainStep",        "-");
}

int NoiseTest::setup(CommandArgs &args)
{
        m_subSampling   = args.optionInt("--ntSub", 32);
        m_exposureStart = args.optionInt("--ntExposureStart", 0);
        m_exposureEnd   = args.optionInt("--ntExposureEnd", 1000);
        m_exposureStep  = args.optionInt("--ntExposureStep", 50);
        m_gainStart     = args.optionInt("--ntGainStart", 0);
        m_gainEnd       = args.optionInt("--ntGainEnd", 12000);
        m_gainStep      = args.optionInt("--ntGainStep", 1000);

        return 0;
}

int NoiseTest::exec(V4L2ImageSource &imageSource, V4L2Image &image)
{
        std::cout << "Start Noise Test ..." << std::endl;

        if (0 != imageSource.streamOn(3)) {
                return -1;
        }

        std::ofstream file;
        file.open("test.txt");

        // Write table header
        for (int gain=m_gainStart; gain<=m_gainEnd; gain+=m_gainStep) {
                file << ";" << gain;
        }

        for (int exposure=m_exposureStart; exposure<=m_exposureEnd; exposure+=m_exposureStep) {
                for (int gain=m_gainStart; gain<=m_gainEnd; gain+=m_gainStep) {
                        imageSource.setExposure(exposure);
                        imageSource.setGain(gain);

                        if (0 == imageSource.getNextImage(image, 1000000)) {
                                u_int16_t min, max, mean;
                                image.stats(min, max, mean, m_subSampling);
                                if (max < 1023) {
                                        printf("[#%04d, exposure: %6u, gain: %6u] -> (min: %5u, max: %5u, "
                                                "range: %5u, mean: %5u)\n",
                                                image.sequence(), exposure, gain, min, max, max-min, mean);
                                }
                                
                                imageSource.releaseImage(image);
                        }
                }
        }

        file.close();
        imageSource.streamOff();

        return 0;
}