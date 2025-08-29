#pragma once

#include <runners/socketclientrunner.hpp>


class LiveRoiTestRunner : public SocketClientRunner
{
public:
        LiveRoiTestRunner();

        void printArgs();
        int setup(CommandArgs &args);

private:       
        virtual int processImage(ImageSource *imageSource, Image *image);

        int     m_aWidth;
        int     m_aHeight;
        int     m_oWidth;
        int     m_oHeight;
        int     m_angle;
        int     m_time;
        int     m_timeCount;
};
