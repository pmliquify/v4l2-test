#pragma once

#include <sources/imagesource.hpp>
#include <sources/v4l2image.hpp>
#include <string>
#include <linux/videodev2.h>


class V4L2ImageSource : public ImageSource
{
public:
        V4L2ImageSource();
        ~V4L2ImageSource();

        void printArgs();
        int setup(CommandArgs &args);

        int open(const std::string devicePath, const std::string subDevicePath);
        int close();
        int getFormat();
        int setFormat(int pixelFormat = 0);
        int printFormat();
        int setSelection(int left, int top, int width, int height);
        
        int streamOn(int bufferCount = 3);
        int streamOff();
        int getNextImage(Image *&image, int timeout, bool lastImage = true);
        int releaseImage(Image *image);
        int getImage(Image *&image, int timeout, bool lastImage = true);

        int setExposure(int exposure);
        int setGain(int gain);
        int setBlackLevel(int blackLevel);
        int setBinning(int binning);
        int setTriggerMode(int triggerMode);
        int setIOMode(int ioMode);
        int setFrameRate(int frameRate);

protected:
        int m_deviceFd;
        int m_subDeviceFd;
        
        int setControl(unsigned int id, int value, bool printError = true);
        int setExtControl(unsigned int id, unsigned int type, int value, bool printError = true);
        int setControl(std::string name, int value, bool printError = true);

private:
        struct v4l2_format m_format;
        struct Buffer {
                struct v4l2_buffer buffer;
                unsigned char **ptrs;
        } *m_buffers;

        V4L2Image *m_image;
        unsigned int m_bufferCount;
        unsigned int m_nextBufferIndex;

        int initBuffers(int count);
        void clearBuffers();
        int enqueueBuffer(int bufferIndex);
        struct v4l2_buffer * dequeueBuffer(unsigned int &bufferIndex);
        int waitForNextBuffer(int timeout);

        void handleErrorForOpen(const char *path, int err);
        void handleErrorForClose(int fd, int err);
        void handleErrorForSelect(int fd, int err);
        void handleErrorForIoctl(unsigned long int request, int err);
};