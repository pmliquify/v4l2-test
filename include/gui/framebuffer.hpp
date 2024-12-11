#pragma once

#include <linux/fb.h>
#include <cv/image.hpp>
#include <cstdint> 
class FrameBuffer
{
public:
        FrameBuffer();
        ~FrameBuffer();

        int open();
        int close();
        void fill();
        int show(const Image *image);

protected:
        int m_fd;
        void *m_ptr;

private:
        struct fb_var_screeninfo m_varScreenInfo;
        struct fb_fix_screeninfo m_fixScreenInfo;

        uint16_t grey10BitToRGB565 (const uint16_t &grey) const;
        uint16_t grey8BitToRGB565 (const char &grey) const;
        unsigned long grey8BitToRGB888(const uint16_t &grey) const;
        unsigned long grey10BitToRGB888(const uint16_t &grey) const;


        void print08(const Image *image);
        void print16(const Image *image, unsigned char shift);
        void printDeBayer08(const Image *image);
        void printGrey10(const Image *image);
        void printGrey10P(const Image *image);
        void writeTestImage();

        void handleErrorForOpen(const char *path, int err);
        void handleErrorForClose(int fd, int err);
        void handleErrorForIoctl(unsigned long int request, int err);
};