#pragma once

#include <cv/image.hpp>
#include "text.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;


void drawImageInfo(Mat img, Image *image, int x, int y);


template <typename T>
void drawMat(Mat img, Mat mat, int x, int y)
{
        char value[100];
        for (int col = 0; col < mat.cols; col++) {
                for (int row = 0; row < mat.rows; row++) {
                        T data = mat.at<T>(row, col);
                        Scalar color = Scalar(255, 255, 255);
                        if (data >= 1.0) {
                                color = Scalar(0, 0, 255);
                        }
                        sprintf(value, "%+.3f", data);

                        Text text(x + 7*col, y + row);
                        text.setText(value);
                        text.draw(img, color);
                }
        }
}