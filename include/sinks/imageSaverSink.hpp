#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
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
    void writerThread();
    void openNextSegment();

    std::string prefix;
    bool m_saveRaw = false;
    int m_framesPerFile = 0;   // 0 = unlimited

    // segmented file state
    std::ofstream m_rawFile;
    int m_segmentIndex = 0;
    int m_framesInSegment = 0;

    // async writer
    std::thread m_writer;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<cv::Mat> m_queue;
    bool m_stop = false;
};