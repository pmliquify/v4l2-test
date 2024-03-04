#pragma once

#include <cv/image.hpp>
#include <vector>


class ImageStats
{
public:
    typedef std::vector<u_int16_t> u_int16_v;
    typedef std::vector<float> float_v;

    static int stats(const Image *image, u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub);
    static int std(const Image *image, u_int16_t mean, float &std, u_int8_t sub);
    static int rowStats(const Image *image, u_int16_v &min, u_int16_v &max, u_int16_v &mean, u_int8_t sub);
    static int rowStd(const Image *image, const u_int16_v &min, float_v &std, u_int8_t sub);

private:
    static int stats_CPU(const Image *image, u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub);
    static int stats_OpenCV_resize(const Image *image, u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub);
    static int stats_OpenCV_crop(const Image *image, u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub);
};