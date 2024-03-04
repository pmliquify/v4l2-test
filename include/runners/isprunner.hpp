#pragma once

#include <runners/basicstreamrunner.hpp>
#include <sources/autoexposure.hpp>


class IspRunner : public BasicStreamRunner
{
public:
        IspRunner();

        void printArgs();
        int setup(CommandArgs &args);

private:
        AutoExposure m_autoExposure;
        
        virtual int processImage(ImageSource *imageSource, Image *image);
};
