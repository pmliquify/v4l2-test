/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#include <sources/socketserversource.hpp>
#include <unistd.h>


SocketServerSource::SocketServerSource() :
        m_image(NULL)
{
        m_image = new Image();
}

SocketServerSource::~SocketServerSource()
{
        delete m_image;
        m_image = NULL;
}

void SocketServerSource::printArgs()
{
        printArgSection("SocketServerSource");
        printArg("--port", "Sets the server port");
}

int SocketServerSource::setup(CommandArgs &args)
{
        unsigned short port = args.optionInt("--port", 9000);
        if (m_socket.listen(port) != 0) {
                printf("Unable to bind port %u!\n", port);
                return -1;
        }
        printf("Listen on port %u\n", port);
        return 0;
}

int SocketServerSource::getNextImage(Image *&image, int timeout, bool lastImage)
{
        image = NULL;

        if (!m_socket.isConnected()) {
                printf("Wait for client to connect ...\n");
                if (m_socket.accept() == 0) {
                        printf("Client connected!\n");
                } else {
                        usleep(1000000);
                }
        }
        
        if (m_socket.receiveImage(m_image) == 0) {
                image = m_image;
                return 0;
        }

        printf("Connection to client closed!\n");
        return -1;
}

int SocketServerSource::setExposure(int exposure)  
{ 
        return m_socket.sendControl(CID_EXPOSURE, exposure);; 
}

int SocketServerSource::setGain(int gain) 
{
        return m_socket.sendControl(CID_GAIN, gain); 
}

int SocketServerSource::setBlackLevel(int blackLevel) 
{ 
        return m_socket.sendControl(CID_BLACKLEVEL, blackLevel);; 
}

int SocketServerSource::setFrameRate(int frameRate) 
{ 
        return m_socket.sendControl(CID_FRAMERATE, frameRate);; 
}