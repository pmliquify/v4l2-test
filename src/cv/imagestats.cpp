#include <cv/imagestats.hpp>
#include <math.h>
// #include <opencv2/opencv.hpp>



int ImageStats::stats(const Image *image, unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub)
{
        return stats_CPU(image, min, max, mean, sub);
        // return stats_OpenCV_resize(image, min, max, mean, sub);
        // return stats_OpenCV_crop(image, min, max, mean, sub);
}

int ImageStats::stats_CPU(const Image *image, unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub)
{
        unsigned long sum = 0;
        min = -1;
        max = 0;
        mean = 0;

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = image->height()/threadCount;
        unsigned int yMin = threadId * threadHeight;
        unsigned int yMax = (threadId+1) * threadHeight;
        unsigned long lsum = 0;
#else
        unsigned int yMin = 0;
        unsigned int yMax = image->height();
#endif
        for (unsigned int y = yMin; y < yMax; y += sub) {
                for (unsigned int x = 0; x < image->width(); x += sub) {
                        unsigned short val16 = image->pixelValue(x, y);
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


// int ImageStats::stats_OpenCV_resize(unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub) const
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

// int ImageStats::stats_OpenCV_crop(unsigned short &min, unsigned short &max, unsigned short &mean, unsigned char sub) const
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

int ImageStats::std(const Image *image, unsigned short mean, float &std, unsigned char sub)
{
        short dev;
        int64_t sum = 0;

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = image->height()/threadCount;
        unsigned int yMin = threadId * threadHeight;
        unsigned int yMax = (threadId+1) * threadHeight;
        unsigned long lsum = 0;
#else
        unsigned int yMin = 0;
        unsigned int yMax = image->height();
#endif
        for (unsigned int y = yMin; y < yMax; y += sub) {
                for (unsigned int x = 0; x < image->width(); x += sub) {
                        unsigned short val16 = image->pixelValue(x, y);
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

int ImageStats::rowStats(const Image *image, u_short_v &min, u_short_v &max, u_short_v &mean, unsigned char sub)
{
        short dev = 0;
        int64_t sum = 0;
        unsigned short sumCount = 0;
        unsigned short count = image->height()/sub;
        unsigned short index = 0;
        min.resize(count);
        max.resize(count);
        mean.resize(count);

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        unsigned int threadCount = omp_get_num_procs();
        omp_set_num_threads(threadCount);
        unsigned int threadHeight = image->height()/threadCount;
        unsigned int yMin = threadId * threadHeight;
        unsigned int yMax = (threadId+1) * threadHeight;
        unsigned long lsum = 0;
#else
        unsigned int yMin = 0;
        unsigned int yMax = image->height();
#endif
        for (unsigned int y = yMin; y < yMax; y += sub) {
                sumCount = 0;
                min[index] = -1;
                max[index] = 0;
                for (unsigned int x = 0; x < image->width(); x += sub) {
                        unsigned short val16 = image->pixelValue(x, y);
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
                unsigned short meanValue = sum / sumCount;
                mean[index] = meanValue;
                sum = 0;
#endif          
                index++;
        }
        return 0;
}

int ImageStats::rowStd(const Image *image, const u_short_v &mean, float_v &std, unsigned char sub)
{
        short dev = 0;
        int64_t sum = 0;
        unsigned short sumCount = 0;
        float stdValue = 0;
        unsigned short count = image->height()/sub;
        unsigned short index = 0;
        std.resize(count);

#undef _OPENMP
#if _OPENMP 
#pragma omp parallel
                {
        int threadId = omp_get_thread_num();
        unsigned int threadCount = omp_get_num_procs();
                omp_set_num_threads(threadCount);
                unsigned int threadHeight = m_height/threadCount;
        unsigned int yMin = threadId * threadHeight;
        unsigned int yMax = (threadId+1) * threadHeight;
        unsigned long lsum = 0;
#else
        unsigned int yMin = 0;
        unsigned int yMax = image->height();
#endif
        for (unsigned int y = yMin; y < yMax; y += sub) {
                sumCount = 0;
                for (unsigned int x = 0; x < image->width(); x += sub) {
                        unsigned short val16 = image->pixelValue(x, y);
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