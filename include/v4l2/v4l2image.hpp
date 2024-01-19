#pragma once

#include <sys/types.h>
#include <vector>


class V4L2Image
{
public:
        V4L2Image();

        void print(u_int32_t format, int16_t x, int16_t y, u_int8_t count = 10, int16_t shift = 0);
        int stats(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;

        u_int16_t m_bufferIndex;
        u_int16_t m_width;
        u_int16_t m_height;
        u_int16_t m_bytesPerLine;
        u_int32_t m_imageSize;
        u_int32_t m_bytesUsed;
        u_int32_t m_pixelformat;
        u_int32_t m_sequence;
        u_int64_t m_timestamp;
        std::vector<void *> m_planes;
        u_int16_t m_shift;

private:
        u_int64_t m_lastTimestamp;
        u_int8_t m_subMask;

        int stats_CPU(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;
        int stats_OpenCV_resize(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;
        int stats_OpenCV_crop(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const;
};