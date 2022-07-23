#pragma once

#include <sys/types.h>
#include <vector>


class V4L2Image
{
public:
        V4L2Image();

        void print(u_int32_t format, int16_t x, int16_t y, u_int8_t count = 10);

        u_int16_t m_bufferIndex;
        u_int16_t m_width;
        u_int16_t m_height;
        u_int16_t m_bytesPerLine;
        u_int32_t m_pixelformat;
        u_int32_t m_sequence;
        u_int64_t m_timestamp;
        std::vector<void *> m_planes;

private:
        u_int64_t m_lastTimestamp;
};