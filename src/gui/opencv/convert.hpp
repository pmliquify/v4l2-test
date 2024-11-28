#pragma once

#include <cv/image.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

Mat convert(Image *image);
cv::Mat convert_bayer10p_to_rgb(const uint8_t *data, int width, int height, int blackcols);
