#include <runners/gstreamerRunner.hpp>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <chrono>


GstreamerRunner::GstreamerRunner()
{

}

void GstreamerRunner::printArgs()
{
        printArgSection("Gstreamer runner");
        BasicStreamRunner::printArgs();
        m_gstreamerSink.printArgs();
        printArg("--downscale", "Cropping factor for downscale [2,3,4...]");

     
}

int GstreamerRunner::setup(CommandArgs &args)
{
        BasicStreamRunner::setup(args);
        m_gstreamerSink.setup(args);

        m_scaleFactor = args.optionInt("--downscale", 1);


        return 0;
}

int GstreamerRunner::processImage(ImageSource *imageSource, Image *image)
{
        if(m_firstRun)
        {
                m_gstreamerSink.init(image->width()/m_scaleFactor, image->height()/m_scaleFactor);
                m_firstRun = false;
        }
        auto start = std::chrono::high_resolution_clock::now(); // Start time

        cv::Mat img;
        ImageData data ;
        switch (image->pixelformat())
        {
                case V4L2_PIX_FMT_GREY:
                        img = cv::Mat(image->height(), image->width(), CV_8UC1, image->planes()[0]);
                        cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
                        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
                        break;
                case V4L2_PIX_FMT_Y10:
                        data = m_imageConvertCPU.convert10BitGreyToRGB888(image, m_scaleFactor);
                        img = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data);
                        break;
                case V4L2_PIX_FMT_Y10P:
                        data = m_imageConvertCPU.convert10BitPackedGreyToRGB888(image, m_scaleFactor);
                        img = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data);
                        break;
                case V4L2_PIX_FMT_SRGGB10P:
                case V4L2_PIX_FMT_SGBRG10P:
                        data = m_imageConvertCPU.convert10BitPackedBayerToRGB888(image, m_scaleFactor);
                        img = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data);
                        break;
                case V4L2_PIX_FMT_SGBRG8:
                case V4L2_PIX_FMT_SRGGB8:
                        data = m_imageConvertCPU.convert8BitBayerToRGB888(image, m_scaleFactor);
                        img = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data);
                        break;
                case V4L2_PIX_FMT_SRGGB12P:
                case V4L2_PIX_FMT_SGBRG12P:
                        data = m_imageConvertCPU.convert12BitPackedBayerToRGB888(image, 1);
                        img = cv::Mat(image->height()/1, image->width()/1, CV_16UC1, data.data);
                        img.convertTo(img, CV_8U, 1.0 / 256.0);
                        break;
                case V4L2_PIX_FMT_NV12 :
                        img = cv::Mat(image->height() * 3/2, image->width(), CV_8UC1, (char *)image->planes()[0]);

                        cv::cvtColor(img, img, cv::COLOR_YUV2BGR_NV12);
                        break;
                default:
                        std::cerr << "Format not supported" << std::endl;
                        break;
        }
        
        if(img.empty())
        {
                return -1;
        }
        BasicStreamRunner::processImage(imageSource, image);
        auto middle = std::chrono::high_resolution_clock::now(); // End time
        std::chrono::duration<double, std::milli> duration = middle - start; // Calculate duration in milliseconds

        // std::cout << "Time taken to create the image: " << duration.count() << " ms" << std::endl;
        m_gstreamerSink.pushFrame(img);

        // auto end = std::chrono::high_resolution_clock::now(); // End time
        // duration = end - middle; // Calculate duration in milliseconds

        //std::cout << "Time taken to push the image: " << duration.count() << " ms" << std::endl;

        img.release();
        if(data.size)
                free(data.data);

        return 0;
        // cv::Mat img(image->height(), image->width(), CV_16U, (char *)image->planes()[0]);
        // cv::Mat rgb(image->height(), image->width(), CV_16U, (char *)image->data());
        // cv::cvtColor(img, rgb, cv::COLOR_BayerRG2RGB);

}