#pragma once

#include <sys/types.h>
#include <vector>


class Image
{
public:
        Image();

        virtual u_int16_t pixelValue(u_int16_t x, u_int16_t y) const;

        virtual void init(u_int16_t width, u_int16_t height, u_int16_t bytesPerLine, u_int32_t imageSize,
                u_int32_t bytesUsed, u_int32_t pixelformat, u_int32_t sequence, u_int64_t timestamp);

        u_int16_t width() const { return m_width; }
        u_int16_t height() const { return m_height; }
        u_int16_t bytesPerLine() const { return m_bytesPerLine; }
        u_int32_t imageSize() const { return m_imageSize; }
        void setImageSize(u_int32_t size) { m_imageSize = size; }
        u_int32_t bytesUsed() const { return m_bytesUsed; }
        u_int32_t pixelformat() const { return m_pixelformat; }
        u_int32_t sequence() const { return m_sequence; }
        u_int64_t timestamp() const { return m_timestamp; }
        u_int16_t shift() const { return m_shift; }
        void setShift(u_int16_t shift) { m_shift = shift; }

        typedef std::vector<u_int8_t *> Planes;
        const Planes &planes() const { return m_planes; }
        Planes &planes() { return m_planes; }
        const u_int8_t *plane(u_int8_t index) const { return m_planes[index]; }

protected:
        u_int16_t m_width;
        u_int16_t m_height;
        u_int16_t m_bytesPerLine;
        u_int8_t  m_bytesPerPixel;
        u_int32_t m_imageSize;
        u_int32_t m_bytesUsed;
        u_int32_t m_pixelformat;
        u_int32_t m_sequence;
        u_int64_t m_timestamp;
        u_int16_t m_shift;
        Planes    m_planes;
};