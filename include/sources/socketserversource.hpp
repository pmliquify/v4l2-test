#pragma once

#include <sources/imagesource.hpp>
#include <network/imagesocket.hpp>


class SocketServerSource : public ImageSource
{
public:
        SocketServerSource();
        ~SocketServerSource();

        void printArgs();
        int setup(CommandArgs &args);

        virtual Image *getNextImage(int timeout, bool lastImage = true);

        virtual int setGain(int gain);
        virtual int setExposure(int exposure);
        virtual int setBlackLevel(int blackLevel);
        virtual int setFrameRate(int frameRate);

private:
        ImageSocketServer m_socket;
        Image *           m_image;
};