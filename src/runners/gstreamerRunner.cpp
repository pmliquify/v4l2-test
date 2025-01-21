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
     
}

int GstreamerRunner::setup(CommandArgs &args)
{
        BasicStreamRunner::setup(args);
        m_gstreamerSink.setup(args);

        return 0;
}

int GstreamerRunner::processImage(ImageSource *imageSource, Image *image)
{
        if(m_firstRun)
        {
                m_gstreamerSink.init(image->width(), image->height());
                m_firstRun = false;
        }
        auto start = std::chrono::high_resolution_clock::now(); // Start time

        cv::Mat img;
        if (image->pixelformat() == V4L2_PIX_FMT_Y10) {
                ImageData data = m_imageConvertCPU.convert10BitGreyToRGB888(image);
               img = cv::Mat(image->height(), image->width(), CV_8UC3, data.data);
        }
        if (image->pixelformat() == V4L2_PIX_FMT_Y10P) {
                ImageData data = m_imageConvertCPU.convert10BitPackedGreyToRGB888(image);
                img = cv::Mat(image->height(), image->width(), CV_8UC3, data.data);
        }
        if(img.empty())
        {
                return -1;
        }
        BasicStreamRunner::processImage(imageSource, image);
        auto middle = std::chrono::high_resolution_clock::now(); // End time
        std::chrono::duration<double, std::milli> duration = middle - start; // Calculate duration in milliseconds

        std::cout << "Time taken to create the image: " << duration.count() << " ms" << std::endl;
        m_gstreamerSink.pushFrame(img);

        auto end = std::chrono::high_resolution_clock::now(); // End time
        duration = end - middle; // Calculate duration in milliseconds

        std::cout << "Time taken to push the image: " << duration.count() << " ms" << std::endl;


        return 0;
        // cv::Mat img(image->height(), image->width(), CV_16U, (char *)image->planes()[0]);
        // cv::Mat rgb(image->height(), image->width(), CV_16U, (char *)image->data());
        // cv::cvtColor(img, rgb, cv::COLOR_BayerRG2RGB);

}