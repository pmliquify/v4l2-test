#include <v4l2imagesocket.hpp>
#include <errno.hpp>
#include <sys/socket.h>
#include <memory.h>
#include <iostream>


V4L2ImageSocket::V4L2ImageSocket()
{
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
}

V4L2ImageSocket::~V4L2ImageSocket()
{
        close();
}

int V4L2ImageSocket::open(std::string address, u_int16_t port)
{
        if (m_fd < 0) {
                return -1;
        }

        memset(&m_addr, 0, sizeof(m_addr)); 
        m_addr.sin_family = AF_INET;
        if (inet_aton(address.c_str() , &m_addr.sin_addr) == 0) {
                return -1;
        }
        m_addr.sin_port = htons(port);

        std::cout << "TCPSocket open(" << address << ":" << port << ")" << std::endl;

        return 0;
}

int V4L2ImageSocket::close()
{
        return 0;
}

int V4L2ImageSocket::listen(u_int16_t port)
{
        if (m_fd < 0) return -1;

        memset(&m_addr, 0, sizeof(m_addr)); 
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = INADDR_ANY;
        m_addr.sin_port = htons(port);

        if (::bind(m_fd, (const struct sockaddr *)&m_addr,  sizeof(m_addr)) < 0) { 
                return -1;
        } 
        if (::listen(m_fd, 1) < 0) {
                return -1;
        }

        std::cout << "TCPSocket listen(" << port << ")" << std::endl;

        return 0;
}

int V4L2ImageSocket::accept()
{
        // connfd = ::accept(m_fd, (struct sockaddr*)NULL, NULL); 
        return 0;
}

struct ImageHeader
{
        u_int16_t width;
        u_int16_t height;
        u_int16_t bytesPerLine;
        u_int32_t imageSize;
        u_int32_t bytesUsed;
        u_int32_t pixelformat;
        u_int32_t sequence;
        u_int64_t timestamp;
        u_int8_t  numPlanes;
};

int V4L2ImageSocket::send(V4L2Image image, int sendBuffSize)
{
        if (m_fd < 0) return -1;

        struct ImageHeader header;
        header.width = image.m_width;
        header.height = image.m_height;
        header.bytesPerLine = image.m_bytesPerLine;
        header.imageSize = image.m_imageSize;
        header.bytesUsed = image.m_bytesUsed;
        header.pixelformat = image.m_pixelformat;
        header.sequence = image.m_sequence;
        header.timestamp = image.m_timestamp;
        header.numPlanes = image.m_planes.size();

        u_int32_t sendSize = sizeof(header);
        ssize_t size = sendto(m_fd, (const char *)&header, sizeof(header), 
                MSG_CONFIRM, 
                (const struct sockaddr *) &m_addr, sizeof(m_addr));


        sendSize = (image.m_bytesUsed > 0) ? image.m_bytesUsed : image.m_imageSize;
        const char * data = (const char *)image.m_planes.at(0);
        int index = 0;
        while (index < sendSize) {
                size = sendto(m_fd, data + index, sendBuffSize, 
                        MSG_CONFIRM, 
                        (const struct sockaddr *) &m_addr, sizeof(m_addr));

                index += sendBuffSize;
        }

        index -= sendBuffSize;
        if (index < sendSize) {
                sendBuffSize = sendSize - index;
                size = sendto(m_fd, data + index, sendBuffSize, 
                        MSG_CONFIRM, 
                        (const struct sockaddr *) &m_addr, sizeof(m_addr));
        }

        return 0;
}

int V4L2ImageSocket::receive(V4L2Image &image)
{
        if (m_fd < 0) return -1;

        struct ImageHeader header;
        unsigned int len = sizeof(m_addr);
        ssize_t size = recvfrom(m_fd, (char *)&header, sizeof(header),  
                MSG_WAITALL, (struct sockaddr *) &m_addr, &len); 

        if (size == sizeof(header)) {
                if (image.m_imageSize != header.imageSize) {
                        if (image.m_imageSize != 0) {
                                for (int index=0; index<image.m_planes.size(); index++) {
                                        free(image.m_planes.at(index));
                                }
                        }
                        image.m_planes.resize(header.numPlanes);
                        image.m_imageSize = header.imageSize;
                        for (int index=0; index<image.m_planes.size(); index++) {
                                image.m_planes[index] = malloc(image.m_imageSize);
                        }
                }

                u_int32_t receiveSize = (image.m_bytesUsed > 0) ? image.m_bytesUsed : image.m_imageSize;
                char * data = (char *)image.m_planes[0];
                int index = 0;
                while (index < receiveSize) {
                        size = recvfrom(m_fd, data + index, receiveSize,  
                                MSG_WAITALL, (struct sockaddr *) &m_addr, &len); 

                        std::cout << "Receive (" << receiveSize << ", " << size << ", " << index << ")" << std::endl;

                        index += size;
                }
                                
                image.m_width = header.width;
                image.m_height = header.height;
                image.m_bytesPerLine = header.bytesPerLine;
                image.m_imageSize = header.imageSize;
                image.m_bytesUsed = header.bytesUsed;
                image.m_pixelformat = header.pixelformat;
                image.m_sequence = header.sequence;
                image.m_timestamp = header.timestamp;
        }

        return 0;
}