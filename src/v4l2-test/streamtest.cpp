#include "streamtest.hpp"
#include <unistd.h>


StreamTest::StreamTest() :
        m_fb(false),
        m_x(-1),
        m_y(-1),
        m_width(0),
        m_height(0),
        m_delay(0),
        m_print(16),
        m_shift(0),
        m_imageCount(-1)
{
}

void StreamTest::printArgs()
{
        printArgSection("Stream test");
        printArg("--fb","Output the image to the framebuffer");
        printArg("-h",  "Set image height");
        printArg("-l",  "Set delay in us in image acquisition loop");
        printArg("-n",  "Number of images to captured");
        printArg("-p",  "Print image data in format (1: bin, 10: dec, 16: hex)");
        printArg("-w",  "Set image width");
        printArg("-x",  "Set x pixel position to print");
        printArg("-y",  "Set y pixel position to print");
}

int StreamTest::setup(CommandArgs &args)
{
        m_fb            = args.exists("--fb");
        m_height        = args.optionInt("-h", 0);
        m_delay         = args.optionInt("-l",  0);
        m_imageCount    = args.optionInt("-n", -1);
        m_print         = args.optionInt("-p", 16);
        m_width         = args.optionInt("-w", 0);
        m_x             = args.optionInt("-x", -1);
        m_y             = args.optionInt("-y", -1);
        return 0;
}

int StreamTest::exec(V4L2ImageSource &imageSource, V4L2Image &image) 
{
        // if (args.exists("-i")) {
        //         for (int index = 0; index < imageCount; index++) {
        //                 imageSource.getImage(image, 1000000);
        //                 processImage(args, imageSource, image, frameBuffer, fb, autoExposure, ae);
        //         }
        // }

        if (m_fb) {
                m_frameBuffer.open();
                m_frameBuffer.fill();
        }

        if (m_width > 0 || m_height > 0) {
                imageSource.setSelection((m_x==-1)?0:m_x, (m_y==-1)?0:m_y, m_width, m_height);
        }

        if (imageSource.streamOn(3) == 0) {
                int count = m_imageCount;
                int step = 1;
                if (count == -1) {
                        count = 1;
                        step = 0;
                }

                for (int index = 0; index < count; index *= step) {
                        if (0 == imageSource.getNextImage(image, 1000000)) {
                                if (m_delay > 0)
                                        usleep(m_delay);

                                image.print(m_print, m_x, m_y, 10);

                                if (m_fb)
                                        m_frameBuffer.print(image);

                                imageSource.releaseImage(image);
                        }
                }

                imageSource.streamOff();
        }

        if (m_fb) {
                m_frameBuffer.close();
        }

        return 0;
}