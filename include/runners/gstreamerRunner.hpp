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
        int run(ImageSource *imageSource) override;

private:
        GstreamerSink m_gstreamerSink;
        ImageConvertCPU m_imageConvertCPU;
        int m_scaleFactor = 1;

        virtual int processImage(ImageSource *imageSource, Image *image);
};
