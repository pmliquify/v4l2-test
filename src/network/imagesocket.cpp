/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#include <network/imagesocket.hpp>
#include <utils/errno.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <memory.h>


// *** Socket *****************************************************************

Socket::Socket() :
        m_socket(0),
        m_connected(false)
{

}

Socket::~Socket()
{
        close();
}

bool Socket::isConnected()
{
        // unsigned char buf;
        // int err = ::recv(m_socket , &buf, 1, MSG_PEEK);
        // return err == -1 ? false : true;

        return m_connected;
}

int Socket::close()
{
        shutdown(m_socket, SHUT_RDWR);
        m_socket = 0;
        m_connected = false;
        return 0;
}

int Socket::send(int __fd, const void *__buf, size_t __n)
{
        int error_code = 0;
        socklen_t error_code_size = sizeof(error_code);
        getsockopt(__fd, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
        if (error_code != 0) {
                return -1;
        }

        return ::send(__fd, __buf, __n, MSG_CONFIRM);
}

int Socket::receive(int __fd, void *__restrict __buf, size_t __n, int __flags)
{
        int size = ::recv(__fd, __buf, __n, __flags);
        if (size == -1 && __flags & MSG_DONTWAIT) {
                return 0;
        }
        if (size == -1) {
                handleErrorForRecv(errno);
                return 0;
        }
        return size;
}

void Socket::handleErrorForRecv(int err)
{
        printf("%s\n", errorsForRecv(err));
}



// *** Structs ****************************************************************

struct ImageHeader
{
        unsigned short  width;
        unsigned short  height;
        unsigned short  bytesPerLine;
        unsigned int    imageSize;
        unsigned int    bytesUsed;
        unsigned int    pixelformat;
        unsigned int    sequence;
        unsigned long   timestamp;
        unsigned char   numPlanes;
        unsigned short  shift;
};

struct ControlHeader
{
        unsigned int id;
        unsigned long value;
};



// *** ImageSocketClient ******************************************************

ImageSocketClient::ImageSocketClient()
{
        
}

ImageSocketClient::~ImageSocketClient()
{

}

int ImageSocketClient::open(std::string address, unsigned short port)
{
        if (isConnected()) {
                return -1;
        }

        m_socket = socket(AF_INET, SOCK_STREAM, 0);

        struct hostent *server = gethostbyname(address.c_str());
        if (server == NULL) {
                return -1;
        }
     
        struct sockaddr_in addr; 
        memset(&addr, 0, sizeof(addr)); 
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);
        
        int ret = connect(m_socket, (const struct sockaddr *)&addr, sizeof(addr));
        if (ret != 0) {
                return -1;
        }

        m_connected = true;
        return 0;
}

int ImageSocketClient::receiveControl(unsigned int &id, unsigned long& value)
{
        struct ControlHeader header;
        ssize_t size = receive(m_socket, (char *)&header, sizeof(header), MSG_PEEK | MSG_DONTWAIT);
        if (size < sizeof(header)) {
                return -1;
        }

        size = receive(m_socket, (char *)&header, sizeof(header), MSG_WAITALL);
        if (size == sizeof(header)) {
                id = header.id;
                value = header.value;
                return 0;
        }

        close();
        return -1;
}

int ImageSocketClient::sendImage(const Image *image)
{
        if (!isConnected())  {
                return -1;
        }

        struct ImageHeader header;
        header.width = image->width();
        header.height = image->height();
        header.pixelformat = image->pixelformat();
        header.imageSize = image->size();
        header.numPlanes = image->planes().size();
        header.bytesPerLine = image->bytesPerLine();
        header.sequence = image->sequence();
        header.timestamp = image->timestamp();
        header.shift = image->shift();

        unsigned int sendSize = sizeof(header);
        ssize_t size = send(m_socket, (const char *)&header, sendSize);
        if (size < 0) {
                return -1;
        }

        for (int planeIx=0; planeIx<image->planes().size(); planeIx++) {
                unsigned int planeSize = image->plane(planeIx).size();
                ssize_t size = send(m_socket, (const char *)&planeSize, sizeof(planeSize));
                if (size < 0) {
                        return -1;
                }

                const unsigned char * plane = image->plane(planeIx).data();
                int index = 0;
                while (index < planeSize) {
                        const unsigned char *data = plane + index;
                        unsigned int remainingSize = planeSize - index;
                        size = send(m_socket, data, remainingSize);
                        if (size < 0) {
                                return -1;
                        }
                        index += size;
                }
                if (index < planeSize) {
                        return -1;
                }
        }

        return 0;
}



// *** ImageSocketServer ******************************************************

ImageSocketServer::ImageSocketServer() :
        m_client(0),
        m_listening(false)
{
}

ImageSocketServer::~ImageSocketServer()
{
        close();
}

int ImageSocketServer::listen(unsigned short port)
{
        if (m_listening) {
                return -1;
        }

        m_socket = ::socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr; 
        memset(&addr, 0, sizeof(addr)); 
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (::bind(m_socket, (const struct sockaddr *)&addr,  sizeof(addr)) < 0) { 
                return -1;
        } 
        if (::listen(m_socket, 1) < 0) {
                return -1;
        }

        m_listening = true;
        return 0;
}

int ImageSocketServer::accept()
{
        if (!m_listening) {
                return -1;
        }

        int fd = ::accept(m_socket, (struct sockaddr*)NULL, NULL); 
        if (fd == -1) {
                return -1;
        }

        m_client = fd;
        m_connected = true;
        return 0;
}

int ImageSocketServer::close()
{
        shutdown(m_client, SHUT_RDWR);
        m_client = 0;
        m_connected = false;
        return 0;
}

int ImageSocketServer::sendControl(unsigned int id, unsigned long value)
{
        if (!isConnected()) {
                return -1;
        }

        struct ControlHeader header;
        header.id = id;
        header.value = value;
        ssize_t size = send(m_client, (const char *)&header, sizeof(header));
        if (size < 0) {
                return -1;
        }

        return 0;
}

int ImageSocketServer::receiveImage(Image *image)
{
        if (!isConnected()) {
                return -1;
        } 

        struct ImageHeader header;
        ssize_t size = receive(m_client, (char *)&header, sizeof(header), MSG_WAITALL);

        if (size == sizeof(header)) {
                image->init(header.width, header.height, 
                        header.pixelformat,
                        header.imageSize, header.bytesPerLine,
                        header.sequence, header.timestamp);
                image->setShift(header.shift);
                image->planes().resize(header.numPlanes);
                for (int planeIx=0; planeIx<header.numPlanes; planeIx++) {
                        unsigned int planeSize = 0;
                        size = receive(m_client, (char *)&planeSize, sizeof(planeSize), MSG_WAITALL);
                        if (size != sizeof(planeSize)) {
                                close();
                                return -1;
                        }
                        image->plane(planeIx).init(planeSize);
                        unsigned int receiveSize = planeSize;
                        unsigned char *plane = (unsigned char *)image->plane(planeIx).data();
                        int index = 0;
                        while (index < receiveSize) {
                                unsigned char *data = plane + index;
                                unsigned int remainingSize = receiveSize - index;
                                size = receive(m_client, data, remainingSize, MSG_WAITALL);
                                if (size <= 0) {
                                        close();
                                        return -1;
                                }
                                index += size;
                        }
                }

                return 0;
        }

        close();
        return -1;
}

