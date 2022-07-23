#include <v4l2imagesource.hpp>
#include "commandargs.hpp"
#include "framebuffer.hpp"
#include <unistd.h>

void printHelp()
{
        printf("  Usage: v4l2-test [OPTION...]\n");
        printf("\n");
        printf("  -b,      Set black level\n");
        printf("  -d,      Set device /dev/video0\n");
        printf("  -e,      Set exposure time\n");
        printf("  -f,      Set pixel format\n");
        printf("  -g,      Set gain value\n");
        printf("  -h,      Set image height\n");
        printf("  --help,  Show this help\n");
        printf("  -i,      Start single image acquisition\n");
        printf("  -l,      Set delay in us in image acquisition loop\n");
        printf("  -m,      Set IO mode\n");
        printf("  -n,      Number of images to captured\n");
        printf("  -p,      print image data in format (1: bin, 10: dec, 16: hex)\n");
        printf("  -r,      Set frame rate\n");
        printf("  -s,      Start image streaming\n");
        printf("  -t,      Set trigger mode\n");
        printf("  -w,      Set image width\n");
        printf("  -x,      Set x pixel position to print\n");
        printf("  -y,      Set y pixel position to print\n");
        printf("  --shift, Set bit shift for each pixel in RAW10, RAW12 format\n");
}

int main(int argc, char *argv[])
{
        CommandArgs args(argc, argv);

        if (args.exists("--help")) {
                printHelp();
                return 0;
        }

        V4L2ImageSource imageSource;  
        imageSource.open(args.option("-d", "/dev/video0"));
        imageSource.printFormat();
        
        if (args.exists("-e")) {
                imageSource.setExposure(args.optionInt("-e"));
        }
        if (args.exists("-g")) {
                imageSource.setGain(args.optionInt("-g"));
        }
        if (args.exists("-b")) {
                imageSource.setBlackLevel(args.optionInt("-b"));
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

        int x =          args.optionInt("-x", -1);
        int y =          args.optionInt("-y", -1);
        int width =      args.optionInt("-w", 0);
        int height =     args.optionInt("-h", 0);
        if (width > 0 || height > 0) {
                imageSource.setSelection((x==-1)?0:x, (y==-1)?0:y, width, height);
        }

        int imageCount = args.optionInt("-n", -1);
        int delay =      args.optionInt("-l",  0);
        int print =      args.optionInt("-p", 16);
        int shift =      args.optionInt("--shift", 0);

        V4L2Image image;
        FrameBuffer frameBuffer;

        if (args.exists("-i")) {
                frameBuffer.open();
                frameBuffer.fill();

                for (int index = 0; index < imageCount; index++) {
                        imageSource.getImage(image, 1000000);
                        image.print(print, x, y);
                        frameBuffer.print(image, shift);
                        usleep(delay);
                }
        }
        if (args.exists("-s")) {
                frameBuffer.open();
                frameBuffer.fill();

                if (0 == imageSource.streamOn(3)) {
                        if (-1 == imageCount) {
                                while(true) {
                                        if (0 == imageSource.getNextImage(image, 1000000)) {
                                                image.print(print, x, y);
                                                frameBuffer.print(image, shift);
                                                imageSource.releaseImage(image);
                                        }
                                        usleep(delay);
                                }
                        } else {
                                for (int index = 0; index < imageCount; index++) {
                                        if (0 == imageSource.getNextImage(image, 1000000)) {
                                                image.print(print, x, y);
                                                frameBuffer.print(image, shift);
                                                imageSource.releaseImage(image);
                                        }
                                        usleep(delay);
                                }
                        }
                        imageSource.streamOff();
                }
        }

        frameBuffer.close();
        imageSource.close();

        return 0;
}