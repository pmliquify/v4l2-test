#include <runners/basicstreamrunner.hpp>
#include <cv/imageprint.hpp>
#include <unistd.h>


BasicStreamRunner::BasicStreamRunner() :
        m_print(16),
        m_lastTimestamp(0),
        m_fb(false),
        m_gui(false),
        m_delay(0),
        m_x(-1),
        m_y(-1),
        m_width(0),
        m_height(0),
        m_singleAcquisition(false),
        m_imageCount(-1)
{
}

void BasicStreamRunner::printArgs()
{
        printArg("-p",  "Print image data in format (1: bin, 10: dec, 16: hex)");
        printArg("--fb","Output the image to the framebuffer");
        printArg("--gui","Output the image to the viewer gui");
        printArg("-l",  "Set delay in us in image acquisition loop");
        printArg("-h",  "Set image height");
        printArg("-i",  "Start single image acquisition");
        printArg("-n",  "Number of images to captured");
        printArg("-w",  "Set image width");
        printArg("-x",  "Set x pixel position to print");
        printArg("-y",  "Set y pixel position to print");
}

int BasicStreamRunner::setup(CommandArgs &args)
{
        m_print                 = args.optionInt("-p", -1);
        m_fb                    = args.exists("--fb");
        m_gui                   = args.exists("--gui");
        m_delay                 = args.optionInt("-l",  0);
        m_height                = args.optionInt("-h", 0);
        m_singleAcquisition     = args.exists("-i");
        m_imageCount            = args.optionInt("-n", -1);
        m_width                 = args.optionInt("-w", 0);
        m_x                     = args.optionInt("-x", -1);
        m_y                     = args.optionInt("-y", -1);
        return 0;
}

int BasicStreamRunner::processImage(ImageSource *imageSource, Image *image)
{
        if (m_delay > 0) {
                usleep(m_delay);
        }

        if (m_print != -1) {
                if (m_lastTimestamp == 0) {
                        m_lastTimestamp = image->timestamp();
                }
                ImagePrint::print(image, m_print, m_x, m_y, m_lastTimestamp, 10);
                m_lastTimestamp = image->timestamp();
        } else {
                printf(">");
                fflush(stdout);
        }

        if (m_fb) {
                m_frameBuffer.update(image);
        }

        if (m_gui) {
                m_viewer.update(image);
        }

        return 0;
}

int BasicStreamRunner::run(ImageSource *imageSource) 
{
        if (m_fb) {
                m_frameBuffer.open();
                m_frameBuffer.fill();
        }

        if (m_gui) {
                m_viewer.show(imageSource);
        }

        if (m_width > 0 || m_height > 0) {
                imageSource->setSelection((m_x==-1)?0:m_x, (m_y==-1)?0:m_y, m_width, m_height);
        }
        
        int count = m_imageCount;
        int step = 1;
        if (count == -1) {
                count = 1;
                step = 0;
        }

        if (m_singleAcquisition) {
                for (int index = 0; index < count; index += step) {
                        Image *image = imageSource->getImage(1000000);
                        if (image != NULL) {
                                processImage(imageSource, image);
                        }
                }
        } else {
                if (imageSource->streamOn(3) == 0) {
                        for (int index = 0; index < count; index += step) {
                                Image *image = imageSource->getNextImage(1000000);
                                if (image != NULL) {
                                        processImage(imageSource, image);
                                        imageSource->releaseImage(image);
                                }
                        }
                        imageSource->streamOff();
                }
        }

        if (m_fb) {
                m_frameBuffer.close();
        }

        if (m_gui) {
                m_viewer.hide();
        }

        return 0;
}