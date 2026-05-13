#pragma once

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include <utils/commandargsconsumer.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoSink : public CommandArgsConsumer
{
public:
    VideoSink();
    ~VideoSink();

    void printArgs();
    int setup(CommandArgs &args);
    int init(int width, int height, bool grayscale);
    int pushFrame(cv::Mat &frame, uint64_t timestamp);
    void close();

private:
    struct FrameEntry {
        cv::Mat mat;
        uint64_t timestamp;
    };

    void writerThread();
    int encodeAndWrite(cv::Mat &frame);
    void flushEncoder();

    std::string m_output;
    int         m_fps;
    int         m_crf;
    std::string m_codec;
    bool        m_grayscale = false;
    int         m_width     = 0;
    int         m_height    = 0;
    int64_t     m_frameIndex = 0;

    AVFormatContext *m_fmtCtx   = nullptr;
    AVCodecContext  *m_codecCtx = nullptr;
    AVStream        *m_stream   = nullptr;
    AVFrame         *m_frame    = nullptr;
    AVPacket        *m_packet   = nullptr;
    SwsContext      *m_swsCtx   = nullptr;

    std::thread             m_writer;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    std::queue<FrameEntry>  m_queue;
    bool m_stop        = false;
    bool m_initialized = false;
};
