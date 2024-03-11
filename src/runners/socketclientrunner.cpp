#include <runners/socketclientrunner.hpp>
#include <cv/imageprint.hpp>
#include <utils/errno.hpp>
#include <unistd.h>


SocketClientRunner::SocketClientRunner() :
    m_port(0)
{
}

void SocketClientRunner::printArgs()
{
        printArgSection("Client runner");
        printArg("--port", "Set the servers port");
        printArg("--ip",   "Sets the servers ip address");
        BasicStreamRunner::printArgs();
}

int SocketClientRunner::setup(CommandArgs &args)
{
        BasicStreamRunner::setup(args);
        m_address = args.option("--ip", "localhost");
        m_port    = args.optionInt("--port", 9000);
        return 0;
}

int SocketClientRunner::run(ImageSource *imageSource)
{
        connectToServer();

        return BasicStreamRunner::run(imageSource);
}

int SocketClientRunner::processImage(ImageSource *imageSource, Image *image)
{
        unsigned int id;
        unsigned long value;
        while (m_socket.receiveControl(id, value) == 0) {
                switch (id) {
                case CID_EXPOSURE:
                        imageSource->setExposure(value);
                        break;
                case CID_GAIN:
                        imageSource->setGain(value);
                        break;
                case CID_BLACKLEVEL:
                        imageSource->setBlackLevel(value);
                        break;
                case CID_FRAMERATE:
                        imageSource->setFrameRate(value);
                        break;
                }
        }

        while (m_socket.sendImage(image) == -1) {
                printf("Connection to server %s:%u lost!\n", m_address.c_str(), m_port);
                m_socket.close();
                connectToServer();
        }

        return BasicStreamRunner::processImage(imageSource, image);
}

void SocketClientRunner::connectToServer()
{
        while (!m_socket.isConnected()) {
                printf("Open connection to server %s:%u \n", m_address.c_str(), m_port);
                int ret = -1;
                while (ret != 0) {               
                        ret = m_socket.open(m_address, m_port);
                        if (ret != 0) {
                                printf(".");
                                fflush(stdout);
                                usleep(1000000);
                        }
                }
                printf("\n");
        }
}