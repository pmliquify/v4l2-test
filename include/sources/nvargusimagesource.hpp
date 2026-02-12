#pragma once

#include <sources/imagesource.hpp>


typedef struct _GstElement GstElement;

class NvArgusImageSource : public ImageSource
{
public:
        NvArgusImageSource();
        ~NvArgusImageSource();

        void printArgs();
        int setup(CommandArgs &args);

        int open(const std::string devicePath, const std::string subDevicePath);
        int close();
        int setSelection(int left, int top, int width, int height);

        int streamOn(int bufferCount = 3);
        int streamOff();

        int getNextImage(Image *&image, int timeout, bool lastImage);
        int releaseImage(Image *image);

private:
        Image*          m_image;
        GstElement*     m_pipeline;
        GstElement*     m_sink;

        int             m_sensorId;
        bool            m_aeLock;
        int             m_aeLeft;
        int             m_aeTop;
        int             m_aeWidth;
        int             m_aeHeight;
        int             m_gainRange;
        int             m_ispDigitalGainRange;
        bool            m_awbLock;
        int             m_wbMode;
        int             m_tnrMode;
        int             m_width;
        int             m_height;
        int             m_frameRate;
        unsigned int    m_sequence;
};