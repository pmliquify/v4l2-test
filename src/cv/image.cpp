#include <cv/image.hpp>
#include <linux/videodev2.h>

Image::Image() :
        m_width(0),
        m_height(0),
        m_bytesPerLine(0),
        m_bytesPerPixel(0),
        m_imageSize(0),
        m_bytesUsed(0),
        m_pixelformat(0),
        m_sequence(0),
        m_timestamp(0)
{
}

void Image::init(u_int16_t width, u_int16_t height, u_int16_t bytesPerLine, u_int32_t imageSize,
                u_int32_t bytesUsed, u_int32_t pixelformat, u_int32_t sequence, u_int64_t timestamp)
{
        m_width = width;
        m_height = height;
        m_bytesPerLine = bytesPerLine;
        m_imageSize = imageSize;
        m_bytesUsed = bytesUsed;
        m_pixelformat = pixelformat;
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

u_int16_t Image::pixelValue(u_int16_t x, u_int16_t y) const
{
        const u_int8_t *data = plane(0);
        u_int32_t index = y*m_bytesPerLine + x*m_bytesPerPixel;
        const u_int8_t *pixel = data + index;
        u_int16_t val16 = 0;

        switch (m_pixelformat) {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8: 
                val16 = (*(u_int8_t*)pixel);
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
                val16 = (*(u_int16_t*)pixel);
        }
        val16 = val16 >> m_shift;
        return val16;
}