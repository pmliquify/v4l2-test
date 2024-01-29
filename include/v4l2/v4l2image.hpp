#pragma once

#include <sys/types.h>
#include <vector>
#include <commandargclass.hpp>

class V4L2Image : public CommandArgClass
{
public:
        V4L2Image();

        void printArgs();
        int setup(CommandArgs &args);

        void init(u_int16_t width, u_int16_t height, u_int16_t bytesPerLine, u_int32_t imageSize,
                u_int32_t bytesUsed, u_int32_t pixelformat, u_int32_t sequence, u_int64_t timestamp);
        void print(u_int32_t format, int16_t x, int16_t y, u_int8_t count = 10, int16_t shift = 0);
        int stats(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;

        u_int16_t width() const { return m_width; }
        u_int16_t height() const { return m_height; }
        u_int16_t bytesPerLine() const { return m_bytesPerLine; }
        u_int32_t imageSize() const { return m_imageSize; }
        u_int32_t bytesUsed() const { return m_bytesUsed; }
        u_int32_t pixelformat() const { return m_pixelformat; }
        u_int32_t sequence() const { return m_sequence; }
        u_int64_t timestamp() const { return m_timestamp; }
        u_int16_t shift() const { return m_shift; }

        u_int16_t m_bufferIndex;
        u_int32_t m_imageSize;
        std::vector<void *> m_planes;

private:
        u_int64_t m_lastTimestamp;
        u_int8_t m_subMask;
        u_int16_t m_width;
        u_int16_t m_height;
        u_int16_t m_bytesPerLine;
        u_int32_t m_bytesUsed;
        u_int32_t m_pixelformat;
        u_int32_t m_sequence;
        u_int64_t m_timestamp;
        u_int16_t m_shift;

        int stats_CPU(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;
        int stats_OpenCV_resize(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;
        int stats_OpenCV_crop(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;
};