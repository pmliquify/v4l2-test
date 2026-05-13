#pragma once

#include <runners/basicstreamrunner.hpp>
#include <cv/imageConvertCPU.hpp>
#include <sinks/videoSink.hpp>

class VideoRunner : public BasicStreamRunner
{
public:
    VideoRunner();

    void printArgs();
    int setup(CommandArgs &args);

private:
    ImageConvertCPU m_imageConvertCPU;
    VideoSink       m_videoSink;
    bool            m_firstRun   = true;
    int             m_scaleFactor = 1;
    int             m_maxCount   = 0;
    int             m_counter    = 0;

    virtual int processImage(ImageSource *imageSource, Image *image);
};
