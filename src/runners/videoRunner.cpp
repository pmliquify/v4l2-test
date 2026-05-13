#include <runners/videoRunner.hpp>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>


VideoRunner::VideoRunner()
{
}

void VideoRunner::printArgs()
{
    printArgSection("Video runner");
    BasicStreamRunner::printArgs();
    m_videoSink.printArgs();
    printArg("--downscale", "Downscale factor [2,3,4...]");
    printArg("--count",     "Number of frames to record");
}

int VideoRunner::setup(CommandArgs &args)
{
    BasicStreamRunner::setup(args);
    m_videoSink.setup(args);

    m_scaleFactor = args.optionInt("--downscale", 1);
    m_maxCount    = args.optionInt("--count", 0);
    m_lastImage   = false; // consume every frame in order

    return 0;
}

int VideoRunner::processImage(ImageSource *imageSource, Image *image)
{
    cv::Mat img, src;
    ImageData data{};

    switch (image->pixelformat())
    {
        case V4L2_PIX_FMT_GREY:
            img = cv::Mat(image->height(), image->width(), CV_8UC1, image->planes()[0]).clone();
            if (m_scaleFactor != 1)
                cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            break;
        case V4L2_PIX_FMT_Y10:
            data = m_imageConvertCPU.convert10BitGreyToRGB888(image, m_scaleFactor);
            img  = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data).clone();
            break;
        case V4L2_PIX_FMT_Y10P:
            data = m_imageConvertCPU.convert10BitPackedGreyToRGB888(image, m_scaleFactor);
            img  = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data).clone();
            break;
        case V4L2_PIX_FMT_SRGGB10P:
        case V4L2_PIX_FMT_SGBRG10P:
            data = m_imageConvertCPU.convert10BitPackedBayerToRGB888(image, m_scaleFactor);
            img  = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data).clone();
            break;
        case V4L2_PIX_FMT_SRGGB10:
            img = cv::Mat(image->height(), image->width(), CV_16UC1, image->planes()[0]);
            img.convertTo(img, CV_8UC1, 1.0/4.0);
            cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            cv::demosaicing(img, img, cv::COLOR_BayerRGGB2RGB);
            break;
        case V4L2_PIX_FMT_SGBRG10:
            img = cv::Mat(image->height(), image->width(), CV_16UC1, image->planes()[0]);
            img.convertTo(img, CV_8UC1, 1.0/4.0);
            cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            cv::demosaicing(img, img, cv::COLOR_BayerGBRG2RGB);
            break;
        case V4L2_PIX_FMT_SRGGB12P:
        case V4L2_PIX_FMT_SGBRG12P:
            data = m_imageConvertCPU.convert12BitPackedBayerToRGB888(image, m_scaleFactor);
            img  = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data).clone();
            break;
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SGBRG12:
            src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
            cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
            if (m_scaleFactor != 1)
                cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            break;
        case V4L2_PIX_FMT_SGBRG8:
            src = cv::Mat(image->height(), image->width(), CV_8UC1, (char *)image->planes()[0]);
            cv::demosaicing(src, img, cv::COLOR_BayerGB2BGR);
            if (m_scaleFactor != 1)
                cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            break;
        case V4L2_PIX_FMT_SRGGB8:
            src = cv::Mat(image->height(), image->width(), CV_8UC1, (char *)image->planes()[0]);
            cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
            if (m_scaleFactor != 1)
                cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            break;
        case V4L2_PIX_FMT_NV12:
            src = cv::Mat(image->height() * 3/2, image->width(), CV_8UC1, (char *)image->planes()[0]);
            cv::cvtColor(src, img, cv::COLOR_YUV2BGR_NV12);
            if (m_scaleFactor != 1)
                cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
            break;
        case V4L2_PIX_FMT_SRGGB16:
            src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
            cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
            break;
        case V4L2_PIX_FMT_SGBRG16:
            src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
            cv::demosaicing(src, img, cv::COLOR_BayerGB2BGR);
            break;
        default:
            std::cerr << "VideoRunner: pixel format not supported!" << std::endl;
            return -1;
    }

    if (img.empty()) {
        std::cerr << "VideoRunner: empty frame, skipping" << std::endl;
        return 0;
    }

    if (m_firstRun) {
        m_firstRun = false;
        bool grayscale = (img.channels() == 1);
        if (m_videoSink.init(img.cols, img.rows, grayscale) != 0)
            return -1;
    }

    BasicStreamRunner::processImage(imageSource, image);

    uint64_t timestamp = image->timestamp();
    m_videoSink.pushFrame(img, timestamp);

    img.release();
    if (data.size)
        free(data.data);

    if (m_maxCount > 0 && (++m_counter == m_maxCount))
        return 1;

    return 0;
}
