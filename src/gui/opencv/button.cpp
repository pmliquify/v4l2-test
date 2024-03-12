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
    char text[100];
    sprintf(text, "%s", m_name.c_str());
    m_text.setText(text);

    if (m_mouseOver) {
        rectangle(img, m_text.rect(), Scalar(0, 0, 0), 6);
        rectangle(img, m_text.rect(), Scalar(255, 255, 255), 2);
    }
    Scalar color = Scalar(100, 100, 100);
    if (m_pressed) {
        color = Scalar(255, 255, 255);
    }
    m_text.draw(img, color);
}

bool Button::pressed() 
{ 
    m_hasChanged = false;
    return m_pressed; 
}

void Button::setPressed(bool pressed)
{
    m_hasChanged = true;
    m_pressed = pressed;
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