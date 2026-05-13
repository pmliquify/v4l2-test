#include <sinks/videoSink.hpp>
#include <iostream>

VideoSink::VideoSink()
{
}

VideoSink::~VideoSink()
{
    close();
}

void VideoSink::printArgs()
{
    printArgSection("VideoSink");
    printArg("--output",  "Output video file (default: output.mp4)");
    printArg("--fps",     "Frames per second (default: 30)");
    printArg("--crf",     "H.264 quality: 0=lossless, 23=default, 51=worst (default: 23)");
    printArg("--vcodec",  "Video codec name passed to libav (default: libx264)");
}

int VideoSink::setup(CommandArgs &args)
{
    m_output = args.optionString("--output", "output.mp4");
    m_fps    = args.optionInt("--fps",  30);
    m_crf    = args.optionInt("--crf",  23);
    m_codec  = args.optionString("--vcodec", "libx264");
    return 0;
}

int VideoSink::init(int width, int height, bool grayscale)
{
    m_width     = width;
    m_height    = height;
    m_grayscale = grayscale;

    // Allocate output format context
    if (avformat_alloc_output_context2(&m_fmtCtx, nullptr, nullptr, m_output.c_str()) < 0) {
        std::cerr << "VideoSink: could not deduce output format from filename '" << m_output << "'" << std::endl;
        return -1;
    }

    // Find encoder
    const AVCodec *codec = avcodec_find_encoder_by_name(m_codec.c_str());
    if (!codec) {
        std::cerr << "VideoSink: encoder '" << m_codec << "' not found" << std::endl;
        return -1;
    }

    // Create stream
    m_stream = avformat_new_stream(m_fmtCtx, nullptr);
    if (!m_stream) {
        std::cerr << "VideoSink: could not allocate stream" << std::endl;
        return -1;
    }
    m_stream->id = 0;

    // Allocate codec context
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        std::cerr << "VideoSink: could not allocate codec context" << std::endl;
        return -1;
    }

    m_codecCtx->width     = m_width;
    m_codecCtx->height    = m_height;
    m_codecCtx->time_base = { 1, m_fps };
    m_codecCtx->framerate = { m_fps, 1 };
    m_codecCtx->pix_fmt   = AV_PIX_FMT_YUV420P;
    m_codecCtx->gop_size  = m_fps; // one keyframe per second

    // CRF quality via private options (works for libx264 / libx265)
    av_opt_set_int(m_codecCtx->priv_data, "crf", m_crf, 0);
    // Fast encoding preset; ignore failure (not all codecs support it)
    av_opt_set(m_codecCtx->priv_data, "preset", "fast", 0);

    if (m_fmtCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        m_codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // Open codec
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        std::cerr << "VideoSink: could not open codec '" << m_codec << "'" << std::endl;
        return -1;
    }

    avcodec_parameters_from_context(m_stream->codecpar, m_codecCtx);
    m_stream->time_base = m_codecCtx->time_base;

    // Open output file
    if (!(m_fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_fmtCtx->pb, m_output.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "VideoSink: could not open '" << m_output << "' for writing" << std::endl;
            return -1;
        }
    }

    if (avformat_write_header(m_fmtCtx, nullptr) < 0) {
        std::cerr << "VideoSink: error writing header" << std::endl;
        return -1;
    }

    // Allocate reusable frame (YUV420P)
    m_frame = av_frame_alloc();
    m_frame->format = AV_PIX_FMT_YUV420P;
    m_frame->width  = m_width;
    m_frame->height = m_height;
    if (av_frame_get_buffer(m_frame, 0) < 0) {
        std::cerr << "VideoSink: could not allocate frame buffer" << std::endl;
        return -1;
    }

    m_packet = av_packet_alloc();

    // sws context: convert from BGR or GRAY8 to YUV420P
    AVPixelFormat srcFmt = m_grayscale ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_BGR24;
    m_swsCtx = sws_getContext(
        m_width, m_height, srcFmt,
        m_width, m_height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!m_swsCtx) {
        std::cerr << "VideoSink: could not create sws context" << std::endl;
        return -1;
    }

    m_initialized = true;
    m_stop        = false;
    m_frameIndex  = 0;

    m_writer = std::thread(&VideoSink::writerThread, this);

    std::cout << "VideoSink: writing " << m_output
              << " (" << m_codec << ", " << m_fps << " fps, crf=" << m_crf << ")" << std::endl;
    return 0;
}

void VideoSink::close()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_one();
    if (m_writer.joinable())
        m_writer.join();

    if (m_initialized) {
        flushEncoder();
        av_write_trailer(m_fmtCtx);
    }

    sws_freeContext(m_swsCtx);       m_swsCtx   = nullptr;
    av_frame_free(&m_frame);
    av_packet_free(&m_packet);
    avcodec_free_context(&m_codecCtx);

    if (m_fmtCtx) {
        if (!(m_fmtCtx->oformat->flags & AVFMT_NOFILE))
            avio_closep(&m_fmtCtx->pb);
        avformat_free_context(m_fmtCtx);
        m_fmtCtx = nullptr;
    }

    m_initialized = false;
}

int VideoSink::pushFrame(cv::Mat &frame, uint64_t timestamp)
{
    FrameEntry entry;
    entry.mat       = frame.clone();
    entry.timestamp = timestamp;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::move(entry));
    }
    m_cv.notify_one();
    return 0;
}

void VideoSink::writerThread()
{
    while (true) {
        FrameEntry entry;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_queue.empty() || m_stop; });
            if (m_queue.empty())
                break;
            entry = std::move(m_queue.front());
            m_queue.pop();
        }
        encodeAndWrite(entry.mat);
    }
}

int VideoSink::encodeAndWrite(cv::Mat &frame)
{
    if (av_frame_make_writable(m_frame) < 0) {
        std::cerr << "VideoSink: frame not writable" << std::endl;
        return -1;
    }

    // Convert cv::Mat to AVFrame (YUV420P)
    const uint8_t *srcSlice[1] = { frame.data };
    int srcStride[1]            = { static_cast<int>(frame.step) };
    sws_scale(m_swsCtx, srcSlice, srcStride, 0, m_height,
              m_frame->data, m_frame->linesize);

    m_frame->pts = m_frameIndex++;

    // Send frame to encoder
    int ret = avcodec_send_frame(m_codecCtx, m_frame);
    if (ret < 0) {
        std::cerr << "VideoSink: error sending frame to encoder" << std::endl;
        return -1;
    }

    // Receive and write packets
    while (ret >= 0) {
        ret = avcodec_receive_packet(m_codecCtx, m_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0) {
            std::cerr << "VideoSink: error receiving packet from encoder" << std::endl;
            return -1;
        }
        av_packet_rescale_ts(m_packet, m_codecCtx->time_base, m_stream->time_base);
        m_packet->stream_index = m_stream->index;
        av_interleaved_write_frame(m_fmtCtx, m_packet);
        av_packet_unref(m_packet);
    }
    return 0;
}

void VideoSink::flushEncoder()
{
    // Send flush signal
    avcodec_send_frame(m_codecCtx, nullptr);
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(m_codecCtx, m_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            break;
        av_packet_rescale_ts(m_packet, m_codecCtx->time_base, m_stream->time_base);
        m_packet->stream_index = m_stream->index;
        av_interleaved_write_frame(m_fmtCtx, m_packet);
        av_packet_unref(m_packet);
    }
}
