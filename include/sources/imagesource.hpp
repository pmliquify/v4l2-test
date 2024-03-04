#pragma once

#include <utils/commandargsconsumer.hpp>
#include <cv/image.hpp>


class ImageSource : public CommandArgsConsumer
{
public:
        virtual int open(const std::string devicePath, const std::string subDevicePath) { return 0; }
        virtual int close() { return 0; }
        virtual int getFormat() { return 0; }
        virtual int setFormat(int pixelFormat = 0) { return 0; }
        virtual int setSelection(int left, int top, int width, int height) { return 0; }
        
        virtual int streamOn(int bufferCount = 3) { return 0; }
        virtual int streamOff() { return 0; }
        virtual Image *getNextImage(int timeout, bool lastImage = true) { return 0; }
        virtual int releaseImage(Image *image) { return 0; }
        virtual Image *getImage(int timeout, bool lastImage = true) { return 0; }

        virtual int setExposure(int exposure)  { return 0; }
        virtual int setGain(int gain) { return 0; }
        virtual int setBlackLevel(int blackLevel) { return 0; }
        virtual int setBinning(int binning) { return 0; }
        virtual int setTriggerMode(int triggerMode) { return 0; }
        virtual int setIOMode(int ioMode) { return 0; }
        virtual int setFrameRate(int frameRate) { return 0; }
};