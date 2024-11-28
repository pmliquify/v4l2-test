#include "convert.hpp"
#include <linux/videodev2.h>


Mat convert(Image *image)
{
        int type = CV_8UC1;
        bool debayer = false;
        int divider = 1;
        int code = 0;

        switch (image->pixelformat()) {
        case V4L2_PIX_FMT_GREY:    type =  CV_8UC1; debayer = false; divider =    1; break;
        case V4L2_PIX_FMT_SRGGB8:  type =  CV_8UC1; debayer = true;  divider =    1; code =  COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_SGBRG8:  type =  CV_8UC1; debayer = true;  divider =    1; code =  COLOR_BayerGB2RGB; break;
        case V4L2_PIX_FMT_SGRBG8:  type =  CV_8UC1; debayer = true;  divider =    1; code =  COLOR_BayerGR2RGB; break;
        case V4L2_PIX_FMT_SBGGR8:  type =  CV_8UC1; debayer = true;  divider =    1; code =  COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_Y10:     type = CV_16UC1; debayer = false; divider = 1023; break;
        case V4L2_PIX_FMT_SRGGB10: type = CV_16UC1; debayer = true;  divider = 1023; code =  COLOR_BayerRG2RGB; break;
        case V4L2_PIX_FMT_SGBRG10: type = CV_16UC1; debayer = true;  divider = 1023; code =  COLOR_BayerGB2RGB; break;
        case V4L2_PIX_FMT_SGRBG10: type = CV_16UC1; debayer = true;  divider = 1023; code =  COLOR_BayerGR2RGB; break;
        case V4L2_PIX_FMT_SBGGR10: type = CV_16UC1; debayer = true;  divider = 1023; code =  COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_Y12:     type = CV_16UC1; debayer = false; divider = 4095; break;
        case V4L2_PIX_FMT_SRGGB12: type = CV_16UC1; debayer = true;  divider = 4095; code =  COLOR_BayerRG2RGB; break;
        case V4L2_PIX_FMT_SGBRG12: type = CV_16UC1; debayer = true;  divider = 4095; code =  COLOR_BayerGB2RGB; break;
        case V4L2_PIX_FMT_SGRBG12: type = CV_16UC1; debayer = true;  divider = 4095; code =  COLOR_BayerGR2RGB; break;
        case V4L2_PIX_FMT_SBGGR12: type = CV_16UC1; debayer = true;  divider = 4095; code =  COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_YUYV:    type =  CV_8UC2; debayer = true;  divider =    1; code = COLOR_YUV2BGR_YUY2; break;
        case V4L2_PIX_FMT_SRGGB10P: type = CV_16UC1; debayer = true;  divider = 1023; code =  COLOR_BayerRG2RGB; break;
        }

        //Print the pixel format as string
        std::string pixelformat;
        switch (image->pixelformat()) {
        case V4L2_PIX_FMT_GREY:    pixelformat = "GREY"; break;
        case V4L2_PIX_FMT_SRGGB8:  pixelformat = "SRGGB8"; break;
        case V4L2_PIX_FMT_SGBRG8:  pixelformat = "SGBRG8"; break;
        case V4L2_PIX_FMT_SGRBG8:  pixelformat = "SGRBG8"; break;
        case V4L2_PIX_FMT_SBGGR8:  pixelformat = "SBGGR8"; break;
        case V4L2_PIX_FMT_Y10:     pixelformat = "Y10"; break;
        case V4L2_PIX_FMT_SRGGB10: pixelformat = "SRGGB10"; break;
        case V4L2_PIX_FMT_SGBRG10: pixelformat = "SGBRG10"; break;
        case V4L2_PIX_FMT_SGRBG10: pixelformat = "SGRBG10"; break;
        case V4L2_PIX_FMT_SBGGR10: pixelformat = "SBGGR10"; break;
        case V4L2_PIX_FMT_Y12:     pixelformat = "Y12"; break;
        case V4L2_PIX_FMT_SRGGB12: pixelformat = "SRGGB12"; break;
        case V4L2_PIX_FMT_SGBRG12: pixelformat = "SGBRG12"; break;
        case V4L2_PIX_FMT_SGRBG12: pixelformat = "SGRBG12"; break;
        case V4L2_PIX_FMT_SBGGR12: pixelformat = "SBGGR12"; break;
        case V4L2_PIX_FMT_YUYV:    pixelformat = "YUYV"; break;
        case V4L2_PIX_FMT_SRGGB10P: pixelformat = "SRGGB10P"; break;
        case V4L2_PIX_FMT_SBGGR10P: pixelformat = "SBGGR10P"; break;
        case V4L2_PIX_FMT_SGBRG10P: pixelformat = "SGBRG10P"; break;
        default: pixelformat = "Unknown"; break;
        }

        if(image->pixelformat() == V4L2_PIX_FMT_SRGGB10P || image->pixelformat() == V4L2_PIX_FMT_SBGGR10P || image->pixelformat() == V4L2_PIX_FMT_SGBRG10P)
        {
            return convert_bayer10p_to_rgb((uint8_t *)image->planes()[0], image->width(), image->height(),0);
        }

        printf("pixelformat: %s (%d)\n", pixelformat.c_str(), image->pixelformat());

        printf("convert: %d %d %d %d\n", image->width(), image->height(), image->bytesPerLine(), image->imageSize());

        Mat imageRAW8(image->height(), image->width(), type, Scalar(200, 0, 0));
        int imageSize = image->bytesPerLine() * image->height();
        if (image->imageSize() < imageSize) {
                imageSize = image->imageSize();
        }
        memcpy(imageRAW8.data, (char *)image->planes()[0], imageSize);

        if (divider > 1) {
                imageRAW8.convertTo(imageRAW8, CV_8UC1, 255.0/divider/(1 << image->shift()));
        }

        Mat imageResult;
        if (debayer) {
                Mat imageBGR(image->height(), image->width(), CV_8UC3);
                cvtColor(imageRAW8, imageBGR, code);
                imageResult = imageBGR;

        } else {
                imageResult = imageRAW8;
        }

        return imageResult;
}

cv::Mat convert_bayer10p_to_rgb(const uint8_t *data, int width, int height, int blackcols)
{
    // Create a 16-bit Mat from the 10-bit data
    cv::Mat rgb;
    cv::Mat raw(height, width, CV_8UC1);

    auto pointer = data;


    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; j += 4)
        {
            const uint8_t *local_pointer = pointer + ((i * width + j) * 5) / 4 + i * blackcols;

            raw.at<uint8_t>(i, j + 0) = static_cast<uint8_t>(*local_pointer);
            raw.at<uint8_t>(i, j + 1) = static_cast<uint8_t>(*(local_pointer + 1));
            raw.at<uint8_t>(i, j + 2) = static_cast<uint8_t>(*(local_pointer + 2));
            raw.at<uint8_t>(i, j + 3) = static_cast<uint8_t>(*(local_pointer + 3));

        }
    }
    cv::cvtColor(raw, rgb, cv::COLOR_BayerBG2BGR);

    // Convert the Bayer image to RGB

    return rgb;
}