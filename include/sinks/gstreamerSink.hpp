#include <iostream>
#include <string.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <opencv2/opencv.hpp>
#include <utils/commandargsconsumer.hpp>

class GstreamerSink: CommandArgsConsumer
{
public:
    enum SinkType
    {
        FBDEV,
        UDP,
        FILE,
        KMS,
        NONE
    };

    GstreamerSink();
    ~GstreamerSink();
    void printArgs();
    int setup(CommandArgs &args);
    int init(int width, int height);
    int initUdp(GstElement *link);
    int initFbdev(GstElement *link);
    int initMp4(GstElement *link);
    int initKmsSink(GstElement *link);

    int pushFrame(cv::Mat &frame);
    static void initGStreamer(); // Add this line
    void close();
    void finishFile();
    std::string m_framebufferDevice;
    SinkType m_sink;

private:
    GstPipeline *m_pipeline;
    GstAppSrc *m_appsrc;
    GstBuffer *buffer;
    GMainLoop *loop;
    GstBus *bus;
    guint bus_watch_id;


    static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);

    static void cb_enough_data(GstElement *appsrc, gpointer user_data);
    static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data);
    cv::Mat bufferImage;

    int m_width, m_height;
    std::string m_udpHost;
    int m_udpPort;

    std::string m_filename;
};