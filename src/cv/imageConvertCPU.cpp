
#include <cv/imageConvertCPU.hpp>
#include <cstdint>
#include <omp.h>
#include <iostream>
#include <linux/videodev2.h>

uint16_t ImageConvertCPU::grey10BitToRGB565(const uint16_t &grey) const
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
uint16_t ImageConvertCPU::grey8BitToRGB565(const char &grey) const
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
unsigned long ImageConvertCPU::grey10BitToRGB888(const uint16_t &grey) const
{
    // Grey value to RGB888 conversion
    // Convert 10-bit (0-1023) to 8-bit (0-255)
    unsigned int grey8 = grey >> 8;

    // Pack into RGB888 format: RRRR RGGG GGGB BBBB
    return (grey8 << 16) | (grey8 << 8) | grey8;
}
unsigned long ImageConvertCPU::grey8BitToRGB888(const uint16_t &grey) const
{
    return (grey << 16) | (grey << 8) | grey;
}

ImageData ImageConvertCPU::convert10BitGreyToRGB888(const Image *image) const

{
    const unsigned int bytesPerPixelFB = 3;
    size_t dataSize = image->height() * image->width() * bytesPerPixelFB;
    unsigned char *data = new unsigned char[dataSize];

    const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
    unsigned int threadCount = omp_get_num_procs();
    omp_set_num_threads(threadCount);
    unsigned int threadHeight = image->height() / threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : image->height()); y++)
        {
#else
    for (unsigned int y = 0; y < image->height(); y++)
    {
#endif

            unsigned int yOffsetPtrFB = y * image->width();
            unsigned int YOffsetPtrImage = y * image->bytesPerLine();

            unsigned int bytesPerPixelImage = 2;
            unsigned char pixelStep = 4;

            for (unsigned int x = 0; x < image->width() / pixelStep; x += 1)
            {
                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep;
                unsigned char *pixelFB = data + yOffsetPtrFB + xOffsetPtrFB;

                switch (bytesPerPixelFB)
                {
                default:
                    *((unsigned long *)pixelFB) = grey10BitToRGB888(*(uint16_t *)pixelImage);
                    break;
                }
            }
        }

#if _OPENMP
    }
#endif
    ImageData result;
    result.data = data;
    result.size = dataSize;
    return result;
}

ImageData ImageConvertCPU::convert10BitPackedGreyToRGB888(const Image *image) const

{
    const unsigned int bytesPerPixelFB = 3;
    const u_int16_t width = image->width();
    const u_int16_t height = image->height();
    const u_int16_t bytesPerLine = image->bytesPerLine();
    size_t dataSize = image->height() * image->width() * bytesPerPixelFB;
    unsigned char *data = new unsigned char[dataSize];

    const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
    unsigned int threadCount = omp_get_num_procs();
    omp_set_num_threads(threadCount);
    unsigned int threadHeight = height / threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : height); y++)
        {

#else
    for (unsigned int y = 0; y < image->height(); y++)
    {
#endif

            unsigned int yOffsetPtrFB = y * width * bytesPerPixelFB;
            unsigned int YOffsetPtrImage = y * bytesPerLine;

            unsigned int bytesPerPixelImage = 5;
            unsigned char pixelStep = 4;

            for (unsigned int x = 0; x < width / pixelStep; x += 1)
            {
                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep;
                unsigned char *pixelFB = data + yOffsetPtrFB + xOffsetPtrFB;

                *((unsigned long *)pixelFB) = grey8BitToRGB888(pixelImage[0]);
                *((unsigned long *)(pixelFB + bytesPerPixelFB)) = grey8BitToRGB888(pixelImage[1]);
                *((unsigned long *)(pixelFB + 2 * bytesPerPixelFB)) = grey8BitToRGB888(pixelImage[2]);
                *((unsigned long *)(pixelFB + 3 * bytesPerPixelFB)) = grey8BitToRGB888(pixelImage[3]);
            }
        }

#if _OPENMP
    }
#endif
    ImageData result;
    result.data = data;
    result.size = dataSize;
    return result;
}

ImageData ImageConvertCPU::convert8BitBayerToRGB888(const Image *image) const
{
    const unsigned int bytesPerPixelFB = 3;
    const unsigned int bytesPerPixelImage = 1;
    const unsigned char pixelStep = 1; // downscaling > 1
    const u_int16_t width = image->width();
    const u_int16_t height = image->height();
    const u_int16_t bytesPerLine = image->bytesPerLine();
    size_t dataSize = image->height() * image->width() * bytesPerPixelFB;
    unsigned char *data = new unsigned char[dataSize];

    const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
    unsigned int threadCount = omp_get_num_procs();
    omp_set_num_threads(threadCount);
    unsigned int threadHeight = height / threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : height); y += pixelStep)
        {

#else
    for (unsigned int y = 0; y < image->height(); y++)
    {
#endif

            unsigned int yOffsetPtrFB = y * width * bytesPerPixelFB;
            unsigned int YOffsetPtrImage = y * bytesPerLine;

            for (unsigned int x = 0; x < width / pixelStep; x += pixelStep)
            {
                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep;
                unsigned char *pixelFB = data + yOffsetPtrFB + xOffsetPtrFB;

                unsigned char r = 0, g = 0, b = 0;
                if (image->pixelformat() == V4L2_PIX_FMT_SRGGB8)
                {
                    // Bayer to RGB conversion for SRGGB8
                    if ((y % 2 == 0) && (x % 2 == 0))
                    {
                        r = pixelImage[0];
                        g = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        b = pixelImage[bytesPerLine + 1];
                    }
                    else if ((y % 2 == 0) && (x % 2 == 1))
                    {
                        r = (pixelImage[0] + pixelImage[2]) / 2;
                        g = (pixelImage[0] + pixelImage[bytesPerLine + 1]) / 2;
                        b = pixelImage[bytesPerLine];
                    }
                    else if ((y % 2 == 1) && (x % 2 == 0))
                    {
                        r = pixelImage[bytesPerLine];
                        g = (pixelImage[0] + pixelImage[bytesPerLine + 1]) / 2;
                        b = pixelImage[1];
                    }
                    else
                    {
                        r = pixelImage[bytesPerLine + 1];
                        g = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        b = pixelImage[0];
                    }
                }
                else if (image->pixelformat() == V4L2_PIX_FMT_SGBRG8)
                {
                    // Bayer to RGB conversion for SGBRG8
                    if ((y % 2 == 0) && (x % 2 == 0))
                    {
                        g = (pixelImage[0] + pixelImage[bytesPerLine + 1]) / 2;
                        r = pixelImage[bytesPerLine];
                        b = pixelImage[1];
                    }
                    else if ((y % 2 == 0) && (x % 2 == 1))
                    {
                        g = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        r = pixelImage[bytesPerLine + 1];
                        b = pixelImage[0];
                    }
                    else if ((y % 2 == 1) && (x % 2 == 0))
                    {
                        g = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        r = pixelImage[0];
                        b = pixelImage[bytesPerLine + 1];
                    }
                    else
                    {
                        g = (pixelImage[0] + pixelImage[bytesPerLine + 1]) / 2;
                        r = pixelImage[1];
                        b = pixelImage[bytesPerLine];
                    }
                }

                pixelFB[0] = b;
                pixelFB[1] = g;
                pixelFB[2] = r;
            }
        }

#if _OPENMP
    }
#endif
    ImageData result;
    result.data = data;
    result.size = dataSize;
    return result;
}

ImageData ImageConvertCPU::convert10BitPackedBayerToRGB888(const Image *image) const
{
    const unsigned int bytesPerPixelFB = 3;
    const u_int16_t width = image->width();
    const u_int16_t height = image->height();
    const u_int16_t bytesPerLine = image->bytesPerLine();
    size_t dataSize = image->height() * image->width() * bytesPerPixelFB;
    unsigned char *data = new unsigned char[dataSize];

    const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
    unsigned int threadCount = omp_get_num_procs();
    omp_set_num_threads(threadCount);
    unsigned int threadHeight = height / threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : height); y++)
        {

#else
    for (unsigned int y = 0; y < image->height(); y++)
    {
#endif

            unsigned int yOffsetPtrFB = y * width * bytesPerPixelFB;
            unsigned int YOffsetPtrImage = y * bytesPerLine;

            unsigned int bytesPerPixelImage = 5;
            unsigned char pixelStep = 4;

            for (unsigned int x = 0; x < width / pixelStep; x += 1)
            {
                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep;
                unsigned char *pixelFB = data + yOffsetPtrFB + xOffsetPtrFB;

                unsigned char r, g, b;
                if (image->pixelformat() == V4L2_PIX_FMT_SRGGB10P)
                {
                    // Bayer to RGB conversion for SRGGB8
                    if ((y % 2 == 0) && (x % 2 == 0))
                    {
                        r = pixelImage[0];
                        g = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        b = pixelImage[bytesPerLine + 1];
                    }
                    else if ((y % 2 == 0) && (x % 2 == 1))
                    {
                        r = (pixelImage[0] + pixelImage[2]) / 2;
                        g = pixelImage[1];
                        b = (pixelImage[bytesPerLine] + pixelImage[bytesPerLine + 2]) / 2;
                    }
                    else if ((y % 2 == 1) && (x % 2 == 0))
                    {
                        r = (pixelImage[0] + pixelImage[2 * bytesPerLine]) / 2;
                        g = pixelImage[bytesPerLine];
                        b = (pixelImage[bytesPerLine + 1] + pixelImage[2 * bytesPerLine + 1]) / 2;
                    }
                    else
                    {
                        r = pixelImage[0];
                        g = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        b = pixelImage[bytesPerLine + 1];
                    }
                }
                else if (image->pixelformat() == V4L2_PIX_FMT_SGBRG10P)
                {
                    // Bayer to RGB conversion for SGBRG8
                    if ((y % 2 == 0) && (x % 2 == 0))
                    {
                        g = pixelImage[0];
                        r = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        b = pixelImage[bytesPerLine + 1];
                    }
                    else if ((y % 2 == 0) && (x % 2 == 1))
                    {
                        g = (pixelImage[0] + pixelImage[2]) / 2;
                        r = pixelImage[1];
                        b = (pixelImage[bytesPerLine] + pixelImage[bytesPerLine + 2]) / 2;
                    }
                    else if ((y % 2 == 1) && (x % 2 == 0))
                    {
                        g = (pixelImage[0] + pixelImage[2 * bytesPerLine]) / 2;
                        r = pixelImage[bytesPerLine];
                        b = (pixelImage[bytesPerLine + 1] + pixelImage[2 * bytesPerLine + 1]) / 2;
                    }
                    else
                    {
                        g = pixelImage[0];
                        r = (pixelImage[1] + pixelImage[bytesPerLine]) / 2;
                        b = pixelImage[bytesPerLine + 1];
                    }
                    *((unsigned long *)pixelFB) = r << 16 | g << 8 | b;
                    *((unsigned long *)(pixelFB + bytesPerPixelFB)) = r << 16 | g << 8 | b;
                    *((unsigned long *)(pixelFB + 2 * bytesPerPixelFB)) = r << 16 | g << 8 | b;
                    *((unsigned long *)(pixelFB + 3 * bytesPerPixelFB)) = r << 16 | g << 8 | b;
                }
            }
        }

#if _OPENMP
    }
#endif
    ImageData result;
    result.data = data;
    result.size = dataSize;
    return result;
}
