#include <cv/imagestats.hpp>
#include <math.h>
// #include <opencv2/opencv.hpp>



int ImageStats::stats(const Image *image, u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub)
{
        return stats_CPU(image, min, max, mean, sub);
        // return stats_OpenCV_resize(image, min, max, mean, sub);
        // return stats_OpenCV_crop(image, min, max, mean, sub);
}

int ImageStats::stats_CPU(const Image *image, u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub)
{
        u_int64_t sum = 0;
        min = -1;
        max = 0;
        mean = 0;

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        u_int32_t threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        u_int32_t threadHeight = image->height()/threadCount;
        u_int32_t yMin = threadId * threadHeight;
        u_int32_t yMax = (threadId+1) * threadHeight;
        u_int64_t lsum = 0;
#else
        u_int32_t yMin = 0;
        u_int32_t yMax = image->height();
#endif
        for (u_int32_t y = yMin; y < yMax; y += sub) {
                for (u_int32_t x = 0; x < image->width(); x += sub) {
                        u_int16_t val16 = image->pixelValue(x, y);
                        if (val16 < min) {
                                min = val16;
                        }
                        if (val16 > max) {
                                max = val16;
                        }
#if _OPENMP
                        lsum += val16;
#else
                        sum += val16;
#endif
                }
        }
#if _OPENMP
        sum += lsum;
        }
#endif
        mean = sum / (image->width()/sub * image->height()/sub);
        return 0;
}


// int ImageStats::stats_OpenCV_resize(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const
// {
// 	min = -1;
// 	max = 0;
// 	mean = 0;

// 	cv::Mat img(m_height, m_width, CV_16U, (char *)m_planes[0]);
    
// 	cv::Mat dst;
// 	cv::resize(img, dst, cv::Size(), 1.0/sub, 1.0/sub, cv::INTER_NEAREST);
    
// 	cv::Scalar sMean = cv::mean(img, dst);
// 	mean = sMean[0];
// 	return 0;
// }

// int ImageStats::stats_OpenCV_crop(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub) const
// {
// 	min = -1;
// 	max = 0;
// 	mean = 0;

// 	cv::Mat img(m_height, m_width, CV_16U, (char *)m_planes[0]);
// 	cv::Mat crop = img(cv::Rect(0, 0, m_width/sub, m_height/sub));

// 	cv::Scalar sMean = cv::mean(crop);
// 	mean = sMean[0];
// 	return 0;
// }

int ImageStats::std(const Image *image, u_int16_t mean, float &std, u_int8_t sub)
{
        int16_t dev;
        int64_t sum = 0;

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        u_int32_t threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        u_int32_t threadHeight = image->height()/threadCount;
        u_int32_t yMin = threadId * threadHeight;
        u_int32_t yMax = (threadId+1) * threadHeight;
        u_int64_t lsum = 0;
#else
        u_int32_t yMin = 0;
        u_int32_t yMax = image->height();
#endif
        for (u_int32_t y = yMin; y < yMax; y += sub) {
                for (u_int32_t x = 0; x < image->width(); x += sub) {
                        u_int16_t val16 = image->pixelValue(x, y);
                        dev = val16 - mean;
#if _OPENMP
                        lsum += dev * dev;
#else
                        sum += dev * dev;
#endif
                }
        }
#if _OPENMP
        sum += lsum;
        }
#endif
        std = sqrt(sum / (image->width()/sub * image->height()/sub));
        return 0;
}

int ImageStats::rowStats(const Image *image, u_int16_v &min, u_int16_v &max, u_int16_v &mean, u_int8_t sub)
{
        int16_t dev = 0;
        int64_t sum = 0;
        u_int16_t sumCount = 0;
        u_int16_t count = image->height()/sub;
        u_int16_t index = 0;
        min.resize(count);
        max.resize(count);
        mean.resize(count);

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        u_int32_t threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        u_int32_t threadHeight = image->height()/threadCount;
        u_int32_t yMin = threadId * threadHeight;
        u_int32_t yMax = (threadId+1) * threadHeight;
        u_int64_t lsum = 0;
#else
        u_int32_t yMin = 0;
        u_int32_t yMax = image->height();
#endif
        for (u_int32_t y = yMin; y < yMax; y += sub) {
                sumCount = 0;
                min[index] = -1;
                max[index] = 0;
                for (u_int32_t x = 0; x < image->width(); x += sub) {
                        u_int16_t val16 = image->pixelValue(x, y);
                        if (val16 < min[index]) {
                                min[index] = val16;
                        }
                        if (val16 > max[index]) {
                                max[index] = val16;
                        }
#if _OPENMP
                        lsum += val16;
#else
                        sum += val16;
#endif
                        sumCount++;
                }
#if _OPENMP
                mean[index] = lsum / sumCount;
                lsum = 0;
#else
                u_int16_t meanValue = sum / sumCount;
                mean[index] = meanValue;
                sum = 0;
#endif          
                index++;
        }
        return 0;
}

int ImageStats::rowStd(const Image *image, const u_int16_v &mean, float_v &std, u_int8_t sub)
{
        int16_t dev = 0;
        int64_t sum = 0;
        u_int16_t sumCount = 0;
        float stdValue = 0;
        u_int16_t count = image->height()/sub;
        u_int16_t index = 0;
        std.resize(count);

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        u_int32_t threadCount = omp_get_num_procs();
                omp_set_num_threads(threadCount);
                u_int32_t threadHeight = m_height/threadCount;
        u_int32_t yMin = threadId * threadHeight;
        u_int32_t yMax = (threadId+1) * threadHeight;
        u_int64_t lsum = 0;
#else
        u_int32_t yMin = 0;
        u_int32_t yMax = image->height();
#endif
        for (u_int32_t y = yMin; y < yMax; y += sub) {
                sumCount = 0;
                for (u_int32_t x = 0; x < image->width(); x += sub) {
                        u_int16_t val16 = image->pixelValue(x, y);
                        dev = val16 - mean[index];
#if _OPENMP
                        lsum += dev * dev;
#else
                        sum += dev * dev;
#endif
                        sumCount++;
                }
#if _OPENMP
                std[index] = sqrt(lsum / sumCount);
                lsum = 0;
#else
                stdValue = sqrt(sum / sumCount);
                std[index] = stdValue;
                sum = 0;
#endif  
                index++;
        }
        return 0;
}