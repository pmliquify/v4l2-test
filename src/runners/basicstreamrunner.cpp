#include <runners/basicstreamrunner.hpp>
#include <cv/imageprint.hpp>
#include <unistd.h>
#include <cstdlib> // for system()

BasicStreamRunner::BasicStreamRunner() :
        m_print(16),
        m_lastTimestamp(0),
        m_fb(false),
#ifdef WITH_GUI
        m_gui(false),
#endif
        m_delay(0),
        m_x(-1),
        m_y(-1),
        m_width(0),
        m_height(0),
        m_singleAcquisition(false),
        m_imageCount(-1),
        m_timeout(1000000)
{
}

BasicStreamRunner::~BasicStreamRunner()
{
        printf("BasicStreamRunner::~BasicStreamRunner()\n");
        fflush(stdout);
        m_running = false;
}

void BasicStreamRunner::printArgs()
{
        printArg("-p",          "Print image data in format (1: bin, 10: dec, 16: hex)");
        printArg("--fb",        "Output the image to the framebuffer");
#ifdef WITH_GUI
        printArg("--gui",       "Output the image to the viewer gui");
#endif
        printArg("-l",          "Set delay in us in image acquisition loop");
        printArg("-h",          "Set image height");
        printArg("-i",          "Start single image acquisition");
        printArg("-n",          "Number of images to captured");
        printArg("-w",          "Set image width");
        printArg("-x",          "Set x pixel position to print");
        printArg("-y",          "Set y pixel position to print");
        printArg("--timeout",   "Set the timeout for image acquisition");
}

int BasicStreamRunner::setup(CommandArgs &args)
{
        m_print                 = args.optionInt("-p", -1);
        m_fb                    = args.exists("--fb");
#ifdef WITH_GUI
        m_gui                   = args.exists("--gui");
#endif
        m_delay                 = args.optionInt("-l",  0);
        m_height                = args.optionInt("-h", 0);
        m_singleAcquisition     = args.exists("-i");
        m_imageCount            = args.optionInt("-n", -1);
        m_width                 = args.optionInt("-w", 0);
        m_x                     = args.optionInt("-x", -1);
        m_y                     = args.optionInt("-y", -1);
        m_timeout               = args.optionInt("--timeout", 1000000);
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
                if (!m_fb) {
                        printf(">");
                        fflush(stdout);
                }
        }

        if (m_fb) {             

                if (m_frameBuffer.show(image) != 0) {                       
                        return -1;
                }


        }

#ifdef WITH_GUI
        if (m_gui) {
                if (m_viewer.show(image) != 0) {
                        return -1;
                }
        }
#endif

        return 0;
}



int BasicStreamRunner::run(ImageSource *imageSource) 
{
        m_running = true;
        if (m_fb) {
                m_frameBuffer.open();
                m_frameBuffer.fill();
        }

#ifdef WITH_GUI
        if (m_gui) {
                m_viewer.init(imageSource);
        }
#endif

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
                int error = 0;
                for (int index = 0; index < count && error == 0; index += step) {
                        Image *image = NULL;
                        int ret = imageSource->getImage(image, m_timeout);
                        if (ret == 0) {
                                error = processImage(imageSource, image);

                        } else if (ret == -2) {
                               printf("Timeout: No image acquired! (%u us)\n", m_timeout);
                        }
                }
        } else {
                if (imageSource->streamOn(3) == 0) {
                        int error = 0;
                        for (int index = 0; index < count && error == 0 && m_running; index += step) {
                                
        
                                Image *image = NULL;
                                int ret = imageSource->getNextImage(image, m_timeout);
                                if (ret == 0) {
                                        error = processImage(imageSource, image);
                                        imageSource->releaseImage(image);

                                } else if (ret == -2) {
                                       printf("Timeout: No image acquired! (%u us)\n", m_timeout);
                                }
                        }
                        imageSource->streamOff();
                }
        }

        if (m_fb) {
                m_frameBuffer.close();

        }

#ifdef WITH_GUI
        if (m_gui) {
                m_viewer.hide();
        }
#endif


        return 0;
}
int BasicStreamRunner::closeRunner()
{
        m_running = false;
        if(m_fb)  m_frameBuffer.close();

        return 0;
}