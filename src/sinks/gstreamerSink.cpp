#include <sinks/gstreamerSink.hpp>

GstreamerSink::GstreamerSink()
{
}
GstreamerSink::~GstreamerSink()
{
    close();
}
void GstreamerSink::printArgs()
{
        printArgSection("GstreamerSink");
        printArg("--gst", "Gstreamer sink [file,udp,framebuffer,kms,autovideo]");
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
    else if (optionGstreamerSink == "udprk")
    {
        m_sink = SinkType::UDP_RK;
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
    else if (optionGstreamerSink == "autovideo")
    {
        m_sink = SinkType::AUTOVIDEO;
    }
    else
    {
        m_sink = SinkType::NONE;
    }

    return 0;


}

int GstreamerSink::initUdp(GstElement *link)
{
    // Create elements with standard naming
    GstElement *queue1 = gst_element_factory_make("queue", "queue1");
    GstElement *x264enc = gst_element_factory_make("x264enc", "x264enc");
    GstElement *queue2 = gst_element_factory_make("queue", "queue2");
    GstElement *h264pay = gst_element_factory_make("rtph264pay", "pay0");
    GstElement *udpsink = gst_element_factory_make("udpsink", "udpsink0");

    // Configure queue elements — bounded to prevent memory accumulation
    g_object_set(G_OBJECT(queue1),
                "max-size-buffers", 2,
                "max-size-bytes", (guint)0,
                "max-size-time", (guint64)0,
                "leaky", 2, // drop oldest when full
                NULL);

    g_object_set(G_OBJECT(queue2),
                "max-size-buffers", 2,
                "max-size-bytes", (guint)0,
                "max-size-time", (guint64)0,
                "leaky", 2, // drop oldest when full
                NULL);

    g_object_set(G_OBJECT(x264enc),
                "tune",         4,       // zerolatency
                "speed-preset", 1,       // ultrafast
                "bitrate",      4000,    // 4 Mbps in kbps
                "key-int-max",  30,
                "threads",      4,
                "byte-stream",  TRUE,    // Annex-B format for RTP
                NULL);

    // Configure RTP payloader
    g_object_set(G_OBJECT(h264pay),
                "config-interval", -1,  // Send config with every key frame
                "pt", 96,              // Payload type 96
                NULL);

    // Configure UDP sink
    g_object_set(G_OBJECT(udpsink),
                "host", m_udpHost.c_str(),
                "port", m_udpPort,
                "sync", FALSE,          // Don't sync on the clock
                "async", FALSE,         // Don't async on the clock
                NULL);

    if (!queue1 || !x264enc || !queue2 || !h264pay || !udpsink) {
        fprintf(stderr, "Failed to create one or more elements\n");
        return -1;
    }

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), queue1, x264enc, queue2, h264pay, udpsink, NULL);

    // Link elements
    if (!gst_element_link_many(link, queue1, x264enc, queue2, h264pay, udpsink, NULL)) {
        fprintf(stderr, "Failed to link elements\n");
        return -1;
    }

    return 0;
}
int GstreamerSink::initUdpRK(GstElement *link)
{
    // Create elements with standard naming
    GstElement *queue1 = gst_element_factory_make("queue", "queue1");
    GstElement *mpph264enc = gst_element_factory_make("mpph264enc", "mpph264enc");
    GstElement *queue2 = gst_element_factory_make("queue", "queue2");
    GstElement *h264pay = gst_element_factory_make("rtph264pay", "pay0");
    GstElement *udpsink = gst_element_factory_make("udpsink", "udpsink0");

    // Configure queue elements — bounded to prevent memory accumulation
    g_object_set(G_OBJECT(queue1),
                "max-size-buffers", 2,
                "max-size-bytes", (guint)0,
                "max-size-time", (guint64)0,
                "leaky", 2, // drop oldest when full
                NULL);

    g_object_set(G_OBJECT(queue2),
                "max-size-buffers", 2,
                "max-size-bytes", (guint)0,
                "max-size-time", (guint64)0,
                "leaky", 2, // drop oldest when full
                NULL);

    // Configure mpph264enc for low-latency streaming
    g_object_set(G_OBJECT(mpph264enc),
                "tune", 4,           // zero-latency
                "speed-preset", 1,   // ultrafast
                "bitrate", 2000,     // 2Mbps
                "key-int-max", 30,   // Keyframe every 30 frames
                "threads", 4,        // Use 4 threads for encoding
                NULL);

    // Configure RTP payloader
    g_object_set(G_OBJECT(h264pay),
                "config-interval", -1,  // Send config with every key frame
                "pt", 96,              // Payload type 96
                NULL);

    // Configure UDP sink
    g_object_set(G_OBJECT(udpsink),
                "host", m_udpHost.c_str(),
                "port", m_udpPort,
                "sync", FALSE,          // Don't sync on the clock
                "async", FALSE,         // Don't async on the clock
                NULL);

    if (!queue1 || !mpph264enc || !queue2 || !h264pay || !udpsink) {
        fprintf(stderr, "Failed to create one or more elements\n");
        return -1;
    }

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), queue1, mpph264enc, queue2, h264pay, udpsink, NULL);

    // Link elements
    if (!gst_element_link_many(link, queue1, mpph264enc, queue2, h264pay, udpsink, NULL)) {
        fprintf(stderr, "Failed to link elements\n");
        return -1;
    }

    return 0;
}
int GstreamerSink::initMp4(GstElement *link)
{
    GstElement *x264enc  = gst_element_factory_make("x264enc",  "vc_x264enc");
    GstElement *mp4mux   = gst_element_factory_make("mp4mux",   "vc_mp4mux");
    GstElement *filesink = gst_element_factory_make("filesink", "vc_sink");

    if (!x264enc || !mp4mux || !filesink) {
        fprintf(stderr, "initMp4: failed to create one or more elements\n");
        return -1;
    }

    g_object_set(G_OBJECT(x264enc),
                 "bitrate",      10000,
                 "speed-preset", 3,
                 NULL);
    g_object_set(G_OBJECT(filesink), "location", m_filename.c_str(), NULL);

    gst_bin_add_many(GST_BIN(m_pipeline), x264enc, mp4mux, filesink, NULL);

    if (!gst_element_link_many(link, x264enc, mp4mux, filesink, NULL)) {
        fprintf(stderr, "initMp4: failed to link elements\n");
        return -1;
    }

    return 0;
}

int GstreamerSink::initKmsSink(GstElement *link)
{

    GstElement *kmssink = gst_element_factory_make("kmssink", "vc_sink");
    g_object_set(G_OBJECT(kmssink), "sync", TRUE, NULL);

    if (!kmssink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        return -1;
    }
    
    // GstCaps *capsScale2fbdevsink; 
    // capsScale2fbdevsink = gst_caps_new_simple("video/x-raw", "format",
    //                                           G_TYPE_STRING, "RGB16", "width", G_TYPE_INT, m_width, "height",
    //                                           G_TYPE_INT, m_height, "framerate", GST_TYPE_FRACTION, 0, 1, NULL);


    gst_bin_add_many(GST_BIN(m_pipeline), kmssink, NULL);


    gst_element_link(link, kmssink);
    // gst_element_link_filtered(videoscale, kmssink, capsScale2fbdevsink);


    return 0;
}
int GstreamerSink::initAutoVideoSink(GstElement *link)
{
    GstElement *autovideosink = gst_element_factory_make("autovideosink", "vc_sink");
    if (!autovideosink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        return -1;
    }
    
    gst_bin_add_many(GST_BIN(m_pipeline), autovideosink, NULL);

    gst_element_link(link, autovideosink);
    return 0;
}

int GstreamerSink::initFbdev(GstElement *link)
{
     GstElement *videoscale = gst_element_factory_make("videoscale", "vc_scale");

    GstElement *fbdevsink = gst_element_factory_make("fbdevsink", "vc_sink");
    g_object_set(G_OBJECT (fbdevsink), "device", m_framebufferDevice.c_str(), NULL);
    g_object_set(G_OBJECT(fbdevsink), "sync", FALSE, NULL);
    if (!videoscale || !fbdevsink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        return -1;
    }
    
    GstCaps *capsScale2fbdevsink; 
    capsScale2fbdevsink = gst_caps_new_simple("video/x-raw", "format",
                                              G_TYPE_STRING, "RGB16", "width", G_TYPE_INT, m_width, "height",
                                              G_TYPE_INT, m_height, NULL);


    gst_bin_add_many(GST_BIN(m_pipeline), videoscale, fbdevsink, NULL);


    gst_element_link(link, videoscale);
    gst_element_link_filtered(videoscale, fbdevsink, capsScale2fbdevsink);


    return 0;
}
int GstreamerSink::init(int width, int height)
{
    initGStreamer();

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
                 "block", FALSE,
                 "max-bytes", (guint64)0,         // unlimited; flow control is in pusherThread
                 "blocksize", width * height * 3, // Set blocksize to image size
                 nullptr);

    loop = g_main_loop_new(NULL, FALSE);

    bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    bus_watch_id = gst_bus_add_watch(bus, GstreamerSink::bus_call, loop);
    gst_object_unref(bus);

    // queue0 decouples the pusher thread from downstream processing:
    // appsrc → queue0 (thread boundary) → videoconvert → ...
    // Without queue0, videoconvert runs synchronously in the pusher thread.
    GstElement *queue0 = gst_element_factory_make("queue", "input_queue");
    g_object_set(G_OBJECT(queue0),
                 "max-size-buffers", 2,
                 "max-size-bytes",   (guint)0,
                 "max-size-time",    (guint64)0,
                 "leaky",            2,  // drop oldest when full
                 NULL);

    gst_bin_add_many(GST_BIN(m_pipeline), GST_ELEMENT(m_appsrc), queue0, videoconvert, NULL);
    GstCaps *capsappsrc2VideoConvert;
    capsappsrc2VideoConvert = gst_caps_new_simple("video/x-raw", "format",
                                                  G_TYPE_STRING, "BGR", "width", G_TYPE_INT, m_width, "height",
                                                  G_TYPE_INT, m_height, "framerate", GST_TYPE_FRACTION, 0, 1, NULL);

    gst_element_link_filtered(GST_ELEMENT(m_appsrc), queue0, capsappsrc2VideoConvert);
    gst_element_link(queue0, videoconvert);


     switch (m_sink)
    {
    case SinkType::FBDEV:

        initFbdev(videoconvert);
        break;
    case SinkType::UDP:
        initUdp(videoconvert);
        break;
    case SinkType::UDP_RK:
        initUdpRK(videoconvert);
        break;
    case SinkType::FILE:
        initMp4(videoconvert);
        break;
    case SinkType::KMS:
        initKmsSink(videoconvert);
        break;
    case SinkType::AUTOVIDEO:
        initAutoVideoSink(videoconvert);
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

    m_stop   = false;
    m_pusher = std::thread(&GstreamerSink::pusherThread, this);

    return 1;
}
void GstreamerSink::close()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_one();
    if (m_pusher.joinable())
        m_pusher.join();

    if (m_pipeline)
    {
        if (m_appsrc && m_sink == SinkType::FILE) {
            gst_app_src_end_of_stream(m_appsrc);
            GstBus *ebus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
            GstMessage *msg = gst_bus_timed_pop_filtered(ebus, 30 * GST_SECOND,
                (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
            if (msg) gst_message_unref(msg);
            gst_object_unref(ebus);
        }

        gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_appsrc   = nullptr;
    }
}

void GstreamerSink::initGStreamer()
{
    static bool initialized = false;
    if (!initialized)
    {
        gst_init(nullptr, nullptr);
        gst_debug_set_active(FALSE);
        initialized = true;
    }
}

int GstreamerSink::pushFrame(cv::Mat &frame)
{
    cv::Mat cloned = frame.clone();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_frameQueue.size() >= 2)
            m_frameQueue.pop(); // drop oldest to keep queue bounded
        m_frameQueue.push(std::move(cloned));
    }
    m_cv.notify_one();
    return 0;
}

void GstreamerSink::pusherThread()
{
    while (true) {
        cv::Mat frame;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_frameQueue.empty() || m_stop; });
            if (m_frameQueue.empty())
                break;
            frame = std::move(m_frameQueue.front());
            m_frameQueue.pop();
        }

        int size = frame.total() * frame.elemSize();
        GstBuffer *buf = gst_buffer_new_allocate(NULL, size, NULL);
        GstMapInfo map;
        gst_buffer_map(buf, &map, GST_MAP_WRITE);
        memcpy(map.data, frame.data, size);
        gst_buffer_unmap(buf, &map);

        GstClock *clk = gst_element_get_clock(GST_ELEMENT(m_pipeline));
        if (clk) {
            GstClockTime abs  = gst_clock_get_time(clk);
            GstClockTime base = gst_element_get_base_time(GST_ELEMENT(m_pipeline));
            GST_BUFFER_PTS(buf) = (abs > base) ? (abs - base) : 0;
            gst_object_unref(clk);
        } else {
            GST_BUFFER_PTS(buf) = GST_CLOCK_TIME_NONE;
        }
        GST_BUFFER_DURATION(buf) = GST_CLOCK_TIME_NONE;

        GstFlowReturn ret = gst_app_src_push_buffer(m_appsrc, buf);
        if (ret != GST_FLOW_OK)
            std::cerr << "Error pushing buffer to GStreamer pipeline" << std::endl;
    }
}

void GstreamerSink::cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    GstreamerSink *sink = static_cast<GstreamerSink *>(user_data);
    // std::cout << "Need data signal received" << std::endl;    
}

void GstreamerSink::cb_enough_data(GstElement *appsrc, gpointer user_data)
{
    // No action needed; back-pressure is handled by the bounded frameQueue.
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