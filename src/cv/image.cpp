#include <cv/image.hpp>
#include <linux/videodev2.h>

Image::Image() :
        m_width(0),
        m_height(0),
        m_pixelformat(0),
        m_size(0),
        m_bytesPerLine(0),
        m_bytesPerPixel(0),
        m_sequence(0),
        m_timestamp(0),
        m_shift(0)
{
}

void Image::init(unsigned short width, unsigned short height, 
        unsigned int pixelformat, 
        unsigned int size, unsigned short bytesPerLine,
        unsigned int sequence, unsigned long timestamp)
{
        m_width = width;
        m_height = height;
        m_pixelformat = pixelformat;
        m_size = size;
        m_bytesPerLine = bytesPerLine;
        m_sequence = sequence;
        m_timestamp = timestamp;

        switch (m_pixelformat) {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8: 
                m_bytesPerPixel = 1;
                break;
        case V4L2_PIX_FMT_Y10:
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_Y12:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SBGGR12:
                m_bytesPerPixel = 2;
        }
}

unsigned short Image::pixelValue(unsigned short x, unsigned short y) const
{
        const unsigned char *data = plane(0).data();
        unsigned int index = y*m_bytesPerLine + x*m_bytesPerPixel;
        const unsigned char *pixel = data + index;
        unsigned short val16 = 0;

        switch (m_pixelformat) {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8: 
                val16 = (*(unsigned char*)pixel);
                break;
        case V4L2_PIX_FMT_Y10:
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_Y12:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SBGGR12:
                val16 = (*(unsigned short*)pixel);
        }
        val16 = val16 >> m_shift;
        return val16;
}