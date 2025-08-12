#include <runners/imagesaverrunner.hpp>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <chrono>


ImageSaverRunner::ImageSaverRunner()
{

}

void ImageSaverRunner::printArgs()
{
        printArgSection("Gstreamer runner");
        BasicStreamRunner::printArgs();
        imageSaverSink.printArgs();
        printArg("--downscale", "Cropping factor for downscale [2,3,4...]");
        printArg("--count", "Images to be recorded");

     
}

int ImageSaverRunner::setup(CommandArgs &args)
{
        BasicStreamRunner::setup(args);
        imageSaverSink.setup(args);

        m_scaleFactor = args.optionInt("--downscale", 1);
        m_maxCount = args.optionInt("--count", 0);

        return 0;
}

int ImageSaverRunner::processImage(ImageSource *imageSource, Image *image)
{
        if(m_firstRun)
        {
                m_firstRun = false;
        }
        auto start = std::chrono::high_resolution_clock::now(); // Start time

        cv::Mat img, src;
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
                        src = cv::Mat(image->height(), image->width(), CV_8UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
                case V4L2_PIX_FMT_SRGGB8:
                        src = cv::Mat(image->height(), image->width(), CV_8UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
                        break;
                case V4L2_PIX_FMT_SRGGB10:
                        src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);  
                        break;      
                case V4L2_PIX_FMT_SBGGR10:
                        src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerBG2BGR);  
                        break;       
                case V4L2_PIX_FMT_SRGGB12:
                case V4L2_PIX_FMT_SGBRG12:
                        src = cv::Mat(image->height() , image->width(), CV_16UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
                        break;
                case V4L2_PIX_FMT_SRGGB12P:
                case V4L2_PIX_FMT_SGBRG12P:
                        data = m_imageConvertCPU.convert12BitPackedBayerToRGB888(image, m_scaleFactor);
                        img = cv::Mat(image->height()/m_scaleFactor, image->width()/m_scaleFactor, CV_8UC3, data.data);
                        break;
                case V4L2_PIX_FMT_NV12 :
                        src = cv::Mat(image->height() * 3/2, image->width(), CV_8UC1, (char *)image->planes()[0]);

                        cv::cvtColor(src, img, cv::COLOR_YUV2BGR_NV12);
                        break;
                //16 bit formats
                case V4L2_PIX_FMT_Y16:
                        src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
                        break;
                case V4L2_PIX_FMT_SRGGB16:
                        src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerRG2BGR);
                        break;
                case V4L2_PIX_FMT_SGBRG16:
                        src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerGB2BGR);       
                        break;
                case V4L2_PIX_FMT_SGRBG16:
                        src = cv::Mat(image->height(), image->width(), CV_16UC1, (char *)image->planes()[0]);
                        cv::demosaicing(src, img, cv::COLOR_BayerGR2BGR);               
                        break;
                default:
                        std::cerr << "Format not supported!" << std::endl;
                        break;
        }
        
        if(img.empty())
        {
                return -1;
        }
        if (m_scaleFactor != 1)
        {
                cv::resize(img, img, cv::Size(image->width()/m_scaleFactor, image->height()/m_scaleFactor));
        }
        BasicStreamRunner::processImage(imageSource, image);
        auto middle = std::chrono::high_resolution_clock::now(); // End time
        std::chrono::duration<double, std::milli> duration = middle - start; // Calculate duration in milliseconds

        uint64_t timestamp = image->timestamp();
        //Display the timestamp in seconds and milliseconds
        // std::cout << timestamp/1000 << "." << timestamp%1000 << " Time taken to convert the image: " << duration.count() << " ms" << std::endl;


        auto end = std::chrono::high_resolution_clock::now(); // End time
        duration =  end - middle; // Calculate duration in milliseconds

        imageSaverSink.pushFrame(img, timestamp);
        // std::cout << timestamp/1000 << "." << timestamp%1000 << " Time to encode image and store it: " << duration.count() << " ms" << std::endl;
        

        

        img.release();
        if(data.size)
                free(data.data);        
        if(m_maxCount > 0  && (++counter == m_maxCount))
        {
                std::exit(0);
        }
        return 0;
     

}