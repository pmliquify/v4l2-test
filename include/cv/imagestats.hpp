#pragma once

#include <cv/image.hpp>
#include <vector>


class ImageStats
{
public:
    typedef std::vector<unsigned short> u_short_v;
    typedef std::vector<float> float_v;

    static int stats(const Image *image, unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub);
    static int std(const Image *image, unsigned short mean, float &std, unsigned char sub);
    static int rowStats(const Image *image, u_short_v &min, u_short_v &max, u_short_v &mean, unsigned char sub);
    static int rowStd(const Image *image, const u_short_v &min, float_v &std, unsigned char sub);

private:
    static int stats_CPU(const Image *image, unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub);
    static int stats_OpenCV_resize(const Image *image, unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub);
    static int stats_OpenCV_crop(const Image *image, unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub);
};