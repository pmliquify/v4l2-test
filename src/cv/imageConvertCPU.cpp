
#include <cv/imageConvertCPU.hpp>
#include <cstdint>
#if _OPENMP
#include <omp.h>
#endif
#include <iostream>
#include <linux/videodev2.h>
#include <exception>
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

ImageData ImageConvertCPU::convert10BitGreyToRGB888(const Image *image, int scaleFactor) const

{
    unsigned int bytesPerPixelImage = 2;
    if (!(scaleFactor > 0))
        throw std::out_of_range("ScaleFactor must be bigger 0");
    const unsigned char pixelStep = scaleFactor;
    const unsigned int bytesPerPixelFB = 3;
    size_t targetWidth = image->width() / scaleFactor;
    size_t targetHeight = image->height() / scaleFactor;
    size_t dataSize = targetWidth * targetHeight * bytesPerPixelFB; //(size_t)pixelStep;
    std::cout << "Data size: " << dataSize << std::endl;
    unsigned char *data = new unsigned char[dataSize];

    const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
    unsigned int threadCount = omp_get_num_procs();
    omp_set_num_threads(threadCount);
    unsigned int threadHeight = targetHeight / threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : targetHeight); y += 1)
        {
#else
    for (unsigned int y = 0; y < targetHeight; y += 1)
    {
#endif

            unsigned int yOffsetPtrFB = y * targetWidth * bytesPerPixelFB;
            unsigned int YOffsetPtrImage = y * image->bytesPerLine() * pixelStep;

            for (unsigned int x = 0; x < targetWidth; x += 1)
            {

                unsigned int xOffsetPtrImage = x * bytesPerPixelImage * pixelStep;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
                unsigned char *pixelFB = data + yOffsetPtrFB + xOffsetPtrFB;

                std::cout << "X: " << x << ", Y: " << y << ", Offset: " << yOffsetPtrFB + xOffsetPtrFB << std::endl;

                *((unsigned long *)pixelFB) = grey10BitToRGB888(*(uint16_t *)pixelImage);
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

ImageData ImageConvertCPU::convert10BitPackedGreyToRGB888(const Image *image, int scaleFactor) const

{
    const unsigned int bytesPerPixelFB = 3;
    if (!(scaleFactor > 0))
        throw std::out_of_range("ScaleFactor must be bigger 0");
    const unsigned char pixelStep = 5;
    const unsigned int bytesPerPixelImage = 4;

    size_t targetWidth = image->width() / scaleFactor;
    size_t targetHeight = image->height() / scaleFactor;
    const u_int16_t bytesPerLine = image->bytesPerLine();
    size_t dataSize = targetWidth * targetHeight * bytesPerPixelFB;
    unsigned char *data = new unsigned char[dataSize];

    for (int i = 0; i < dataSize; i++)
    {
        data[i] = 111;
    }

    const unsigned char *ptrImage = image->planes()[0];

#if _OPENMP
    unsigned int threadCount = omp_get_num_procs();
    omp_set_num_threads(threadCount);
    unsigned int threadHeight = targetHeight / threadCount;

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        for (unsigned int y = threadId * threadHeight; y < ((threadId + 1 < threadCount) ? ((threadId + 1) * threadHeight) : targetHeight); y += 1)
        {

#else
    for (unsigned int y = 0; y < targetHeight; y += 1)
    {
#endif

            unsigned int yOffsetPtrFB = y * targetWidth * bytesPerPixelFB;
            unsigned int YOffsetPtrImage = y * bytesPerLine * scaleFactor;

            for (double x = 0; x < targetWidth; x += bytesPerPixelImage)
            {
                unsigned int xOffsetPtrImage = x * scaleFactor * pixelStep / bytesPerPixelImage;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB;
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

ImageData ImageConvertCPU::convert8BitBayerToRGB888(const Image *image, int scaleFactor) const
{
    const unsigned int bytesPerPixelFB = 3;
    const unsigned int bytesPerPixelImage = 1;
    const unsigned char pixelStep = 1; // downscaling > 1
    const u_int16_t width = image->width();
    const u_int16_t height = image->height();
    const u_int16_t bytesPerLine = image->bytesPerLine();
    size_t dataSize = image->height() * image->width() * bytesPerPixelFB / (size_t)pixelStep / (size_t)pixelStep;
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
    for (unsigned int y = 0; y < image->height(); y += pixelStep)
    {
#endif

            unsigned int yOffsetPtrFB = y * width * bytesPerPixelFB / pixelStep;
            unsigned int YOffsetPtrImage = y * bytesPerLine;

            for (unsigned int x = 0; x < width / pixelStep; x += pixelStep)
            {
                unsigned int xOffsetPtrImage = x * bytesPerPixelImage;
                const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
                unsigned int xOffsetPtrFB = x * bytesPerPixelFB * pixelStep / pixelStep;
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

ImageData ImageConvertCPU::convert10BitPackedBayerToRGB888(const Image *image, int scaleFactor) const
{
    const unsigned int bytesPerPixelFB = 3;
    const u_int16_t width = image->width();
    const u_int16_t height = image->height();
    const unsigned int pixelStep = 4; // 4 pixels per 5 bytes
    const unsigned int bytesPerPixelImage = 5;
    const u_int16_t bytesPerLine = image->bytesPerLine();
    size_t dataSize = image->height() * image->width() * bytesPerPixelFB;
    unsigned char *data = new unsigned char[dataSize];
    const unsigned char *ptrImage = image->planes()[0];

    // Step 1: Unpack the Bayer image into a 2D array of 10-bit values
    std::vector<std::vector<uint16_t>> bayer(height, std::vector<uint16_t>(width, 0));
    for (unsigned int y = 0; y < height; ++y) {
        unsigned int YOffsetPtrImage = y * bytesPerLine;
        for (unsigned int x = 0; x < width; x += pixelStep) {
            unsigned int xOffsetPtrImage = (x / pixelStep) * bytesPerPixelImage;
            const unsigned char *pixelImage = ptrImage + YOffsetPtrImage + xOffsetPtrImage;
            uint16_t unpacked[4];
            unpacked[0] = ((pixelImage[0] << 2) | ((pixelImage[4] >> 0) & 0x3));
            unpacked[1] = ((pixelImage[1] << 2) | ((pixelImage[4] >> 2) & 0x3));
            unpacked[2] = ((pixelImage[2] << 2) | ((pixelImage[4] >> 4) & 0x3));
            unpacked[3] = ((pixelImage[3] << 2) | ((pixelImage[4] >> 6) & 0x3));
            for (unsigned int i = 0; i < pixelStep && (x + i) < width; ++i) {
                bayer[y][x + i] = unpacked[i];
            }
        }
    }

    // Step 2: Bilinear debayering (SRGGB pattern)
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            unsigned int xOffsetPtrFB = y * width * bytesPerPixelFB + x * bytesPerPixelFB;
            unsigned char *pixelFB = data + xOffsetPtrFB;
            uint16_t R = 0, G = 0, B = 0;
            // Handle borders by clamping
            auto get = [&](int yy, int xx) -> uint16_t {
                int y_ = std::max(0, std::min((int)height - 1, yy));
                int x_ = std::max(0, std::min((int)width - 1, xx));
                return bayer[y_][x_];
            };
            if ((y % 2 == 0) && (x % 2 == 0)) {
                // R location
                R = get(y, x);
                G = (get(y, x - 1) + get(y, x + 1) + get(y - 1, x) + get(y + 1, x)) / 4;
                B = (get(y - 1, x - 1) + get(y - 1, x + 1) + get(y + 1, x - 1) + get(y + 1, x + 1)) / 4;
            } else if ((y % 2 == 0) && (x % 2 == 1)) {
                // G on R row
                R = (get(y, x - 1) + get(y, x + 1)) / 2;
                G = get(y, x);
                B = (get(y - 1, x) + get(y + 1, x)) / 2;
            } else if ((y % 2 == 1) && (x % 2 == 0)) {
                // G on B row
                R = (get(y - 1, x) + get(y + 1, x)) / 2;
                G = get(y, x);
                B = (get(y, x - 1) + get(y, x + 1)) / 2;
            } else {
                // B location
                R = (get(y - 1, x - 1) + get(y - 1, x + 1) + get(y + 1, x - 1) + get(y + 1, x + 1)) / 4;
                G = (get(y, x - 1) + get(y, x + 1) + get(y - 1, x) + get(y + 1, x)) / 4;
                B = get(y, x);
            }
            // Scale 10-bit to 8-bit
            pixelFB[2] = (unsigned char)(R >> 2);
            pixelFB[1] = (unsigned char)(G >> 2);
            pixelFB[0] = (unsigned char)(B >> 2);
        }
    }
    ImageData result;
    result.data = data;
    result.size = dataSize;
    return result;
}
ImageData ImageConvertCPU::convert12BitPackedBayerToRGB888(const Image *image, int scaleFactor) const
{
    int scaledWidth = image->width() / scaleFactor;
    int scaledHeight = image->height() / scaleFactor;
    
    // RGB888 output: 3 bytes per pixel
    size_t outputSize = scaledWidth * scaledHeight * 3;
    unsigned char* rgbData = new unsigned char[outputSize];
    
    // Temporary buffer for unpacked 12-bit values
    uint16_t* tempData = new uint16_t[image->width() * image->height()];
    
    const unsigned char *ptrImage = image->planes()[0];
    
    // Unpack 12-bit packed data
    for (int y = 0; y < image->height(); y++) {
        for (int x = 0; x < image->width(); x += 2) {
            // Each pair of pixels is stored in 3 bytes
            int pixelPairIndex = (y * image->width() + x) / 2;
            int byteIndex = pixelPairIndex * 3;
            
            if (byteIndex + 2 < image->imageSize()) {
                // Extract two 12-bit pixels from 3 bytes
                // According to kernel documentation:
                // byte0 = pixel0_high[7:0], byte1 = pixel1_high[7:0]
                // byte2 = pixel1_low[7:4] | pixel0_low[3:0] 
                uint16_t pixel1 = (ptrImage[byteIndex]<< 4) | ((ptrImage[byteIndex + 2] & 0x0F));
                uint16_t pixel2 = (ptrImage[byteIndex + 1]<< 4) | (((ptrImage[byteIndex + 2] & 0xF0) >> 4));

                // std::cout << "Unpacking pixel pair at (" << x << ", " << y << "): "
                //           << "Pixel1: " << pixel1 << ", Pixel2: " << pixel2 << std::endl;

                // //Print raw pixel values for debugging
                // std::cout << "Value: " << (int)  ptrImage[byteIndex] << " " 
                //           << (int) ptrImage[byteIndex + 1] << " " 
                //           <<(int)  ptrImage[byteIndex + 2] << std::endl;
                
                tempData[y * image->width() + x] = pixel1;
                if (x + 1 < image->width()) {
                    tempData[y * image->width() + x + 1] = pixel2;
                }
            }
        }
    }
    
    // Convert Bayer pattern to RGB with scaling
    for (int y = 0; y < scaledHeight; y++) {
        for (int x = 0; x < scaledWidth; x++) {
            int srcX = x * scaleFactor;
            int srcY = y * scaleFactor;
            
            // Simple Bayer to RGB conversion (nearest neighbor debayering)
            uint16_t r = 0, g = 0, b = 0;
            
            // Determine Bayer pattern position (assuming RGGB pattern for SRGGB12P)
            bool isRedRow = (srcY % 2 == 0);
            bool isRedCol = (srcX % 2 == 0);
            
            if (isRedRow && isRedCol) {
                // Red pixel
                r = tempData[srcY * image->width() + srcX];
                // Interpolate green from neighbors
                if (srcX + 1 < image->width()) g += tempData[srcY * image->width() + srcX + 1];
                if (srcY + 1 < image->height()) g += tempData[(srcY + 1) * image->width() + srcX];
                g /= 2;
                // Interpolate blue from diagonal
                if (srcX + 1 < image->width() && srcY + 1 < image->height()) {
                    b = tempData[(srcY + 1) * image->width() + srcX + 1];
                }
            } else if (isRedRow && !isRedCol) {
                // Green pixel (red row)
                g = tempData[srcY * image->width() + srcX];
                // Get red from left, blue from below
                if (srcX > 0) r = tempData[srcY * image->width() + srcX - 1];
                if (srcY + 1 < image->height()) b = tempData[(srcY + 1) * image->width() + srcX];
            } else if (!isRedRow && isRedCol) {
                // Green pixel (blue row)
                g = tempData[srcY * image->width() + srcX];
                // Get red from above, blue from right
                if (srcY > 0) r = tempData[(srcY - 1) * image->width() + srcX];
                if (srcX + 1 < image->width()) b = tempData[srcY * image->width() + srcX + 1];
            } else {
                // Blue pixel
                b = tempData[srcY * image->width() + srcX];
                // Interpolate green from neighbors
                if (srcX > 0) g += tempData[srcY * image->width() + srcX - 1];
                if (srcY > 0) g += tempData[(srcY - 1) * image->width() + srcX];
                g /= 2;
                // Interpolate red from diagonal
                if (srcX > 0 && srcY > 0) {
                    r = tempData[(srcY - 1) * image->width() + srcX - 1];
                }
            }
            
            // Convert from 12-bit to 8-bit and store as RGB888
            int outputIndex = (y * scaledWidth + x) * 3;
            rgbData[outputIndex] = (r >> 4) & 0xFF;     // R
            rgbData[outputIndex + 1] = (g >> 4) & 0xFF; // G  
            rgbData[outputIndex + 2] = (b >> 4) & 0xFF; // B
        }
    }
    
    delete[] tempData;
    
    ImageData result;
    result.data = rgbData;
    result.size = outputSize;
    return result;
}