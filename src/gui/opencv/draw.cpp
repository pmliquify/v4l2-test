#include "draw.hpp"


void drawImageInfo(Mat img, Image *image, int x, int y)
{
        char info[200];
        sprintf(info, "[#%04d, ts:%8ld ms, %u, %u, %u, %c%c%c%c]", 
                image->sequence(), image->timestamp(), 
                image->width(), image->height(), image->bytesPerLine(),
                (image->pixelformat() >> 0 & 0xFF), (image->pixelformat() >> 8 & 0xFF), 
                (image->pixelformat() >> 16 & 0xFF), (image->pixelformat() >> 24 & 0xFF));

        Text infoText(x, y);
        infoText.setText(info);
        infoText.draw(img);

        int imageSize = image->bytesPerLine() * image->height();
        if (imageSize != image->imageSize()) {
                char error[100];
                char format[] = "ERROR: Image data size is %s as expected (%u %s %u bytes)";
                if (image->imageSize() < imageSize) {
                        sprintf(error, format, "smaler", image->imageSize(), "<", imageSize);

                } if (image->imageSize() > imageSize) {
                        sprintf(error, format, "bigger", image->imageSize(), ">", imageSize);
                }
                Text errorText(x, y + 1);
                errorText.setText(error);
                errorText.draw(img, Scalar(0, 0, 255));
        }
}

