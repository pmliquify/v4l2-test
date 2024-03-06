#pragma once

#include <linux/fb.h>
#include <cv/image.hpp>

class FrameBuffer
{
public:
        FrameBuffer();
        ~FrameBuffer();

        int open();
        int close();
        void fill();
        void update(const Image *image);

protected:
        int m_fd;
        void * m_ptr;

private:
        struct fb_var_screeninfo m_varScreenInfo;
        struct fb_fix_screeninfo m_fixScreenInfo;

        void print08(const Image *image);
        void print16(const Image *image, unsigned char shift);

        void handleErrorForOpen(const char *path, int err);
        void handleErrorForClose(int fd, int err);
        void handleErrorForIoctl(unsigned long int request, int err);
};