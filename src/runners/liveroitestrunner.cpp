#include <runners/liveroitestrunner.hpp>
#include <cmath>
#include <chrono>


LiveRoiTestRunner::LiveRoiTestRunner() :
        m_aWidth(4032),
        m_aHeight(3040),
        m_oWidth(2016),
        m_oHeight(1520),
        m_angle(0),
        m_time(0),
        m_timeCount(0)
{
}

void LiveRoiTestRunner::printArgs()
{
        SocketClientRunner::printArgs();
        
        printArgSection("DWE Test");
        printArg("--aCrop", "Sets analog cropping size");
        printArg("--dCrop", "Sets digital cropping size");
}

int LiveRoiTestRunner::setup(CommandArgs &args)
{
        SocketClientRunner::setup(args);

        auto aCrop = args.optionArray<int , 2>("-aCrop", { 4032, 3040 });
        m_aWidth = aCrop[0];
        m_aHeight = aCrop[1];
        auto oCrop = args.optionArray<int , 2>("-oCrop", { 2016, 1520 });
        m_oWidth = oCrop[0];
        m_oHeight = oCrop[1];

        return 0;
}

int LiveRoiTestRunner::processImage(ImageSource *imageSource, Image *image)
{
        int leftMin = 0;
        int leftMax = m_aWidth - m_oWidth;
        int leftCenter = (leftMin + leftMax) / 2;
        int leftRange = (leftMax - leftMin) / 2;

        int topMin = 0;
        int topMax = m_aHeight - m_oHeight;
        int topCenter = (topMin + topMax) / 2;
        int topRange = (topMax - topMin) / 2;

        double radians = m_angle++ * M_PI / 180.0;
        int rx = static_cast<int>(leftCenter + leftRange * sin(radians));
        int ry = static_cast<int>(topCenter + topRange * cos(radians));

        auto t0 = std::chrono::steady_clock::now();

        imageSource->setLiveRoi(16, rx, ry);
        
        auto t1 = std::chrono::steady_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        m_time += us;
        m_timeCount++;
        if (m_time > 1000000) {
                printf("Average execution time %u us\n", m_time/m_timeCount);
                m_time = 0;
                m_timeCount = 0;
        }

        return SocketClientRunner::processImage(imageSource, image);
}