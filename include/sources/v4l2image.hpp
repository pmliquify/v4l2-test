#pragma once

#include <cv/image.hpp>


class V4L2Image : public Image
{
public:
        V4L2Image();

        u_int16_t bufferIndex() { return m_bufferIndex; }
        void setBufferIndex(u_int16_t index) { m_bufferIndex = index; }

private:
        u_int16_t m_bufferIndex;
};