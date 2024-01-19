#include <commandargs.hpp>
#include <v4l2imagesource.hpp>
#include <v4l2imagesocket.hpp>
#include <v4l2autoexposure.hpp>
#include <framebuffer.hpp>
#include <unistd.h>

void printHelp()
{
        printf("  Usage: v4l2-test [OPTION...]\n");
        printf("  Version 0.2.0\n");
        printf("\n");
        printf("  --ae       Activates auto exposure\n");
        printf("  --aeD      Set auto exposure D factor of PD controller\n");
        printf("  --aeP      Set auto exposure P factor of PD controller\n");
        printf("  --aeMin    Set auto exposure minimal mean image brightness\n");
        printf("  --aeMax    Set auto exposure maximal mean image brightness\n");
        printf("  --aeSub    Set auto exposure sub sampling\n");
        printf("  --aeTarget Set auto exposure target mean image brightness\n");
        printf("  --aeTest   Activate auto exposure\n");
        printf("  -bi,       Set binning (see binning modes of each camera module)\n");
        printf("  -bl,       Set black level\n");
        printf("  -d,        Set device /dev/video0\n");
        printf("  -e,        Set exposure time\n");
        printf("  -f,        Set pixel format\n");
        printf("  --fb,      Output the image to the framebuffer\n");
        printf("  -g,        Set gain value\n");
        printf("  -h,        Set image height\n");
        printf("  --help,    Show this help\n");
        printf("  -i,        Start single image acquisition\n");
        printf("  -l,        Set delay in us in image acquisition loop\n");
        printf("  -m,        Set IO mode\n");
        printf("  -n,        Number of images to captured\n");
        printf("  -p,        Print image data in format (1: bin, 10: dec, 16: hex)\n");
        printf("  --port,    \n");
        printf("  -r,        Set frame rate\n");
        printf("  -s,        Start image streaming\n");
        printf("  -sd,       Set subdevice /dev/v4l-subdev0\n");
        printf("  --shift,   Set bit shift for each pixel in RAW10, RAW12 format\n");
        printf("  -t,        Set trigger mode\n");
        printf("  --udp,     \n");
        printf("  -w,        Set image width\n");
        printf("  -x,        Set x pixel position to print\n");
        printf("  -y,        Set y pixel position to print\n");
}

int processImage(CommandArgs &args, V4L2ImageSource &imageSource, V4L2Image &image, 
                 FrameBuffer &frameBuffer, bool fb, V4L2AutoExposure &autoExposure, bool ae)
{
        int x               = args.optionInt("-x", -1);
        int y               = args.optionInt("-y", -1);
        int width           = args.optionInt("-w", 0);
        int height          = args.optionInt("-h", 0);
        if (width > 0 || height > 0) {
                imageSource.setSelection((x==-1)?0:x, (y==-1)?0:y, width, height);
        }

        int delay           = args.optionInt("-l",  0);
        if (delay > 0) {
                usleep(delay);
        }
        int print           = args.optionInt("-p", 16);
        int shift           = args.optionInt("--shift", 0);

        if (ae) {
                autoExposure.exec(image);
        } else {
                image.print(print, x, y, 10, shift);
        }
        if (fb) frameBuffer.print(image, shift);
        
        return 0;
}

int main(int argc, const char *argv[])
{
        CommandArgs args(argc, argv);

        if (args.exists("--help")) {
                printHelp();
                return 0;
        }

        V4L2ImageSource imageSource;  
        imageSource.open(args.option("-d", "/dev/video0"), args.option("-sd", ""));
        imageSource.printFormat();
        
        if (args.exists("-e")) {
                imageSource.setExposure(args.optionInt("-e"));
        }
        if (args.exists("-g")) {
                imageSource.setGain(args.optionInt("-g"));
        }
        if (args.exists("-bi")) {
                imageSource.setBinning(args.optionInt("-bi"));
        }
        if (args.exists("-bl")) {
                imageSource.setBlackLevel(args.optionInt("-bl"));
        }
        if (args.exists("-t")) {
                imageSource.setTriggerMode(args.optionInt("-t"));
        }
        if (args.exists("-m")) {
                imageSource.setIOMode(args.optionInt("-m"));
        }
        if (args.exists("-r")) {
                imageSource.setFrameRate(args.optionInt("-r"));
        }
        if (args.exists("-f")) {
                std::string format = args.option("-f");
                if (format.size() == 4) {
                        imageSource.setFormat(*(int *)format.c_str());
                }
        }

        int imageCount      = args.optionInt("-n", -1);
        
        V4L2Image image;
        image.m_shift       = args.optionInt("--shift", 0);

        V4L2AutoExposure autoExposure(&imageSource);
        bool ae             = args.exists("--ae");
        if (ae) {
                autoExposure.m_aed  = args.optionInt("--aeD", 0);
                autoExposure.m_aep  = args.optionInt("--aeP", 1);
                autoExposure.m_sub  = args.optionInt("--aeSub", 1);
                autoExposure.m_test = args.optionInt("--aeTest", 0);
                autoExposure.m_aeTarget = args.optionInt("--aeTarget", 100);
                autoExposure.m_exposureMin = args.optionInt("--aeMin", 0);
                autoExposure.m_exposureMax = args.optionInt("--aeMax", 10000);
        }
        
        FrameBuffer frameBuffer;
        bool fb              = args.exists("--fb");

        if (args.exists("-i")) {
                if (fb) {
                        frameBuffer.open();
                        frameBuffer.fill();
                }

                for (int index = 0; index < imageCount; index++) {
                        imageSource.getImage(image, 1000000);
                        processImage(args, imageSource, image, frameBuffer, fb, autoExposure, ae);
                }
        }
        if (args.exists("-s")) {
                if (fb) {
                        frameBuffer.open();
                        frameBuffer.fill();
                }

                if (0 == imageSource.streamOn(3)) {
                        if (-1 == imageCount) {
                                while(true) {
                                        if (0 == imageSource.getNextImage(image, 1000000)) {
                                                processImage(args, imageSource, image, frameBuffer, fb, autoExposure, ae);
                                                imageSource.releaseImage(image);
                                        }
                                }
                        } else {
                                for (int index = 0; index < imageCount; index++) {
                                        if (0 == imageSource.getNextImage(image, 1000000)) {
                                                processImage(args, imageSource, image, frameBuffer, fb, autoExposure, ae);
                                                imageSource.releaseImage(image);
                                        }
                                }
                        }
                        imageSource.streamOff();
                }
        }

        if (fb) frameBuffer.close();
        imageSource.close();

        return 0;
}