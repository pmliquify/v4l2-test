#pragma once

#include <cv/plane.hpp>


class Image
{
public:
        Image();

        virtual unsigned short pixelValue(unsigned short x, unsigned short y) const;

        virtual void init(unsigned short width, unsigned short height, 
                unsigned int pixelformat, 
                unsigned int imageSize, unsigned short bytesPerLine,
                unsigned int sequence, unsigned long timestamp);

        unsigned short width() const { return m_width; }
        unsigned short height() const { return m_height; }
        unsigned int pixelformat() const { return m_pixelformat; }
        unsigned int size() const { return m_size; }
        void setSize(unsigned int size) { m_size = size; }
        unsigned short bytesPerLine() const { return m_bytesPerLine; }
        
        const Plane &plane(unsigned int index) const { return m_planes[index]; }
        Plane &plane(unsigned int index) { return m_planes[index]; }
        const Planes &planes() const { return m_planes; }
        Planes &planes() { return m_planes; }

        unsigned int sequence() const { return m_sequence; }
        unsigned long timestamp() const { return m_timestamp; }
        unsigned short shift() const { return m_shift; }
        void setShift(unsigned short shift) { m_shift = shift; }

protected:
        unsigned short m_width;
        unsigned short m_height;
        unsigned int   m_pixelformat;
        unsigned int   m_size;
        unsigned short m_bytesPerLine;
        unsigned char  m_bytesPerPixel;
        Planes         m_planes;
        unsigned int   m_sequence;
        unsigned long  m_timestamp;
        unsigned short m_shift;
};