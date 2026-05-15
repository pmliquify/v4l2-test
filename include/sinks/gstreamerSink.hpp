#include <iostream>
#include <string.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <opencv2/opencv.hpp>
#include <utils/commandargsconsumer.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

class GstreamerSink: CommandArgsConsumer
{
public:
    enum SinkType
    {
        NONE,
        FBDEV,
        UDP,
        UDP_RK,
        FILE,
        KMS,
        AUTOVIDEO
    };

    GstreamerSink();
    ~GstreamerSink();
    void printArgs();
    int setup(CommandArgs &args);
    int init(int width, int height);
    int initUdp(GstElement *link);
    int initUdpRK(GstElement *link);

    int initFbdev(GstElement *link);
    int initMp4(GstElement *link);
    int initKmsSink(GstElement *link);
    int initAutoVideoSink(GstElement *link);

    int pushFrame(cv::Mat &frame);
    static void initGStreamer(); // Add this line
    void close();
    void finishFile();
    std::string m_framebufferDevice;
    SinkType m_sink;

private:
    GstPipeline *m_pipeline;
    GstAppSrc *m_appsrc = nullptr;
    GMainLoop *loop;
    GstBus *bus;
    guint bus_watch_id;

    std::thread             m_pusher;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    std::queue<cv::Mat>     m_frameQueue;
    bool                    m_stop = false;

    void pusherThread();

    static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);

    static void cb_enough_data(GstElement *appsrc, gpointer user_data);
    static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data);

    int m_width, m_height;
    std::string m_udpHost;
    int m_udpPort;
    int m_scaleFactor;

    std::string m_filename;
};