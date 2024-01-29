#include "framebuffer.hpp"
#include <errno.hpp>
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
        u_int32_t width = m_varScreenInfo.xres;
        u_int32_t height = m_varScreenInfo.yres;
        u_int32_t bytesPerPixelFB = m_varScreenInfo.bits_per_pixel/8;
        u_int8_t * ptrFB = (u_int8_t *)m_ptr;

#if _OPENMP        
        u_int32_t threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        u_int32_t threadHeight = height/threadCount;

#pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (u_int32_t y = threadId*threadHeight; y < (threadId+1)*threadHeight; y++) {
#else
                for (u_int32_t y = 0; y < height; y++) {
#endif
                        u_int32_t yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB
                                + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        
                        u_int8_t pixelStep = 2;
                        
                        for (u_int32_t x = 0; x < width; x+=pixelStep) {
                                u_int32_t xOffsetPtrFB = x * bytesPerPixelFB;
                                u_int8_t * pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                switch (bytesPerPixelFB) {
                                default:
                                        *((u_int64_t *)pixelFB) = 0x000000ff000000ff;
                                        break;
                                case 2:
                                        *((u_int32_t *)pixelFB) = 0x001f001f;
                                        break;
                                }
                        }
                }
#if _OPENMP
        }
#endif
}

void FrameBuffer::print(V4L2Image &image)
{
        u_int8_t shift = image.shift();
        if (shift < 0) {
                shift = 0;
        } 
        if (shift > 16) {
                shift = 16;
        }

        switch (image.pixelformat()) {
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
                print16(image, shift + 2); 
                break;
        case V4L2_PIX_FMT_Y12:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SBGGR12:
                // RAW12 shift Nano: 0, XavierNX: 4, TX2: 2
                print16(image, shift + 4); 
                break;
        }
}

void FrameBuffer::print08(V4L2Image &image)
{
        u_int32_t width = (image.width() < m_varScreenInfo.xres) ? image.width() : m_varScreenInfo.xres;
        u_int32_t height = (image.height() < m_varScreenInfo.yres) ? image.height() : m_varScreenInfo.yres-1;
        u_int32_t bytesPerPixelFB = m_varScreenInfo.bits_per_pixel/8;
        u_int8_t * ptrFB = (u_int8_t *)m_ptr;
        u_int8_t * ptrImage = (u_int8_t *)image.m_planes.at(0);

#if _OPENMP
        u_int32_t threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        u_int32_t threadHeight = height/threadCount;

        #pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (u_int32_t y = threadId*threadHeight; y < (threadId+1)*threadHeight; y++) {
#else
                for (u_int32_t y = 0; y < height; y++) {
#endif
                        u_int32_t yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB
                                + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        u_int32_t YOffsetPtrImage = (y * image.bytesPerLine());

                        u_int32_t bytesPerPixelImage = 1;
                        u_int8_t pixelStep = 8;
                        u_int64_t grey1 = 0, grey2 = 0, grey3 = 0, grey4 = 0;
                        u_int64_t grey5 = 0, grey6 = 0, grey7 = 0, grey8 = 0;
                        u_int64_t pixel12 = 0, pixel34 = 0;
                        u_int64_t pixel56 = 0, pixel78 = 0;

                        // #pragma omp for
                        for (u_int32_t x = 0; x < width; x+=pixelStep) {
                                u_int32_t xOffsetPtrImage = x * bytesPerPixelImage;
                                u_int8_t * pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                u_int32_t xOffsetPtrFB = x * bytesPerPixelFB;
                                u_int8_t * pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;

                                u_int64_t grey64 = *((u_int64_t *)pixelImage);

                                grey1 = *((u_int8_t *)&grey64 + 0);
                                grey2 = *((u_int8_t *)&grey64 + 1);
                                grey3 = *((u_int8_t *)&grey64 + 2);
                                grey4 = *((u_int8_t *)&grey64 + 3);
                                grey5 = *((u_int8_t *)&grey64 + 4);
                                grey6 = *((u_int8_t *)&grey64 + 5);
                                grey7 = *((u_int8_t *)&grey64 + 6);
                                grey8 = *((u_int8_t *)&grey64 + 7);

                                pixel12 = (grey2 << 48) | (grey2 << 40) | (grey2 << 32) |
                                         (grey1 << 16) | (grey1 <<  8) | (grey1 <<  0);
                                pixel34 = (grey4 << 48) | (grey4 << 40) | (grey4 << 32) |
                                        (grey3 << 16) | (grey3 <<  8) | (grey3 <<  0);
                                pixel56 = (grey6 << 48) | (grey6 << 40) | (grey6 << 32) |
                                        (grey5 << 16) | (grey5 <<  8) | (grey5 <<  0);
                                pixel78 = (grey8 << 48) | (grey8 << 40) | (grey8 << 32) |
                                        (grey7 << 16) | (grey7 <<  8) | (grey7 <<  0);

                                *((u_int64_t *)(pixelFB +  0)) = pixel12;
                                *((u_int64_t *)(pixelFB +  8)) = pixel34;
                                *((u_int64_t *)(pixelFB + 16)) = pixel56;
                                *((u_int64_t *)(pixelFB + 24)) = pixel78;
                        }
                }
#if _OPENMP
        }
#endif
}

void FrameBuffer::print16(V4L2Image &image, u_int8_t shift)
{
        u_int32_t width = (image.width() < m_varScreenInfo.xres) ? image.width() : m_varScreenInfo.xres;
        u_int32_t height = (image.height() < m_varScreenInfo.yres) ? image.height() : m_varScreenInfo.yres;
        u_int32_t bytesPerPixelFB = m_varScreenInfo.bits_per_pixel/8;
        u_int8_t * ptrFB = (u_int8_t *)m_ptr;
        u_int8_t * ptrImage = (u_int8_t *)image.m_planes.at(0);

#if _OPENMP
        u_int32_t threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        u_int32_t threadHeight = height/threadCount;

        #pragma omp parallel
        {
                int threadId = omp_get_thread_num();
                for (u_int32_t y = threadId*threadHeight; y < (threadId+1)*threadHeight; y++) {
#else
                for (u_int32_t y = 0; y < height; y++) {
#endif
                        u_int32_t yOffsetPtrFB = m_varScreenInfo.xoffset * bytesPerPixelFB
                                + (y + m_varScreenInfo.yoffset) * m_fixScreenInfo.line_length;
                        u_int32_t YOffsetPtrImage = (y * image.bytesPerLine());

                        u_int32_t bytesPerPixelImage = 2;
                        u_int8_t pixelStep = 4;
                        u_int64_t grey1 = 0, grey2 = 0, grey3 = 0, grey4 = 0;
                        u_int8_t pixelValue = 0;

                        // #pragma omp for
                        for (u_int32_t x = 0; x < width; x+=pixelStep) {
                                u_int32_t xOffsetPtrImage = x * bytesPerPixelImage;
                                u_int8_t * pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                                u_int32_t xOffsetPtrFB = x * bytesPerPixelFB;
                                u_int8_t * pixelFB = ptrFB + yOffsetPtrFB + xOffsetPtrFB;
                                u_int64_t pixel64_12 = 0, pixel64_34 = 0;
                                u_int32_t pixel32_12 = 0, pixel32_34 = 0;

                                u_int64_t grey64 = *((u_int64_t *)pixelImage);
                                grey64 = grey64 >> shift;

                                grey1 = *((u_int8_t *)&grey64 + 0);
                                grey2 = *((u_int8_t *)&grey64 + 2);
                                grey3 = *((u_int8_t *)&grey64 + 4);
                                grey4 = *((u_int8_t *)&grey64 + 6);

                                switch (bytesPerPixelFB) {
                                default:
                                case 3:
                                        pixel64_12 = (grey2 << 48) | (grey2 << 40) | (grey2 << 32) |
                                                (grey1 << 16) | (grey1 <<  8) | (grey1 <<  0);
                                        pixel64_34 = (grey4 << 48) | (grey4 << 40) | (grey4 << 32) |
                                                (grey3 << 16) | (grey3 <<  8) | (grey3 <<  0);

                                        *((u_int64_t *)(pixelFB + 0)) = pixel64_12;
                                        *((u_int64_t *)(pixelFB + 8)) = pixel64_34;
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

                                        *((u_int32_t *)(pixelFB + 0)) = pixel32_12;
                                        *((u_int32_t *)(pixelFB + 4)) = pixel32_34;
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
// u_int64_t AvgRed = 0, AvgGreen = 0, AvgBlue = 0;
// unsigned short ScaledGrey = 0;
// u_int32_t Pixel = 0;
// u_int8_t YSelect = y%2;
// u_int8_t XSelect = x%2;
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