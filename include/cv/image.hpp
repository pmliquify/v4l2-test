#pragma once

#include <vector>


class Image
{
public:
        Image();

        virtual unsigned short pixelValue(unsigned short x, unsigned short y) const;

        virtual void init(unsigned short width, unsigned short height, unsigned short bytesPerLine, unsigned int imageSize,
                unsigned int bytesUsed, unsigned int pixelformat, unsigned int sequence, unsigned long timestamp);

        unsigned short width() const { return m_width; }
        unsigned short height() const { return m_height; }
        unsigned short bytesPerLine() const { return m_bytesPerLine; }
        unsigned int imageSize() const { return m_imageSize; }
        void setImageSize(unsigned int size) { m_imageSize = size; }
        unsigned int bytesUsed() const { return m_bytesUsed; }
        unsigned int pixelformat() const { return m_pixelformat; }
        unsigned int sequence() const { return m_sequence; }
        unsigned long timestamp() const { return m_timestamp; }
        unsigned short shift() const { return m_shift; }
        void setShift(unsigned short shift) { m_shift = shift; }

        typedef std::vector<unsigned char *> Planes;
        const Planes &planes() const { return m_planes; }
        Planes &planes() { return m_planes; }
        const unsigned char *plane(unsigned char index) const { return m_planes[index]; }

protected:
        unsigned short m_width;
        unsigned short m_height;
        unsigned short m_bytesPerLine;
        unsigned char  m_bytesPerPixel;
        unsigned int   m_imageSize;
        unsigned int   m_bytesUsed;
        unsigned int   m_pixelformat;
        unsigned int   m_sequence;
        unsigned long  m_timestamp;
        unsigned short m_shift;
        Planes         m_planes;
};