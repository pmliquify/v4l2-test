#include <sinks/gstreamerSink.hpp>

GstreamerSink::GstreamerSink()
{
    initGStreamer();
}
GstreamerSink::~GstreamerSink()
{
    gst_element_set_state(GST_ELEMENT(m_appsrc), GST_STATE_NULL);
    gst_object_unref(GST_ELEMENT(m_appsrc));
}
void GstreamerSink::printArgs()
{
        printArgSection("GstreamerSink");
        printArg("--gst", "Gstreamer sink [file,udp,framebuffer]");
        printArg("--udphost", "Host ip for the udp sink");
        printArg("--udpport", "Port for the udp sink");
        printArg("--filename", "Filename for the file sink");
        printArg("--fbdev", "Framebuffer device for the framebuffer sink");
        
       
}
int GstreamerSink::setup(CommandArgs &args)
{
    std::string optionGstreamerSink = args.optionString("--gst", "");

    if (optionGstreamerSink == "udp")
    {
        m_sink = SinkType::UDP;
        m_udpHost = args.optionString("--udphost", "");
        if(m_udpHost.empty())
        {
            std::cerr << "UDP host not provided" << std::endl;
            return -1;
        }
        if(!args.exists("--udpport"))
        {
            std::cout << "UDP port not provided, using default port 5000" << std::endl;
        }
        m_udpPort = args.optionInt("--udpport", 5000);
    }
    else if (optionGstreamerSink == "file")
    {
        m_sink = SinkType::FILE;
        m_filename = args.optionString("--filename", "test.mp4");
    }
    else if (optionGstreamerSink == "framebuffer")
    {
        m_sink = SinkType::FBDEV;
        m_framebufferDevice = args.optionString("--fbdev", "/dev/fb0");
    }
    else if (optionGstreamerSink == "kms")
    {
        m_sink = SinkType::KMS;
    }
    else
    {
        m_sink = SinkType::NONE;
    }

    return 0;


}

int GstreamerSink::initUdp(GstElement *link)
{
    GstElement *x264enc = gst_element_factory_make("x264enc", "vc_x264enc");
    g_object_set(G_OBJECT(x264enc), 
                "bitrate", 4000,          // Increased bitrate for better quality
                "speed-preset", 6,        // medium preset (0=placebo, 6=medium, 9=veryfast)
                "tune", 0,               // zerolatency tuning for streaming
                "key-int-max", 60,      // Keyframe every 2 seconds at 30fps
                NULL);

    // Add queue elements to prevent pipeline stalls
    GstElement *encqueue = gst_element_factory_make("queue", "enc_queue");
    GstElement *payqueue = gst_element_factory_make("queue", "pay_queue");
    
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "vc_rtph264pay");
    g_object_set(G_OBJECT(rtph264pay),
                "config-interval", 1,     // Send codec config with every keyframe
                "pt", 96,                 // Standard payload type for H.264y
                NULL);

    GstElement *udpsink = gst_element_factory_make("udpsink", "vc_sink");
    g_object_set(G_OBJECT(udpsink), 
                "host", m_udpHost.c_str(),
                "port", m_udpPort,
                "buffer-size", 41943040,  // Increase buffer size to 40MB
                "max-lateness", 500000000,// Allow larger timing variations (500ms)
                "qos", TRUE,             // Enable QoS events
                NULL);

    gst_bin_add_many(GST_BIN(m_pipeline), encqueue, x264enc, payqueue, rtph264pay, udpsink, NULL);

    // Link elements with queues
    gst_element_link(link, encqueue);
    gst_element_link(encqueue, x264enc);
    gst_element_link(x264enc, payqueue);
    gst_element_link(payqueue, rtph264pay);
    gst_element_link(rtph264pay, udpsink);

    return 0;
}
int GstreamerSink::initMp4(GstElement *link)
{
   
    GstElement *x264enc = gst_element_factory_make("x264enc", "vc_x264enc");
    g_object_set(G_OBJECT(x264enc), "bitrate", 10000 , NULL);
    g_object_set(G_OBJECT(x264enc), "speed-preset", 3, NULL);
        g_object_set(G_OBJECT(x264enc), "tune", 4, NULL);
 //     "! mp4mux ! "
    //     "filesink location=test.mp4";
    GstElement *mp4mux = gst_element_factory_make("mp4mux", "vc_rtph264pay");

    GstElement *filesink = gst_element_factory_make("filesink", "vc_sink");
    g_object_set(G_OBJECT(filesink), "location", m_filename.c_str(), NULL);

    gst_bin_add_many(GST_BIN(m_pipeline), x264enc, mp4mux ,filesink, NULL);

    gst_element_link(link, x264enc);
    gst_element_link(x264enc, mp4mux);
    gst_element_link(mp4mux, filesink);



    return 0;
}

int GstreamerSink::initKmsSink(GstElement *link)
{
     GstElement *videoscale = gst_element_factory_make("videoscale", "vc_scale");

    GstElement *kmssink = gst_element_factory_make("kmssink", "vc_sink");
    if (!videoscale || !kmssink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        return -1;
    }
    
    GstCaps *capsScale2fbdevsink; 
    capsScale2fbdevsink = gst_caps_new_simple("video/x-raw", "format",
                                              G_TYPE_STRING, "RGB16", "width", G_TYPE_INT, m_width, "height",
                                              G_TYPE_INT, m_height, "framerate", GST_TYPE_FRACTION, 0, 1, NULL);


    gst_bin_add_many(GST_BIN(m_pipeline), videoscale, kmssink, NULL);


    gst_element_link(link, videoscale);
    gst_element_link_filtered(videoscale, kmssink, capsScale2fbdevsink);


    return 0;
}

int GstreamerSink::initFbdev(GstElement *link)
{
     GstElement *videoscale = gst_element_factory_make("videoscale", "vc_scale");

    GstElement *fbdevsink = gst_element_factory_make("fbdevsink", "vc_sink");
    g_object_set(G_OBJECT (fbdevsink), "device", m_framebufferDevice.c_str(), NULL);
    g_object_set(G_OBJECT(fbdevsink), "sync", FALSE, NULL); // Add this line
    if (!videoscale || !fbdevsink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        return -1;
    }
    
    GstCaps *capsScale2fbdevsink; 
    capsScale2fbdevsink = gst_caps_new_simple("video/x-raw", "format",
                                              G_TYPE_STRING, "RGB16", "width", G_TYPE_INT, m_width, "height",
                                              G_TYPE_INT, m_height, "framerate", GST_TYPE_FRACTION, 0, 1, NULL);


    gst_bin_add_many(GST_BIN(m_pipeline), videoscale, fbdevsink, NULL);


    gst_element_link(link, videoscale);
    gst_element_link_filtered(videoscale, fbdevsink, capsScale2fbdevsink);


    return 0;
}
int GstreamerSink::init(int width, int height)
{
    this->m_width = width;
    this->m_height = height;
    std::cout << "GstreamerSink init" << std::endl;
    GError *error = nullptr;
    m_pipeline = GST_PIPELINE(gst_pipeline_new("vc_pipeline"));
    m_appsrc = GST_APP_SRC(gst_element_factory_make("appsrc", "vc_source"));
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "vc_convert");
   
    if (!m_pipeline || !m_appsrc)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
    }
   

    // Set appsrc properties
    g_object_set(G_OBJECT(m_appsrc),
                 "stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
                 "format", GST_FORMAT_TIME, // timestamps in nano seconds
                 "is-live", TRUE,
                 "block", TRUE,
                 "max-bytes", 100000000,          // Set max-bytes to 10MB
                 "blocksize", width * height * 3, // Set blocksize to image size
                 nullptr);

    loop = g_main_loop_new(NULL, FALSE);

    bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    bus_watch_id = gst_bus_add_watch(bus, GstreamerSink::bus_call, loop);
    gst_object_unref(bus);

    gst_bin_add_many(GST_BIN(m_pipeline), GST_ELEMENT(m_appsrc), videoconvert, NULL);
    GstCaps *capsappsrc2VideoConvert; // between appsrc and jpegenc
    capsappsrc2VideoConvert = gst_caps_new_simple("video/x-raw", "format",
                                                  G_TYPE_STRING, "BGR", "width", G_TYPE_INT, m_width, "height",
                                                  G_TYPE_INT, m_height, "framerate", GST_TYPE_FRACTION, 0, 1, NULL);


    // link elements into pipe: appsrc -> jpegenc -> appsink
    gst_element_link_filtered(GST_ELEMENT(m_appsrc), videoconvert, capsappsrc2VideoConvert);


     switch (m_sink)
    {
    case SinkType::FBDEV:

        initFbdev(videoconvert);
        break;
    case SinkType::UDP:
        initUdp(videoconvert);
        break;
    case SinkType::FILE:
        initMp4(videoconvert);
        break;
    case SinkType::KMS:
        initKmsSink(videoconvert);
        break;
    default:
        return -1;
        break;
    }

    fprintf(stderr, "Setting g_main_loop_run to GST_STATE_PLAYING\n");

    // Connect to the need-data and enough-data signals
    g_signal_connect(m_appsrc, "need-data", G_CALLBACK(GstreamerSink::cb_need_data), this);
    g_signal_connect(m_appsrc, "enough-data", G_CALLBACK(GstreamerSink::cb_enough_data), this);

    // Start playing the pipeline
    gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING);
    return 1;
}
void GstreamerSink::close()
{
    if (m_pipeline)
    {
        // Set the pipeline state to NULL to finalize the file
        gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
        // Unref the pipeline to clean up
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_appsrc = nullptr;
    }
}

void GstreamerSink::initGStreamer()
{
    static bool initialized = false;
    if (!initialized)
    {
        gst_init(nullptr, nullptr);
        initialized = true;
    }
}

int GstreamerSink::pushFrame(cv::Mat &frame)
{
    GstFlowReturn ret;

    int size = frame.total() * frame.elemSize();
    buffer = gst_buffer_new_allocate(NULL, size, NULL);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memmove(map.data, frame.data, size);
    gst_buffer_unmap(buffer, &map);

    if(m_sink == SinkType::FILE)
    {
        // Set timestamp and duration
        GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(gst_clock_get_time(gst_pipeline_get_clock(GST_PIPELINE(m_pipeline))), GST_SECOND, GST_SECOND);
        GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND, 10); // Assuming 10 FPS
    }
    else if (m_sink == SinkType::UDP)
    {
        // Set timestamp and duration
        GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(gst_clock_get_time(gst_pipeline_get_clock(GST_PIPELINE(m_pipeline))), GST_SECOND, GST_SECOND);
        GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND, 10); // Assuming 10 FPS
    }
   

    
    // Push the buffer to appsrc

    // Ensure the buffer is pushed to the framebuffer
    ret = gst_app_src_push_buffer(m_appsrc, buffer);
    // Free the buffer

    if (ret != GST_FLOW_OK)
    {
        std::cerr << "Error pushing buffer to GStreamer pipeline" << std::endl;
        return -1;
    }

    return 0;
}

void GstreamerSink::cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    GstreamerSink *sink = static_cast<GstreamerSink *>(user_data);
    // std::cout << "Need data signal received" << std::endl;    
}

void GstreamerSink::cb_enough_data(GstElement *appsrc, gpointer user_data)
{
    GstreamerSink *sink = static_cast<GstreamerSink *>(user_data);
    std::cout << "Enough data signal received" << std::endl;
    fflush(stdout);
    if(sink->m_sink == SinkType::FILE)
    {
    sink->finishFile();
    std::exit(0);

    }
}
void GstreamerSink::finishFile()
{
    GstFlowReturn ret;

    if (m_appsrc && m_pipeline)
    {
        // Send EOS signal
        ret = gst_app_src_end_of_stream(static_cast<GstAppSrc *>(m_appsrc));
        std::cout << "End of stream signal sent" << ret << std::endl;

        // Get pipeline bus
        if (bus)
        {
            // Wait for EOS or ERROR message with 5 second timeout
            GstMessage *msg = gst_bus_timed_pop_filtered(bus,
                                                         30 * GST_SECOND,
                                                         (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));

            if (msg)
            {
                switch (GST_MESSAGE_TYPE(msg))
                {
                case GST_MESSAGE_EOS:
                    std::cout << "End of stream reached" << std::endl;
                    break;
                case GST_MESSAGE_ERROR:
                    GError *err;
                    gchar *debug;
                    gst_message_parse_error(msg, &err, &debug);
                    std::cerr << "Error: " << err->message << std::endl;
                    g_error_free(err);
                    g_free(debug);
                    break;
                }
                gst_message_unref(msg);
            }
            gst_object_unref(bus);
        }

        // Set pipeline to NULL state
        gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
    }
}

gboolean GstreamerSink::bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg))
    {

    case GST_MESSAGE_EOS:
        fprintf(stderr, "End of stream\n");
        g_main_loop_quit(loop);
        break;

    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *error;

        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);

        g_printerr("Error: %s\n", error->message);
        g_error_free(error);

        g_main_loop_quit(loop);

        break;
    }
    default:
        break;
    }

    return TRUE;
}