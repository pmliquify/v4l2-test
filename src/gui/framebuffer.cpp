#include <gui/framebuffer.hpp>
#include <utils/errno.hpp>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#if _OPENMP
#include <omp.h>
#endif
#include <linux/kd.h>
#include <linux/vt.h>


FrameBuffer::FrameBuffer() : m_fd(0),
                             m_ptr(NULL),
                             m_console_fd(0)
{
}

FrameBuffer::~FrameBuffer()
{
        close();
}

int FrameBuffer::open()
{
        // Try different console devices for cursor control
        m_console_fd = -1;
        const char *consoleDevices[] = {"/dev/tty3", "/dev/tty2", "/dev/tty1", "/dev/tty0", "/dev/console"};

        for (const char* device : consoleDevices) {
                m_console_fd = ::open(device, O_RDWR);
                if (m_console_fd >= 0) {
                        // Turn off cursor using KDSETMODE
                        if (ioctl(m_console_fd, KDSETMODE, KD_GRAPHICS) == 0) {
                               // Also try to hide cursor using VT100 escape sequence
                                const char *hide_cursor = "\033[?25l";
                                write(m_console_fd, hide_cursor, 6);
                                break;
                        }
                        ::close(m_console_fd);
                }
        }

        const char *DevicePath = "/dev/fb0";
        m_fd = ::open(DevicePath, O_RDWR);
        if (-1 == m_fd)
        {
                handleErrorForOpen(DevicePath, errno);
                return -1;
        }

        if (-1 == ioctl(m_fd, FBIOGET_VSCREENINFO, &m_varScreenInfo))
        {
                handleErrorForIoctl(FBIOGET_VSCREENINFO, errno);
                return -1;
        }

        if (-1 == ioctl(m_fd, FBIOGET_FSCREENINFO, &m_fixScreenInfo))
        {
                handleErrorForIoctl(FBIOGET_FSCREENINFO, errno);
                return -1;
        }

        int byteCount = m_varScreenInfo.xres * m_varScreenInfo.yres * m_varScreenInfo.bits_per_pixel / 8;
        printf("FrameBuffer opened: %dx%d, %d bpp\n", m_varScreenInfo.xres, m_varScreenInfo.yres, m_varScreenInfo.bits_per_pixel);
        m_ptr = mmap(NULL, byteCount, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        if (m_ptr == MAP_FAILED)
        {
                return -1;
        }

        return 0;
}

int FrameBuffer::close()
{
        if (m_console_fd >= 0) {
                // Restore text mode
                ioctl(m_console_fd, KDSETMODE, KD_TEXT);
                
                // Show cursor again using VT100 escape sequence
                const char *show_cursor = "\033[?25h";
                write(m_console_fd, show_cursor, 6);

                const char *clear_screen = "\033[2J\033[H";
                write(m_console_fd, clear_screen, 7);

                ::close(m_console_fd);
                m_console_fd = -1;
        }

        if (m_ptr != NULL)
        {
                // Clear the framebuffer by setting it to black
                memset(m_ptr, 0, m_varScreenInfo.xres * m_varScreenInfo.yres * m_varScreenInfo.bits_per_pixel / 8);

                int byteCount = m_varScreenInfo.xres * m_varScreenInfo.yres * m_varScreenInfo.bits_per_pixel / 8;
                munmap(m_ptr, byteCount);
                m_ptr = NULL;
        }

        if (m_fd != 0 && -1 == ::close(m_fd))
        {
                handleErrorForClose(m_fd, errno);
        }

        m_fd = 0;
        return 0;
}

void FrameBuffer::fill()
{
        unsigned int width = m_varScreenInfo.xres;
        unsigned int height = m_varScreenInfo.yres;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height / threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId * threadHeight; y < ((threadId +1  < threadCount ) ?  ((threadId + 1) * threadHeight) : height) ; y++)
                {
#else
        for (unsigned int y = 0; y < height; y++)
        {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;

                        unsigned char pixelStep = 2;

                        for (unsigned int x = 0; x < width; x += pixelStep)
                        {
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                switch (bytesPerPixelFB)
                                {
                                default:
                                        *((unsigned long *)pixelFB) = 0x000000ff000000ff;
                                        break;
                                case 2:
                                        *((unsigned int *)pixelFB) = 0x001f001f;
                                        break;
                                }
                        }
                }
#if _OPENMP
        }
#endif
}

int FrameBuffer::show(const Image *image)
{
        switch (image->pixelformat())
        {
        case V4L2_PIX_FMT_GREY:
                print08(image);
                break;
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8:
                printDeBayer08(image);
                break;
        case V4L2_PIX_FMT_Y10:            
                printGrey10(image);
                break;
        case V4L2_PIX_FMT_Y10P:            
                printGrey10P(image);
                break;
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SBGGR10:
                // RAW10 shift Nano: 0, XavierNX: 5, TX2: 4
                print16(image, image->shift() + 2);
                break;
        case V4L2_PIX_FMT_Y12:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SBGGR12:
                // RAW12 shift Nano: 0, XavierNX: 4, TX2: 2
                print16(image, image->shift() + 4);
                break;
        case V4L2_PIX_FMT_SRGGB10P:
        case V4L2_PIX_FMT_SGBRG10P:
        case V4L2_PIX_FMT_SGRBG10P:
        case V4L2_PIX_FMT_SBGGR10P:
                {ImageData data = m_imageConvertCPU.convert10BitPackedBayerToRGB888(image,1);        
                break;}

        default:
                std::cout << "Not supported format" << std::endl;
                break;
         }
        
        return 0;
}

void FrameBuffer::printGrey10(const Image *image)
{
        {
                unsigned int width = (image->width() < m_varScreenInfo.xres) ? image->width() : m_varScreenInfo.xres;
                unsigned int height = (image->height() < m_varScreenInfo.yres) ? image->height() : m_varScreenInfo.yres;
                unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
                unsigned char *ptrFB = (unsigned char *)m_ptr;
                const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
                unsigned int threadCount = omp_get_num_procs();
                omp_set_num_threads(threadCount);
                unsigned int threadHeight = height / threadCount;

#pragma omp parallel
                {
                        int threadId = omp_get_thread_num();
                for (unsigned int y = threadId * threadHeight; y < ((threadId +1  < threadCount ) ?  ((threadId + 1) * threadHeight) : height) ; y++)
                        {
#else
                for (unsigned int y = 0; y < height; y++)
                {
#endif

                                unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                                unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                                unsigned int bytesPerPixelImage = 2;
                                unsigned char pixelStep = 4;

                                for (unsigned int x = 0; x < width / pixelStep; x += 1)
                                {
                                        unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                        const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                        unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep;
                                        unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                        switch (bytesPerPixelFB)
                                        {
                                        default:
                                                *((unsigned long *)pixelFB) = grey10BitToRGB888(*(uint16_t *)pixelImage);
                                                break;
                                        case 2:

                                                *((uint16_t *)pixelFB) = grey10BitToRGB565(*(uint16_t *)pixelImage);
                                                break;
                                        }
                                }
                        }
                }

#if _OPENMP
        }
#endif
}

uint16_t FrameBuffer::grey10BitToRGB565(const uint16_t &grey) const
{
        // Grey value to RGB565 conversion
        // Convert 10-bit (0-1023) to 5-bit (0-31) for red and blue
        unsigned int r5 = (grey >> 5) & 0x1F;
        unsigned int b5 = (grey >> 5) & 0x1F;

        // Convert 10-bit to 6-bit (0-63) for green
        unsigned int g6 = (grey >> 4) & 0x3F;

        // Pack into RGB565 format: RRRR RGGG GGGB BBBB
        return (r5 << 11) | (g6 << 5) | b5;
}
uint16_t FrameBuffer::grey8BitToRGB565(const char &grey) const
{
        // Grey value to RGB565 conversion
        // Convert 10-bit (0-1023) to 5-bit (0-31) for red and blue
        unsigned int r5 = (grey >> 3) & 0x1F;
        unsigned int b5 = (grey >> 3) & 0x1F;

        // Convert 10-bit to 6-bit (0-63) for green
        unsigned int g6 = (grey >> 2) & 0x3F;

        // Pack into RGB565 format: RRRR RGGG GGGB BBBB
        return (r5 << 11) | (g6 << 5) | b5;
}
unsigned long FrameBuffer::grey10BitToRGB888(const uint16_t &grey) const
{
        // Grey value to RGB888 conversion
        // Convert 10-bit (0-1023) to 8-bit (0-255)
        unsigned int grey8 = grey >> 2;

        // Pack into RGB888 format: RRRR RGGG GGGB BBBB
        return (grey8 << 16) | (grey8 << 8) | grey8;
}
unsigned long FrameBuffer::grey8BitToRGB888(const uint16_t &grey) const
{
        return (grey << 16) | (grey << 8) | grey;
}
void FrameBuffer::printGrey10P(const Image *image)
{
        unsigned int width = (image->width() < m_varScreenInfo.xres) ? image->width() : m_varScreenInfo.xres;
        unsigned int height = (image->height() < m_varScreenInfo.yres) ? image->height() : m_varScreenInfo.yres;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;
        const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height / threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId * threadHeight; y < ((threadId +1  < threadCount ) ?  ((threadId + 1) * threadHeight) : height) ; y++)
                {
#else
        for (unsigned int y = 0; y < height; y++)
        {
#endif

                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                        unsigned int bytesPerPixelImage = 5;
                        unsigned char pixelStep = 4;

                        for (unsigned int x = 0; x < width / pixelStep; x += 1)
                        {
                                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep;
                                unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                switch (bytesPerPixelFB)
                                {
                                default:
                                        *((unsigned long *)pixelFB) = grey8BitToRGB888(pixelImage[0]);
                                        *((unsigned long *)(pixelFB + bytesPerPixelFB)) = grey8BitToRGB888(pixelImage[1]);
                                        *((unsigned long *)(pixelFB + 2 * bytesPerPixelFB)) = grey8BitToRGB888(pixelImage[2]);
                                        *((unsigned long *)(pixelFB + 3 * bytesPerPixelFB)) = grey8BitToRGB888(pixelImage[3]);
                                        break;
                                case 2:

                                        *((uint16_t *)pixelFB) = grey8BitToRGB565(pixelImage[0]);
                                        *((uint16_t *)(pixelFB + 2)) = grey8BitToRGB565(pixelImage[1]);
                                        *((uint16_t *)(pixelFB + 4)) = grey8BitToRGB565(pixelImage[2]);
                                        *((uint16_t *)(pixelFB + 6)) = grey8BitToRGB565(pixelImage[3]);
                                        break;
                                }
                        }
                }

#if _OPENMP
        }
#endif
}

void FrameBuffer::printDeBayer08(const Image *image)
{
        unsigned int width = (image->width() < m_varScreenInfo.xres) ? image->width() : m_varScreenInfo.xres;
        unsigned int height = (image->height() < m_varScreenInfo.yres) ? image->height() : m_varScreenInfo.yres;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;
        const unsigned char *ptrImage = image->plane(0);

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height / threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId * threadHeight; y < ((threadId +1  < threadCount ) ?  ((threadId + 1) * threadHeight) : height) ; y++)
                {
#else
        for (unsigned int y = 0; y < height; y++)
        {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                        unsigned int bytesPerPixelImage = 1;
                        unsigned char pixelStep = 2;

                        for (unsigned int x = 0; x < width; x += pixelStep)
                        {
                                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                unsigned char r, g, b;
                                if (image->pixelformat() == V4L2_PIX_FMT_SRGGB8)
                                {
                                        // Bayer to RGB conversion for SRGGB8
                                        if ((y % 2 == 0) && (x % 2 == 0))
                                        {
                                                r = pixelImage[0];
                                                g = (pixelImage[1] + pixelImage[image->bytesPerLine()]) / 2;
                                                b = pixelImage[image->bytesPerLine() + 1];
                                        }
                                        else if ((y % 2 == 0) && (x % 2 == 1))
                                        {
                                                r = (pixelImage[0] + pixelImage[2]) / 2;
                                                g = pixelImage[1];
                                                b = (pixelImage[image->bytesPerLine()] + pixelImage[image->bytesPerLine() + 2]) / 2;
                                        }
                                        else if ((y % 2 == 1) && (x % 2 == 0))
                                        {
                                                r = (pixelImage[0] + pixelImage[2 * image->bytesPerLine()]) / 2;
                                                g = pixelImage[image->bytesPerLine()];
                                                b = (pixelImage[image->bytesPerLine() + 1] + pixelImage[2 * image->bytesPerLine() + 1]) / 2;
                                        }
                                        else
                                        {
                                                r = pixelImage[0];
                                                g = (pixelImage[1] + pixelImage[image->bytesPerLine()]) / 2;
                                                b = pixelImage[image->bytesPerLine() + 1];
                                        }
                                }
                                else
                                {
                                        r = g = b = pixelImage[0];
                                }

                                switch (bytesPerPixelFB)
                                {
                                default:
                                        *((unsigned long *)pixelFB) = (r << 16) | (g << 8) | b;
                                        break;
                                case 2:
                                        *((unsigned int *)pixelFB) = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                                        break;
                                }
                        }
                }
#if _OPENMP
        }
#endif
}

void FrameBuffer::print08(const Image *image)
{
        unsigned int width = (image->width() < m_varScreenInfo.xres) ? image->width() : m_varScreenInfo.xres;
        unsigned int height = (image->height() < m_varScreenInfo.yres) ? image->height() : m_varScreenInfo.yres - 1;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;
        const unsigned char *ptrImage = image->plane(0);

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height / threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId * threadHeight; y < ((threadId +1  < threadCount ) ?  ((threadId + 1) * threadHeight) : height) ; y++)
                {
#else
        for (unsigned int y = 0; y < height; y++)
        {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                        unsigned int bytesPerPixelImage = 1;
                        unsigned char pixelStep = 8;
                        unsigned long grey1 = 0, grey2 = 0, grey3 = 0, grey4 = 0;
                        unsigned long grey5 = 0, grey6 = 0, grey7 = 0, grey8 = 0;
                        unsigned long pixel12 = 0, pixel34 = 0;
                        unsigned long pixel56 = 0, pixel78 = 0;

                        // #pragma omp for
                        for (unsigned int x = 0; x < width; x += pixelStep)
                        {
                                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                unsigned long grey64 = *((unsigned long *)pixelImage);

                                grey1 = *((unsigned char *)&grey64 + 0);
                                grey2 = *((unsigned char *)&grey64 + 1);
                                grey3 = *((unsigned char *)&grey64 + 2);
                                grey4 = *((unsigned char *)&grey64 + 3);
                                grey5 = *((unsigned char *)&grey64 + 4);
                                grey6 = *((unsigned char *)&grey64 + 5);
                                grey7 = *((unsigned char *)&grey64 + 6);
                                grey8 = *((unsigned char *)&grey64 + 7);

                                pixel12 = (grey2 << 48) | (grey2 << 40) | (grey2 << 32) |
                                          (grey1 << 16) | (grey1 << 8) | (grey1 << 0);
                                pixel34 = (grey4 << 48) | (grey4 << 40) | (grey4 << 32) |
                                          (grey3 << 16) | (grey3 << 8) | (grey3 << 0);
                                pixel56 = (grey6 << 48) | (grey6 << 40) | (grey6 << 32) |
                                          (grey5 << 16) | (grey5 << 8) | (grey5 << 0);
                                pixel78 = (grey8 << 48) | (grey8 << 40) | (grey8 << 32) |
                                          (grey7 << 16) | (grey7 << 8) | (grey7 << 0);

                                *((unsigned long *)(pixelFB + 0)) = pixel12;
                                *((unsigned long *)(pixelFB + 8)) = pixel34;
                                *((unsigned long *)(pixelFB + 16)) = pixel56;
                                *((unsigned long *)(pixelFB + 24)) = pixel78;
                        }
                }
#if _OPENMP
        }
#endif
}

void FrameBuffer::print16(const Image *image, unsigned char shift)
{
        unsigned int width = (image->width() < m_varScreenInfo.xres) ? image->width() : m_varScreenInfo.xres;
        unsigned int height = (image->height() < m_varScreenInfo.yres) ? image->height() : m_varScreenInfo.yres;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;
        const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height / threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId * threadHeight; y < ((threadId +1  < threadCount ) ?  ((threadId + 1) * threadHeight) : height) ; y++)
                {
#else
        for (unsigned int y = 0; y < height; y++)
        {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                        unsigned int bytesPerPixelImage = 2;
                        unsigned char pixelStep = 4;
                        unsigned long grey1 = 0, grey2 = 0, grey3 = 0, grey4 = 0;
                        unsigned char pixelValue = 0;

                        // #pragma omp for
                        for (unsigned int x = 0; x < width; x += pixelStep)
                        {
                                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;
                                unsigned long pixel64_12 = 0, pixel64_34 = 0;
                                unsigned int pixel32_12 = 0, pixel32_34 = 0;

                                unsigned long grey64 = *((unsigned long *)pixelImage);
                                grey64 = grey64 >> shift;

                                grey1 = *((unsigned char *)&grey64 + 0);
                                grey2 = *((unsigned char *)&grey64 + 2);
                                grey3 = *((unsigned char *)&grey64 + 4);
                                grey4 = *((unsigned char *)&grey64 + 6);

                                switch (bytesPerPixelFB)
                                {
                                default:
                                case 3:
                                        pixel64_12 = (grey2 << 48) | (grey2 << 40) | (grey2 << 32) |
                                                     (grey1 << 16) | (grey1 << 8) | (grey1 << 0);
                                        pixel64_34 = (grey4 << 48) | (grey4 << 40) | (grey4 << 32) |
                                                     (grey3 << 16) | (grey3 << 8) | (grey3 << 0);

                                        *((unsigned long *)(pixelFB + 0)) = pixel64_12;
                                        *((unsigned long *)(pixelFB + 8)) = pixel64_34;
                                        break;
                                case 2:
                                        grey1 = (grey1 >> 3);
                                        grey2 = (grey2 >> 3);
                                        grey3 = (grey3 >> 3);
                                        grey4 = (grey4 >> 3);

                                        pixel32_12 = (grey2 << 27) | (grey2 << 22) | (grey2 << 16) |
                                                     (grey1 << 11) | (grey1 << 6) | (grey1 << 0);
                                        pixel32_34 = (grey4 << 27) | (grey4 << 22) | (grey4 << 16) |
                                                     (grey3 << 11) | (grey3 << 6) | (grey3 << 0);
                                        ;

                                        *((unsigned int *)(pixelFB + 0)) = pixel32_12;
                                        *((unsigned int *)(pixelFB + 4)) = pixel32_34;
                                        break;
                                }
                        }
                }
#if _OPENMP
        }
#endif
}

void FrameBuffer::handleErrorForOpen(const char *path, int err)
{
        char message[32];
        sprintf(message, "open (path: %s):", path);
        perror(message);
        printf("%s\n", errorsForOpen(err));
}

void FrameBuffer::handleErrorForClose(int fd, int err)
{
        char message[32];
        sprintf(message, "close (fd: %d):", fd);
        perror(message);
        printf("%s\n", errorsForClose(err));
}

void FrameBuffer::handleErrorForIoctl(unsigned long int request, int err)
{
        switch (request)
        {
        case FBIOGET_VSCREENINFO:
                perror("FBIOGET_VSCREENINFO");
                break;
        case FBIOGET_FSCREENINFO:
                perror("FBIOGET_FSCREENINFO");
                break;
        default:
                perror("Unknown ioctl request");
                break;
        }
        printf("%s\n", errorsForIoctl(request, err));
}

void FrameBuffer::writeTestImage()
{
        unsigned int width = m_varScreenInfo.xres;
        unsigned int height = m_varScreenInfo.yres;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel / 8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;

        printf("Writing test image %u\n", bytesPerPixelFB);

        for (unsigned int y = 0; y < height; y++)
        {
                for (unsigned int x = 0; x < width; x++)
                {
                        unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned char *pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                        // Create a simple test pattern (e.g., vertical stripes)
                        if ((x / 50) % 2 == 0)
                        {
                                switch (bytesPerPixelFB)
                                {
                                default:
                                        *((unsigned long *)pixelFB) = 0x00FF00FF; // Magenta
                                        break;
                                case 2:
                                        *((unsigned int *)pixelFB) = 0xF81F; // Magenta in 16-bit
                                        break;
                                }
                        }
                        else
                        {
                                switch (bytesPerPixelFB)
                                {
                                default:
                                        *((unsigned long *)pixelFB) = 0x000000FF; // Blue
                                        break;
                                case 2:
                                        *((unsigned int *)pixelFB) = 0x001F; // Blue in 16-bit
                                        break;
                                }
                        }
                }
        }
}

// Red:   187,  73,  15
// Green:  48, 202,  40
// Blue:   12,  32,  93
// White: 215, 236, 140
// Dark:    6,   8,   3

// Nach Kalibrierung
// RedScale = 215.0/237.0;
// BlueScale = 140.0/237.0;

// Red:   206,  73,  25
// Green:  52, 202,  67
// Blue:   12,  32, 157
// White: 237, 236, 237

// float RedScale = 1.0, GreenScale = 1.0, BlueScale = 1.0;
// unsigned long AvgRed = 0, AvgGreen = 0, AvgBlue = 0;
// unsigned short ScaledGrey = 0;
// unsigned int Pixel = 0;
// unsigned char YSelect = y%2;
// unsigned char XSelect = x%2;
// if (YSelect == 0) {
//         if (XSelect == 0) {     // Red
//                 ScaledGrey = Grey / RedScale;
//                 AvgRed += ScaledGrey;
//                 if (ScaledGrey > 255)
//                         Pixel = (255 << 16);
//                 else
//                         Pixel = (ScaledGrey << 16);
//         } else {                // Green
//                 ScaledGrey = Grey / GreenScale;
//                 AvgGreen += ScaledGrey;
//                 if (ScaledGrey > 255)
//                         Pixel = (255 << 8);
//                 else
//                         Pixel = (ScaledGrey << 8);
//         }
// } else {
//         if (XSelect == 0) {     // Green
//                 ScaledGrey = Grey / GreenScale;
//                 AvgGreen += ScaledGrey;
//                 if (ScaledGrey > 255)
//                         Pixel = (255 << 8);
//                 else
//                         Pixel = (ScaledGrey << 8);
//         } else {                // Blue
//                 ScaledGrey = Grey / BlueScale;
//                 AvgBlue += ScaledGrey;
//                 if (ScaledGrey > 255)
//                         Pixel = (255 << 0);
//                 else
//                         Pixel = (ScaledGrey << 0);
//         }
// }

// int Size = width * height;
// AvgRed /= Size / 4;
// AvgGreen /= Size / 2;
// AvgBlue /= Size / 4;

// printf("Color: (%llu, %llu, %llu)\n", AvgRed, AvgGreen, AvgBlue);