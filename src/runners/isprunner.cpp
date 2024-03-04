#include <runners/isprunner.hpp>
#include <linux/videodev2.h>
// #include <opencv2/opencv.hpp>


IspRunner::IspRunner()
{

}

void IspRunner::printArgs()
{
        printArgSection("ISP runner");
        BasicStreamRunner::printArgs();
        
        printArgSection("Auto Exposure");
        printArg("--ae",        "Activates auto exposure");
        printArg("--aeD",       "Set auto exposure D factor of PD controller");
        printArg("--aeP",       "Set auto exposure P factor of PD controller");
        printArg("--aeMin",     "Set auto exposure minimal mean image brightness");
        printArg("--aeMax",     "Set auto exposure maximal mean image brightness");
        printArg("--aeSub",     "Set auto exposure sub sampling");
        printArg("--aeTarget",  "Set auto exposure target mean image brightness");
        printArg("--aeRunner",    "Activate auto exposure");
}

int IspRunner::setup(CommandArgs &args)
{
        BasicStreamRunner::setup(args);
        m_autoExposure.setActive(args.exists("--ae"));
        m_autoExposure.setD(args.optionInt("--aeD", 0));
        m_autoExposure.setP(args.optionInt("--aeP", 10));
        m_autoExposure.setSub(args.optionInt("--aeSub", 32));
        m_autoExposure.setTest(args.optionInt("--aeTest", 0));
        m_autoExposure.setTarget(args.optionInt("--aeTarget", 200));
        m_autoExposure.setExposureMin(args.optionInt("--aeMin", 0));
        m_autoExposure.setExposureMax(args.optionInt("--aeMax", 10000));
        return 0;
}

int IspRunner::processImage(ImageSource *imageSource, Image *image)
{
        m_autoExposure.run(imageSource, image);

        if (image->pixelformat() != V4L2_PIX_FMT_SRGGB10) {
                return -1;
        }


        // cv::Mat img(image->height(), image->width(), CV_16U, (char *)image->planes()[0]);
        // cv::Mat rgb(image->height(), image->width(), CV_16U, (char *)image->data());
        // cv::cvtColor(img, rgb, cv::COLOR_BayerRG2RGB);

        return BasicStreamRunner::processImage(imageSource, image);
}