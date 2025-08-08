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
                case V4L2_PIX_FMT_SRGGB12P:
                case V4L2_PIX_FMT_SGBRG12P:
                case V4L2_PIX_FMT_SGRBG12P:
                case V4L2_PIX_FMT_SBGGR12P:
                        m_bytesPerPixel = 1.5; // 12 bits per pixel
                        break; 
                case V4L2_PIX_FMT_Y16:
                case V4L2_PIX_FMT_SRGGB16:
                case V4L2_PIX_FMT_SGBRG16:
                case V4L2_PIX_FMT_SGRBG16:
                case V4L2_PIX_FMT_SBGGR16:
                        m_bytesPerPixel = 2.0; // 16 bits per pixel
                        break;
                default:
                        m_bytesPerPixel = 0.0; // Unsupported pixel format
                        break;
                

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
        case V4L2_PIX_FMT_Y10P:
        case V4L2_PIX_FMT_SRGGB10P:
        case V4L2_PIX_FMT_SGBRG10P:
        case V4L2_PIX_FMT_SGRBG10P:
        case V4L2_PIX_FMT_SBGGR10P:
        {
                unsigned int first_value_index = y*m_bytesPerLine + (x/4)*4  * m_bytesPerPixel + (x%4);
                unsigned int second_value_index = y*m_bytesPerLine + (x/4)*4 * m_bytesPerPixel + 4;
                unsigned int mask = 3 << (x%4);
                val16 = (data[first_value_index] << 2) | ((data[second_value_index] & mask) >> (x%4));
                break;
                
        }
        case V4L2_PIX_FMT_SRGGB12P:
        case V4L2_PIX_FMT_SGBRG12P:
        case V4L2_PIX_FMT_SGRBG12P:
        case V4L2_PIX_FMT_SBGGR12P:
        {
              
                unsigned int pixel_pair_index = (x / 2) * 3;
                unsigned int base_index = y * m_bytesPerLine + pixel_pair_index;
                
                if (x % 2 == 0) {
                        // Even pixel (0, 2, 4, ...)
                        val16 = (data[base_index ]<< 4) | ((data[base_index + 2] & 0x0F));
                } else {
                        // Odd pixel (1, 3, 5, ...)
                        val16 = (data[base_index + 1]<< 4) | ((data[base_index + 2] & 0xF0) >> 4);
                }
                break;
        }

        
        
        case V4L2_PIX_FMT_Y16:
        case V4L2_PIX_FMT_SRGGB16:
        case V4L2_PIX_FMT_SGBRG16:
        case V4L2_PIX_FMT_SGRBG16:
        case V4L2_PIX_FMT_SBGGR16:
                val16 = (*(u_int16_t*)pixel);
                break;
        default:
                val16 = 0; // Unsupported pixel format
                break;
        }
        val16 = val16 >> m_shift;
        return val16;
}
