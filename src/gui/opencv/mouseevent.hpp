#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;


class MouseEvent
{
public:
    MouseEvent();

    void init(int event, int x, int y, int flags);
    void update(int event, int x, int y, int flags);
    void move(int x, int y);

    int event() const { return m_event; }
    Point position() const { return Point(m_x, m_y); }
    int x() const { return m_x; }
    int y() const { return m_y; }
    int flags() const { return m_flags; } 
    int deltaX() const { return m_x - m_xLast; }
    int deltaY() const { return m_y - m_yLast; }
    int absDeltaX() const { return m_x - m_xStart; }
    int absDeltaY() const { return m_y - m_yStart; }

private:
    int m_event;
    int m_x;
    int m_y;
    int m_flags;
    int m_xStart;
    int m_yStart;
    int m_xLast;
    int m_yLast;
};