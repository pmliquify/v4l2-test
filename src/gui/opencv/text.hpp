#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;


class Text
{
public:
    Text(int x, int y);
    
    void setText(const String &text);
    String text() const { return m_text; }
    Rect rect() const;
    void draw(Mat img, Scalar color = Scalar(255, 255, 255));

private:
    int    m_fontFace = FONT_HERSHEY_SIMPLEX;
    double m_fontScale = 1.0;
    int    m_thickness = 2;
    String m_text;
    Point  m_indexedPos;
    Point  m_position;
    Size   m_size;
};