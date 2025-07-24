#include <iostream>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <utils/commandargsconsumer.hpp>

class ImageSaverSink: CommandArgsConsumer
{
public:
   

    ImageSaverSink();
    ~ImageSaverSink();
    void printArgs();
    int setup(CommandArgs &args);
    int init(int width, int height);
    
    int pushFrame(cv::Mat &frame, uint64_t timestamp);
    void close();
    void finishFile();

private:
   

    std::string prefix;
    

    int m_width, m_height;
    std::string m_udpHost;
    int m_udpPort;
    int m_scaleFactor;

    std::string m_filename;
};