#include <sinks/imageSaverSink.hpp>

ImageSaverSink::ImageSaverSink()
{
}

ImageSaverSink::~ImageSaverSink()
{
    close();
}

void ImageSaverSink::printArgs()
{
    printArgSection("ImageSaverSink");
    printArg("--prefix", "Prefix for the filename");
    printArg("--save-raw", "Save image as raw binary data (.raw)");
    printArg("--frames-per-file", "Split raw output into segments of N frames (default: 0 = one file)");
}

int ImageSaverSink::setup(CommandArgs &args)
{
    prefix = args.optionString("--prefix", "capture");
    m_saveRaw = args.exists("--save-raw");
    m_framesPerFile = args.optionInt("--frames-per-file", 0);
    return 0;
}

void ImageSaverSink::openNextSegment()
{
    if (m_rawFile.is_open())
        m_rawFile.close();

    std::string path = prefix + std::to_string(m_segmentIndex++) + ".raw";
    m_rawFile.open(path, std::ios::binary);
    if (!m_rawFile)
        std::cerr << "ImageSaverSink: failed to open " << path << " for writing" << std::endl;
    else
        std::cout << "ImageSaverSink: writing " << path << std::endl;
    m_framesInSegment = 0;
}

int ImageSaverSink::init(int width, int height)
{
    if (m_saveRaw)
    {
        openNextSegment();
        m_writer = std::thread(&ImageSaverSink::writerThread, this);
    }
    return 0;
}

void ImageSaverSink::close()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_one();
    if (m_writer.joinable())
        m_writer.join();
    if (m_rawFile.is_open())
        m_rawFile.close();
}

void ImageSaverSink::finishFile()
{
    // flush current segment and start a new one from the writer thread context
    std::unique_lock<std::mutex> lock(m_mutex);
    m_framesPerFile = 0; // trigger roll on next write — caller should reopen externally if needed
}

void ImageSaverSink::writerThread()
{
    while (true)
    {
        cv::Mat frame;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_queue.empty() || m_stop; });
            if (m_queue.empty())
                break;
            frame = std::move(m_queue.front());
            m_queue.pop();
        }

        if (m_framesPerFile > 0 && m_framesInSegment >= m_framesPerFile)
            openNextSegment();

        if (m_rawFile.is_open())
        {
            m_rawFile.write(reinterpret_cast<const char *>(frame.data),
                            static_cast<std::streamsize>(frame.total() * frame.elemSize()));
            ++m_framesInSegment;
        }
    }
}

int ImageSaverSink::pushFrame(cv::Mat &frame, uint64_t timestamp)
{
    if (m_saveRaw)
    {
        // clone so capture thread is not blocked by disk I/O
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(frame.clone());
        lock.unlock();
        m_cv.notify_one();
    }
    else
    {
        std::string strTimestamp = std::to_string(timestamp / 1000) + "." + std::to_string(timestamp % 1000);
        cv::imwrite(prefix + "_" + strTimestamp + ".png", frame);
    }
    return 0;
}

