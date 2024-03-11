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
        case V4L2_PIX_FMT_SRGGB8:  type =  CV_8UC1; debayer = true;  divider =    1; code =  COLOR_BayerRG2RGB; break;
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
        }

        Mat imageRAW(image->height(), image->width(), type, (char *)image->planes()[0]);
        Mat imageRAW8 = imageRAW.clone();

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