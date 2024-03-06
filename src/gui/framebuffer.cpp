#include <gui/framebuffer.hpp>
#include <utils/errno.hpp>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <omp.h>


FrameBuffer::FrameBuffer() :
        m_fd(0),
        m_ptr(NULL)
{
}

FrameBuffer::~FrameBuffer()
{
        close();
}

int FrameBuffer::open()
{
        const char *DevicePath = "/dev/fb0";
        m_fd = ::open(DevicePath, O_RDWR);
        if (-1 == m_fd) {
                handleErrorForOpen(DevicePath, errno);
                return -1;
        }

        if (-1 == ioctl(m_fd, FBIOGET_VSCREENINFO, &m_varScreenInfo)) {
                handleErrorForIoctl(FBIOGET_VSCREENINFO, errno);
                return -1;
        }

        if (-1 == ioctl(m_fd, FBIOGET_FSCREENINFO, &m_fixScreenInfo)) {
                handleErrorForIoctl(FBIOGET_FSCREENINFO, errno);
                return -1;
        }

        int byteCount = m_varScreenInfo.xres * m_varScreenInfo.yres * m_varScreenInfo.bits_per_pixel / 8;
        m_ptr = mmap(NULL, byteCount, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        if (m_ptr == MAP_FAILED) {
                return -1;
        }

        return 0;
}

int FrameBuffer::close()
{
        if (m_fd != 0 && -1 == ::close(m_fd)) {
                handleErrorForClose(m_fd, errno);
        }

        m_fd = 0;
        return 0;
}

void FrameBuffer::fill()
{
        unsigned int width = m_varScreenInfo.xres;
        unsigned int height = m_varScreenInfo.yres;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel/8;
        unsigned char * ptrFB = (unsigned char *)m_ptr;

#if _OPENMP        
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height/threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId*threadHeight; y < (threadId+1)*threadHeight; y++) {
#else
                for (unsigned int y = 0; y < height; y++) {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB
                                + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        
                        unsigned char pixelStep = 2;
                        
                        for (unsigned int x = 0; x < width; x+=pixelStep) {
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char * pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                switch (bytesPerPixelFB) {
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

void FrameBuffer::update(const Image *image)
{
        switch (image->pixelformat()) {
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SBGGR8: 
                print08(image);
                break;
        case V4L2_PIX_FMT_Y10:
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
        }
}

void FrameBuffer::print08(const Image *image)
{
        unsigned int width = (image->width() < m_varScreenInfo.xres) ? image->width() : m_varScreenInfo.xres;
        unsigned int height = (image->height() < m_varScreenInfo.yres) ? image->height() : m_varScreenInfo.yres-1;
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel/8;
        unsigned char *ptrFB = (unsigned char *)m_ptr;
        const unsigned char *ptrImage = image->plane(0);

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height/threadCount;

        #pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId*threadHeight; y < (threadId+1)*threadHeight; y++) {
#else
                for (unsigned int y = 0; y < height; y++) {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB
                                + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                        unsigned int bytesPerPixelImage = 1;
                        unsigned char pixelStep = 8;
                        unsigned long grey1 = 0, grey2 = 0, grey3 = 0, grey4 = 0;
                        unsigned long grey5 = 0, grey6 = 0, grey7 = 0, grey8 = 0;
                        unsigned long pixel12 = 0, pixel34 = 0;
                        unsigned long pixel56 = 0, pixel78 = 0;

                        // #pragma omp for
                        for (unsigned int x = 0; x < width; x+=pixelStep) {
                                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                const unsigned char * pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char * pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

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
                                         (grey1 << 16) | (grey1 <<  8) | (grey1 <<  0);
                                pixel34 = (grey4 << 48) | (grey4 << 40) | (grey4 << 32) |
                                        (grey3 << 16) | (grey3 <<  8) | (grey3 <<  0);
                                pixel56 = (grey6 << 48) | (grey6 << 40) | (grey6 << 32) |
                                        (grey5 << 16) | (grey5 <<  8) | (grey5 <<  0);
                                pixel78 = (grey8 << 48) | (grey8 << 40) | (grey8 << 32) |
                                        (grey7 << 16) | (grey7 <<  8) | (grey7 <<  0);

                                *((unsigned long *)(pixelFB +  0)) = pixel12;
                                *((unsigned long *)(pixelFB +  8)) = pixel34;
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
        unsigned int bytesPerPixelFB = m_varScreenInfo.bits_per_pixel/8;
        unsigned char * ptrFB = (unsigned char *)m_ptr;
        const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = height/threadCount;

        #pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (unsigned int y = threadId*threadHeight; y < (threadId+1)*threadHeight; y++) {
#else
                for (unsigned int y = 0; y < height; y++) {
#endif
                        unsigned int yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB
                                + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        unsigned int YOffsetPtrImage = (y * image->bytesPerLine());

                        unsigned int bytesPerPixelImage = 2;
                        unsigned char pixelStep = 4;
                        unsigned long grey1 = 0, grey2 = 0, grey3 = 0, grey4 = 0;
                        unsigned char pixelValue = 0;

                        // #pragma omp for
                        for (unsigned int x = 0; x < width; x+=pixelStep) {
                                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                                const unsigned char * pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                                unsigned char * pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;
                                unsigned long pixel64_12 = 0, pixel64_34 = 0;
                                unsigned int pixel32_12 = 0, pixel32_34 = 0;

                                unsigned long grey64 = *((unsigned long *)pixelImage);
                                grey64 = grey64 >> shift;

                                grey1 = *((unsigned char *)&grey64 + 0);
                                grey2 = *((unsigned char *)&grey64 + 2);
                                grey3 = *((unsigned char *)&grey64 + 4);
                                grey4 = *((unsigned char *)&grey64 + 6);

                                switch (bytesPerPixelFB) {
                                default:
                                case 3:
                                        pixel64_12 = (grey2 << 48) | (grey2 << 40) | (grey2 << 32) |
                                                (grey1 << 16) | (grey1 <<  8) | (grey1 <<  0);
                                        pixel64_34 = (grey4 << 48) | (grey4 << 40) | (grey4 << 32) |
                                                (grey3 << 16) | (grey3 <<  8) | (grey3 <<  0);

                                        *((unsigned long *)(pixelFB + 0)) = pixel64_12;
                                        *((unsigned long *)(pixelFB + 8)) = pixel64_34;
                                        break;
                                case 2:
                                        grey1 = (grey1 >> 3);
                                        grey2 = (grey2 >> 3);
                                        grey3 = (grey3 >> 3);
                                        grey4 = (grey4 >> 3);

                                        pixel32_12 = (grey2 << 27) | (grey2 << 22) | (grey2 << 16) |
                                                (grey1 << 11) | (grey1 <<  6) | (grey1 <<  0);
                                        pixel32_34 =  (grey4 << 27) | (grey4 << 22) | (grey4 << 16) |
                                                (grey3 << 11) | (grey3 <<  6) | (grey3 <<  0);;

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
        switch (request) {
        case FBIOGET_VSCREENINFO: perror("FBIOGET_VSCREENINFO");   break;
        case FBIOGET_FSCREENINFO: perror("FBIOGET_FSCREENINFO");   break;
        default:                  perror("Unknown ioctl request"); break;
        }
        printf("%s\n", errorsForIoctl(request, err));
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