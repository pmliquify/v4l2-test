#pragma once

#include <string>
#include <cv/image.hpp>


#define CID_EXPOSURE    1
#define CID_GAIN        2
#define CID_BLACKLEVEL  3
#define CID_FRAMERATE   4

class Socket
{
public:
        Socket();
        ~Socket();

        bool isConnected();
        int close();

protected:
        int                m_socket;
        bool               m_connected;

        int send(int __fd, const void *__buf, size_t __n);
        int receive(int __fd, void *__restrict __buf, size_t __n, int __flags);
        void handleErrorForRecv(int err);
};

class ImageSocketClient : public Socket
{
public:
        ImageSocketClient();
        ~ImageSocketClient();

        int open(std::string address, unsigned short port);

        int receiveControl(unsigned int &id, unsigned long& value);
        int sendImage(const Image *image);
};

class ImageSocketServer : public Socket
{
public:
        ImageSocketServer();
        ~ImageSocketServer();

        int listen(unsigned short port);
        int accept();
        int close();
        
        int sendControl(unsigned int id, unsigned long value);
        int receiveImage(Image *image);

private:
        int                m_client;
        bool               m_listening;
};