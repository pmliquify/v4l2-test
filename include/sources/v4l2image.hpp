#pragma once

#include <cv/image.hpp>


class V4L2Image : public Image
{
public:
        V4L2Image();

        unsigned short bufferIndex() { return m_bufferIndex; }
        void setBufferIndex(unsigned short index) { m_bufferIndex = index; }

private:
        unsigned short m_bufferIndex;
};