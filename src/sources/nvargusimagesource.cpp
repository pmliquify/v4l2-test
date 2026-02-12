#include <sources/nvargusimagesource.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <linux/videodev2.h>
#include <iostream>


NvArgusImageSource::NvArgusImageSource() :
        m_pipeline(NULL),
        m_sink(NULL),
        m_sensorId(0),
        m_aeLock(false),
        m_aeLeft(0),
        m_aeTop(0),
        m_aeWidth(0),
        m_aeHeight(0),
        m_gainRange(1),
        m_ispDigitalGainRange(1),
        m_awbLock(false),
        m_wbMode(1),
        m_tnrMode(1),
        m_width(1024),
        m_height(768),
        m_frameRate(20),
        m_sequence(0)
{
        m_image = new Image();

        gst_init(NULL, NULL);
}


NvArgusImageSource::~NvArgusImageSource()
{
        gst_deinit();

        delete m_image;
        m_image = NULL;
}

void NvArgusImageSource::printArgs() 
{
        printArgSection("NvArgus ImageSource");
        printArg("--sensorId", "Set sensor ID (default: 0)");
        printArg("--aeLock", "Enable Auto Exposure Lock (default: 0 = enabled)");
        printArg("--aeLeft", "Set Auto Exposure Region Left (default: 0)");
        printArg("--aeTop", "Set Auto Exposure Region Top (default: 0)");
        printArg("--aeWidth", "Set Auto Exposure Region Width (default: 0 = full frame)");
        printArg("--aeHeight", "Set Auto Exposure Region Height (default: 0 = full frame)");
        printArg("--gainRange", "Set Analog Gain Range (default: 1 = disabled)");
        printArg("--ispDigitalGainRange", "Set Digital Gain Range (default: 1 = disabled)");
        printArg("--awbLock", "Enable Auto White Balance Lock (default: 0 = enabled)");
        printArg("--wbMode", "Set White Balance mode (default: 1 = auto)");
        printArg("--tnrMode", "Set Temporal Noise Reduction mode (default: 1 = fast)");
        printArg("--width", "Set Image Width (default: 1024)");
        printArg("--height", "Set Image Height (default: 768)");
        printArg("--frameRate", "Set Frame Rate (default: 20)");
}

int NvArgusImageSource::setup(CommandArgs &args)
{
        m_sensorId = args.optionInt("--sensorId", 0);
        m_aeLock = args.optionInt("--aeLock", 0);
        m_aeLeft = args.optionInt("--aeLeft", 0);
        m_aeTop = args.optionInt("--aeTop", 0);
        m_aeWidth = args.optionInt("--aeWidth", 0);
        m_aeHeight = args.optionInt("--aeHeight", 0);
        m_gainRange = args.optionInt("--gainRange", 1);
        m_ispDigitalGainRange = args.optionInt("--ispDigitalGainRange", 1);
        m_awbLock = args.optionInt("--awbLock", 0);
        m_wbMode = args.optionInt("--wbMode", 1);
        m_tnrMode = args.optionInt("--tnrMode", 1);
        m_width = args.optionInt("--width", 1024);
        m_height = args.optionInt("--height", 768);
        m_frameRate = args.optionInt("--frameRate", 20);
        
        open(args.option("-d", "/dev/video0"), args.option("-sd", ""));

        return 0;
}

int NvArgusImageSource::open(const std::string devicePath, const std::string subDevicePath)
{
        m_sequence = 0;

        std::string aeRegion;
        if (m_aeWidth > 0 && m_aeHeight > 0) {
                aeRegion = std::to_string(m_aeLeft) + " " + 
                        std::to_string(m_aeTop) + " " +
                        std::to_string(m_aeLeft + m_aeWidth) + " " + 
                        std::to_string(m_aeTop + m_aeHeight) + " 1";
        }

        // nvarguscamerasrc (NVMM) -> nvvidconv (to system RAM) -> NV12 -> appsink
        // IMPORTANT: The caps "video/x-raw,format=NV12" AFTER nvvidconv ensures host memory.
        std::string pipeline_desc =
                "nvarguscamerasrc sensor-id=" + std::to_string(m_sensorId) + 
                " aelock=" + (m_aeLock ? "true" : "false") +
                (aeRegion.empty() ? "" : " aeregion=\"" + aeRegion + "\"") +
                " gainrange=\"1 " + std::to_string(m_gainRange) + "\"" +
                " ispdigitalgainrange=\"1 " + std::to_string(m_ispDigitalGainRange) + "\"" +
                " awblock=" + (m_awbLock ? "true" : "false") +
                " wbmode=" + std::to_string(m_wbMode) +
                " tnr-mode=" + std::to_string(m_tnrMode) + " ! " +
                "video/x-raw(memory:NVMM), width=" + std::to_string(m_width) +
                ", height=" + std::to_string(m_height) +
                ", format=NV12, framerate=" + std::to_string(m_frameRate) + "/1 ! "
                "nvvidconv ! "
                "video/x-raw, format=NV12, width=" + std::to_string(m_width) +
                ", height=" + std::to_string(m_height) + " ! "
                "appsink name=sink emit-signals=false sync=false max-buffers=2 drop=true";

        std::cout << "GStreamer Pipeline: " << pipeline_desc << std::endl;

        GError* err = nullptr;
        m_pipeline = gst_parse_launch(pipeline_desc.c_str(), &err);
        if (!m_pipeline) {
                std::cerr << "Pipeline-Fehler: " << (err ? err->message : "unknown") << "\n";
                if (err) g_error_free(err);
                return 1;
        }

        m_sink = gst_bin_get_by_name(GST_BIN(m_pipeline), "sink");
        if (!m_sink) {
                std::cerr << "appsink nicht gefunden\n";
                gst_object_unref(m_pipeline);
                return 1;
        }

        return 0;
}

int NvArgusImageSource::setSelection(int left, int top, int width, int height) 
{ 
        std::cout << "NvArgusImageSource::setSelection()\n";
        return 0; 
}

int NvArgusImageSource::close()
{
        if (m_pipeline) {
                gst_element_set_state(m_pipeline, GST_STATE_NULL);
                gst_object_unref(m_pipeline);
                m_pipeline = NULL;
                m_sink = NULL;
        }
        return 0;
}

int NvArgusImageSource::streamOn(int bufferCount)
{
        if (!m_pipeline) {
                return 1;
        }
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        return 0;
}

int NvArgusImageSource::streamOff()
{
        if (!m_pipeline) {
                return 1;
        }
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        return 0;
}

int NvArgusImageSource::getNextImage(Image *&image, int timeout, bool lastImage)
{
        GstAppSink* appSink = GST_APP_SINK(m_sink);
        GstSample* sample = gst_app_sink_pull_sample(appSink); // blockiert bis Frame ankommt
        if (!sample) {
            std::cout << "EOS oder Fehler.\n";
        }

        GstCaps* caps = gst_sample_get_caps(sample);
        GstVideoInfo vinfo;
        gst_video_info_from_caps(&vinfo, caps);

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        if (!buffer) { 
                gst_sample_unref(sample); 
                return 1;
        }

        GstClockTime pts = GST_BUFFER_PTS(buffer);

        GstMapInfo map;
        int ret = gst_buffer_map(buffer, &map, GST_MAP_READ);
        if (!ret) {
                gst_sample_unref(sample);
                return -2;
        }

        unsigned short width = GST_VIDEO_INFO_WIDTH(&vinfo);
        unsigned short height = GST_VIDEO_INFO_HEIGHT(&vinfo);
        unsigned int size = map.size;
        unsigned short bytesPerLine = GST_VIDEO_INFO_PLANE_STRIDE(&vinfo, 0);
        const gsize offsetY  = GST_VIDEO_INFO_PLANE_OFFSET(&vinfo, 0);
        const gsize offsetUV = GST_VIDEO_INFO_PLANE_OFFSET(&vinfo, 1);
        unsigned int sizeY = offsetUV - offsetY;
        unsigned int sizeUV = map.size - offsetUV;

        m_image->init(width, height, V4L2_PIX_FMT_NV12, size, bytesPerLine, 
                m_sequence++, pts / GST_MSECOND);
        m_image->planes().resize(2);
        m_image->plane(0).init(map.data + offsetY, sizeY);
        m_image->plane(1).init(map.data + offsetUV, sizeUV);

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);

        image = m_image;
        return 0;
}

int NvArgusImageSource::releaseImage(Image *image)
{
        return 0;
}

