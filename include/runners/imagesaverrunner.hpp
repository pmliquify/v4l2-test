#pragma once

#include <runners/basicstreamrunner.hpp>
#include <cv/imageConvertCPU.hpp>
#include <sinks/imageSaverSink.hpp>

class ImageSaverRunner : public BasicStreamRunner
{
public:
        ImageSaverRunner();

        void printArgs();
        int setup(CommandArgs &args);


private:
        ImageConvertCPU m_imageConvertCPU;
        bool m_firstRun = true;
        int m_scaleFactor = 1;
        int m_maxCount = 0;
        int counter = 0;
        ImageSaverSink imageSaverSink;
        
        virtual int processImage(ImageSource *imageSource, Image *image);
};
