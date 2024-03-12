#include "control.hpp"


Control::Control(const char *name, const char *unit, int floatPrecision,
    double defaultValue, double step, double min, double max,
    int x, int y, DescriptionMap descMap) :
    m_name(name),
    m_unit(unit),
    m_descMap(descMap),
    m_floatPrecision(floatPrecision),
    m_text(x, y),
    m_mouseOver(false),
    m_default(defaultValue),
    m_value(defaultValue),
    m_step(step),
    m_min(min),
    m_max(max),
    m_hasChanged(false)
{
}

void Control::draw(Mat img)
{
    char text[100];
    sprintf(text, "%s: % .*f %s%s", m_name.c_str(), m_floatPrecision, m_value, m_unit.c_str(),
        m_descMap[(int)m_value].c_str());
    m_text.setText(text);

    Scalar color(255, 255, 255);
    if (m_mouseOver) {
        rectangle(img, m_text.rect(), Scalar(0, 0, 0), 6);
        rectangle(img, m_text.rect(), color, 2);
    }
    m_text.draw(img, color);
}

double Control::value() 
{ 
    m_hasChanged = false;
    return m_value; 
}

void Control::onMouse(const MouseEvent &event)
{
    m_mouseOver = m_text.rect().contains(event.position());

    if (m_mouseOver) {
        if (event.event() & EVENT_LBUTTONUP) {
            m_value = m_default;
        
        } else if (event.event() & EVENT_MOUSEWHEEL) {
            double step = m_step;
            if (event.flags() > 0) {
                step *= -1.0;
            }
            if (event.flags() & EVENT_FLAG_CTRLKEY) {
                if (event.flags() & EVENT_FLAG_SHIFTKEY) {
                    step /= 100.0;
                } else {
                    step /= 10.0;
                }
            } else if (event.flags() & EVENT_FLAG_ALTKEY) {
                step *= 10.0;
            }

            if (!m_descMap.size() > 0) {
                int value = m_value += step;
                while (m_min <= value && value <= m_max) {
                    if (m_descMap[(int)value].empty()) {
                        value += step;
                    } else {
                        break;
                    }
                }
                m_value = value;

            } else {
                m_value += step;
            }

            if (m_value < m_min) {
                m_value = m_min;
            }
            if (m_value > m_max) {
                m_value = m_max;
            }
            m_hasChanged = true;
        }
    }
}