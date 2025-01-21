#pragma once

#include <runners/basicstreamrunner.hpp>
#include <cv/imageConvertCPU.hpp>
#include <sinks/gstreamerSink.hpp>

class GstreamerRunner : public BasicStreamRunner
{
public:
        GstreamerRunner();

        void printArgs();
        int setup(CommandArgs &args);


private:
        GstreamerSink m_gstreamerSink;
        ImageConvertCPU m_imageConvertCPU;
        bool m_firstRun = true;

        
        virtual int processImage(ImageSource *imageSource, Image *image);
};
