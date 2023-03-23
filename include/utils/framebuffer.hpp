#pragma once

#include <sys/types.h>
#include <linux/fb.h>
#include <v4l2image.hpp>

class FrameBuffer
{
        public:
                FrameBuffer();
                ~FrameBuffer();

                int open();
                int close();
                void fill();
                void print(V4L2Image &image, u_int8_t shift);

        protected:
                int m_fd;
                void * m_ptr;

        private:
                struct fb_var_screeninfo m_varScreenInfo;
                struct fb_fix_screeninfo m_fixScreenInfo;

                void print08(V4L2Image &image);
                void print16(V4L2Image &image, u_int8_t shift);

                void handleErrorForOpen(const char *path, int err);
                void handleErrorForClose(int fd, int err);
                void handleErrorForIoctl(unsigned long int request, int err);
};