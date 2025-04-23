#include <cv/image.hpp>
#include <linux/videodev2.h>
#include <iostream>
Image::Image() : m_width(0),
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

        switch (m_pixelformat)
        {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8:
                m_bytesPerPixel = 1.0;
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
                m_bytesPerPixel = 2.0;
                break;
        case V4L2_PIX_FMT_Y10P:
        case V4L2_PIX_FMT_SRGGB10P:
        case V4L2_PIX_FMT_SGBRG10P:
        case V4L2_PIX_FMT_SGRBG10P:
        case V4L2_PIX_FMT_SBGGR10P:
                m_bytesPerPixel = 1.25; // 10 bits per pixel
                break;
        case V4L2_PIX_FMT_Y12P:
        case V4L2_PIX_FMT_SRGGB12P:
        case V4L2_PIX_FMT_SGBRG12P:
        case V4L2_PIX_FMT_SGRBG12P:
        case V4L2_PIX_FMT_SBGGR12P:
                m_bytesPerPixel = 1.5; // 12 bits per pixel
                break;
        case V4L2_PIX_FMT_SRGGB14P:
        case V4L2_PIX_FMT_SGBRG14P:
        case V4L2_PIX_FMT_SGRBG14P:
        case V4L2_PIX_FMT_SBGGR14P:
                m_bytesPerPixel = 1.75; // 14 bits per pixel
                break;
        }
}

u_int16_t Image::pixelValue(u_int16_t x, u_int16_t y) const
{
        const u_int8_t *data = plane(0);
        u_int32_t index = y * m_bytesPerLine + x * m_bytesPerPixel;
        const u_int8_t *pixel = data + index;
        u_int16_t val16 = 0;

        switch (m_pixelformat)
        {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8:
                val16 = (*(u_int8_t *)pixel);
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
                val16 = (*(u_int16_t *)pixel);
        case V4L2_PIX_FMT_Y10P:
        case V4L2_PIX_FMT_SRGGB10P:
        case V4L2_PIX_FMT_SGBRG10P:
        case V4L2_PIX_FMT_SGRBG10P:
        case V4L2_PIX_FMT_SBGGR10P:
        {
                unsigned int first_value_index = y * m_bytesPerLine + (x / 4) * 4 * m_bytesPerPixel + (x % 4);
                unsigned int second_value_index = y * m_bytesPerLine + (x / 4) * 4 * m_bytesPerPixel + 4;
                unsigned int mask = 3 << (x % 4) * 2;
                val16 = (data[first_value_index] << 2) | ((data[second_value_index] & mask) >> ((x % 4) * 2));
                break;
        }
        case V4L2_PIX_FMT_Y12P:
        case V4L2_PIX_FMT_SRGGB12P:
        case V4L2_PIX_FMT_SGBRG12P:
        case V4L2_PIX_FMT_SGRBG12P:
        case V4L2_PIX_FMT_SBGGR12P:
        {
                unsigned int first_value_index = y * m_bytesPerLine + (x / 2) * 2 * m_bytesPerPixel + (x % 4);
                unsigned int second_value_index = y * m_bytesPerLine + (x / 2) * 2 * m_bytesPerPixel + 2;
                unsigned int mask = 0b1111 << ((x % 2) * 4);
                val16 = (data[first_value_index] << 4) | ((((unsigned int)data[second_value_index]) & mask) >> ((x % 2) * 4));
                break;
        }
        case V4L2_PIX_FMT_SRGGB14P:
        case V4L2_PIX_FMT_SGBRG14P:
        case V4L2_PIX_FMT_SGRBG14P:
        case V4L2_PIX_FMT_SBGGR14P:
        {
                unsigned int first_value_index = y * m_bytesPerLine + (x / 4) * 4 * m_bytesPerPixel + (x % 4);
                unsigned int second_value_index = y * m_bytesPerLine + (x / 4) * 4 * m_bytesPerPixel + 4;
                unsigned int mask = 0b111111 << ((x % 4) * 6);

                val16 = (data[first_value_index] << 6) | ((((unsigned int)data[second_value_index]) & mask) >> ((x % 4) * 6));
                break;
        }
        default:
                std::cerr << "Image::pixelValue: Unsupported pixel format" <<   pixelformatString() << std::endl;
                return 0;
        }
        val16 = val16 >> m_shift;
        return val16;
}
char *Image::pixelformatString() const
{
        static char format[5];
        format[0] = (m_pixelformat >> 0 & 0xFF);
        format[1] = (m_pixelformat >> 8 & 0xFF);
        format[2] = (m_pixelformat >> 16 & 0xFF);
        format[3] = (m_pixelformat >> 24 & 0xFF);
        format[4] = '\0';
        return format;
}
