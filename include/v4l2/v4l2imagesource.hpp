#pragma once

#include <string>
#include <linux/videodev2.h>
#include <commandargclass.hpp>
#include <v4l2image.hpp>


class V4L2ImageSource : public CommandArgClass
{
public:
        V4L2ImageSource();
        ~V4L2ImageSource();

        void printArgs();
        int setup(CommandArgs &args);

        int open(const std::string DevicePath, const std::string subDevicePath);
        int close();
        int getFormat();
        int setFormat(int PixelFormat);
        int printFormat();
        int setSelection(int Left, int Top, int Width, int Height);
        
        int streamOn(int BufferCount = 3);
        int streamOff();
        int getNextImage(V4L2Image &Image, int Timeout, bool LastImage = true);
        int releaseImage(V4L2Image &Image);
        int getImage(V4L2Image &image, int Timeout, bool LastImage = true);

        int setGain(int gain);
        int setExposure(int exposure);
        int setBlackLevel(int blackLevel);
        int setBinning(int binning);
        int setTriggerMode(int triggerMode);
        int setIOMode(int ioMode);
        int setFrameRate(int frameRate);

protected:
        int m_deviceFd;
        int m_subDeviceFd;
        
        int setControl(unsigned int id, int value);
        int setExtControl(unsigned int id, unsigned int type, int value);
        int setControl(std::string name, int value);

private:
        struct v4l2_format m_format;
        struct Buffer {
                struct v4l2_buffer buffer;
                void **ptrs;
        } *m_buffers;
        int m_bufferCount;
        int m_nextBufferIndex;

        int initBuffers(int Count);
        void clearBuffers();
        int enqueueBuffer(int BufferIndex);
        struct v4l2_buffer * dequeueBuffer(int BufferIndex);
        int waitForNextBuffer(int Timeout);

        void handleErrorForOpen(const char *path, int err);
        void handleErrorForClose(int fd, int err);
        void handleErrorForSelect(int fd, int err);
        void handleErrorForIoctl(unsigned long int request, int err);
};