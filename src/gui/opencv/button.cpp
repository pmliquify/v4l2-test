#include "button.hpp"


Button::Button(const char *name, int x, int y) :
    m_name(name),
    m_text(x, y),
    m_mouseOver(false),
    m_pressed(false),
    m_hasChanged(false)
{
}

void Button::draw(Mat img)
{
    Scalar color = Scalar(100, 100, 100);
    if (m_pressed) {
        color = Scalar(255, 255, 255);
    }

    char text[100];
    sprintf(text, "%s", m_name.c_str());
    m_text.setText(text);
    m_text.draw(img, color);

    if (m_mouseOver) {
        rectangle(img, m_text.rect(), Scalar(255, 255, 255), 2);
    }
}

bool Button::pressed() 
{ 
    m_hasChanged = false;
    return m_pressed; 
}

void Button::onMouse(const MouseEvent &event)
{
    m_mouseOver = m_text.rect().contains(event.position());

    if (m_mouseOver) {
        if (event.event() & EVENT_LBUTTONUP) {
            m_pressed = m_pressed ? false : true;
            m_hasChanged = true;
        }
    }
}