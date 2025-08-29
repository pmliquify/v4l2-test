#pragma once

#include <runners/basicstreamrunner.hpp>
#include <network/imagesocket.hpp>


class SocketClientRunner : public BasicStreamRunner
{
public:
    SocketClientRunner();

    virtual void printArgs();
    virtual int setup(CommandArgs &args);
    virtual int run(ImageSource *imageSource);

protected:
    virtual int processImage(ImageSource *imageSource, Image *image);

private:
    ImageSocketClient m_socket;
    std::string       m_address;
    int               m_port;
    
    void connectToServer();
};
