
#include <cv/imageConvertCPU.hpp>
#include <cstdint>
#include <omp.h>
#include <iostream>

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
    unsigned int threadHeight = height/ threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : height); y++)
        {

#else
    for (unsigned int y = 0; y < image->height(); y++)
    {
#endif

            unsigned int yOffsetPtrFB = y * width *bytesPerPixelFB;
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
