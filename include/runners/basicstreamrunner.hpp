#pragma once

#include <runners/imagesourcerunner.hpp>
#include <gui/framebuffer.hpp>
#ifdef WITH_GUI
#include <gui/viewer.hpp>
#endif


class BasicStreamRunner : public ImageSourceRunner
{
public:
        BasicStreamRunner();

        virtual void printArgs();
        virtual int setup(CommandArgs &args);
        virtual int run(ImageSource *imageSource);

protected:
        int           m_print;
        unsigned long m_lastTimestamp;
        FrameBuffer   m_frameBuffer;
        bool          m_fb;
#ifdef WITH_GUI
        Viewer        m_viewer;
#endif
        bool          m_gui;
        int           m_delay;
        int           m_x;
        int           m_y;
        int           m_width;
        int           m_height;
        bool          m_singleAcquisition;
        int           m_imageCount;

        virtual int processImage(ImageSource *imageSource, Image *image);
};