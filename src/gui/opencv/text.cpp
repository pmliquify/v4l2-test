#include "text.hpp"


Text::Text(int x, int y) :
        m_fontFace(FONT_HERSHEY_SIMPLEX),
        m_fontScale(1.0),
        m_thickness(2),
        m_indexedPos(x, y)
{

}

void Text::setText(const String &text)
{
        int baseline = 0;
        m_size = getTextSize(text, m_fontFace, m_fontScale, m_thickness, &baseline);
        m_text = text;
}

Rect Text::rect() const 
{ 
        int border = 8;
        int borderTop = border;
        int borderLeft = border;
        int borderBottom = border;
        int borderRight = border;
        return Rect(
                m_position.x - borderLeft, 
                m_position.y - borderTop,
                m_size.width + borderLeft + borderRight,
                m_size.height + borderTop + borderBottom);
}

void Text::draw(Mat img, Scalar color)
{
        int x = m_indexedPos.x;
        int y = m_indexedPos.y;

        if (x < 0) {
                x = (img.cols + 20*x - m_size.width);
        } else {
                x = 20*x;
        }
        if (y < 0) {
                y = (img.rows - m_size.height*(y+2));
        } else {
                y = y*40;
        }

        m_position = Point(x, y - m_size.height + m_thickness/2);
        putText(img, m_text, Point(x, y),
                m_fontFace, m_fontScale, color, m_thickness);
}
