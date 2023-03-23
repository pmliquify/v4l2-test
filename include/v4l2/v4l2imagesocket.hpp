#pragma once

#include <arpa/inet.h>
#include <string>
#include <v4l2image.hpp>


class V4L2ImageSocket 
{
public:
        V4L2ImageSocket();
        ~V4L2ImageSocket();

        int open(std::string address, u_int16_t port);
        int close();
        int listen(u_int16_t port);
        int accept();
        
        int send(V4L2Image image, int sendBuffSize);
        int receive(V4L2Image &image);

private:
        int m_fd;
        
        struct sockaddr_in m_addr;
};