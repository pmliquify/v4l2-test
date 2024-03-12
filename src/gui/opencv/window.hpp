#pragma once

#include "mouseevent.hpp"
#include "control.hpp"
#include "button.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;


class Window
{
public:
    Window(const char *name);
    ~Window();

    void addControl(Control *control);
    Control *control(const char *name);
    void addButton(Button *button);
    Button *button(const char *name);

    virtual void show(Mat img);
    virtual void hide();
    bool wasClosed();

    void onMouse(int event, int x, int y, int flags);

protected:
    virtual void onMouse(const MouseEvent &event) {}
    virtual void update();

private:
    String      m_name;
    Mat         m_img;
    bool        m_visible;
    bool        m_wasClosed;
    MouseEvent  m_mouseEvent;
    Point       m_roiCenter;
    float       m_roiZoom;
    float       m_roiZoomStart;
    Rect        m_roi;
    ControlMap  m_controls;
    ButtonMap   m_buttons;
};