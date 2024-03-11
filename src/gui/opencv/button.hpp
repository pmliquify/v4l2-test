#pragma once

#include "text.hpp"
#include "mouseevent.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;


class Button
{
public:
    Button(const char *name, int x, int y);

    String name() const { return m_name; }
    bool pressed();

    void draw(Mat img);

    bool hasChanged() const { return m_hasChanged;  }
    void onMouse(const MouseEvent& event);

private:
    String m_name;
    Text   m_text;
    bool   m_mouseOver;
    bool   m_pressed;
    bool   m_hasChanged;
};

typedef std::map<std::string, Button *> ButtonMap;