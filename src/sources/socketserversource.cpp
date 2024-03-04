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
        u_int16_t port = args.optionInt("--port", 9000);
        if (m_socket.listen(port) != 0) {
                printf("Unable to bind port %u!\n", port);
                return -1;
        }
        printf("Listen on port %u\n", port);
        return 0;
}

Image *SocketServerSource::getNextImage(int timeout, bool lastImage)
{
        if (!m_socket.isConnected()) {
                printf("Wait for client to connect ...\n");
                if (m_socket.accept() == 0) {
                printf("Client connected!\n");
                } else {
                usleep(1000000);
                }
        }
        
        if (m_socket.receiveImage(m_image) == 0) {
                return m_image;
        }

        printf("Connection to client closed!\n");
        return NULL;
}

int SocketServerSource::setExposure(int exposure)  
{ 
        return m_socket.sendControl(CID_EXPOSURE, exposure);; 
}

int SocketServerSource::setGain(int gain) 
{
        return m_socket.sendControl(CID_GAIN, gain); 
}