#include "mouseevent.hpp"


MouseEvent::MouseEvent() :
        m_xStart(0),
        m_yStart(0),
        m_xLast(0),
        m_yLast(0),
        m_x(0),
        m_y(0)
{

}

void MouseEvent::init(int event, int x, int y, int flags)
{
        m_event = event;
        m_xStart = x;
        m_yStart = y;
        m_xLast = x;
        m_yLast = y;
        m_flags = flags;
}

void MouseEvent::update(int event, int x, int y, int flags)
{
        m_event = event;
        m_xLast = m_x;
        m_x = x;
        m_yLast = m_y;
        m_y = y;
        m_flags = flags;
}

void MouseEvent::move(int x, int y)
{
        m_xStart += x;
        m_yStart += y;
        m_xLast += x;
        m_yLast += y;
        m_x += x;
        m_y += y;
}
