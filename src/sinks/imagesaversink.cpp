#include <sinks/imageSaverSink.hpp>

ImageSaverSink::ImageSaverSink()
{
}
ImageSaverSink::~ImageSaverSink()
{    
}
void ImageSaverSink::printArgs()
{
        printArgSection("ImageSaverSink");
        printArg("--prefix", "Prefix for the filename");
        
       
}
int ImageSaverSink::setup(CommandArgs &args)
{
    prefix = args.optionString("--prefix", "Image");


    return 0;


}


int ImageSaverSink::init(int width, int height)
{

    return 0;
   
}
void ImageSaverSink::close()
{
   
}


int ImageSaverSink::pushFrame(cv::Mat &frame, uint64_t timestamp)
{
    //Timestamp in milliseconds to string    
    std::string strTimestamp = std::to_string(timestamp/1000) + "." +  std::to_string(timestamp%1000);

    cv::imwrite(prefix + "_" + strTimestamp + ".png", frame);

    return 0;

    
}

