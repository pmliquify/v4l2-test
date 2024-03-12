#include "window.hpp"


void onMouseCallback(int event, int x, int y, int flags, void* userdata)
{
        ((Window *)userdata)->onMouse(event, x, y, flags);
}

Window::Window(const char *name) :
        m_name(name),
        m_visible(false),
        m_wasClosed(false),
        m_roiCenter(-1, -1),
        m_roiZoom(1.0),
        m_roiZoomStart(1.0)
{
        
}

Window::~Window()
{
        hide();
}

void Window::addControl(Control *control)
{
        m_controls[control->name()] = control;
}

Control *Window::control(const char *name)
{
     return m_controls[name];
}

void Window::addButton(Button *button)
{
        m_buttons[button->name()] = button;
}

Button *Window::button(const char *name)
{
        return m_buttons[name];
}

void Window::show(Mat img)
{
        if (!m_visible) {
                namedWindow(m_name, WINDOW_NORMAL);
                resizeWindow(m_name, img.cols/2, img.rows/2);
                setMouseCallback(m_name, onMouseCallback, this);
                startWindowThread();
        }
        if (m_visible) {
                try {
                        bool visible = getWindowProperty(m_name, WINDOW_NORMAL);

                } catch (const cv::Exception& e) {
                        m_wasClosed = true;
                        m_visible = false;
                        return;
                }
        }

        m_img = img;
        update();
        m_visible = true;
}

void Window::hide()
{
        if (m_visible) {
                destroyWindow(m_name);
                m_visible = false;
        }
}

bool Window::wasClosed()
{ 
        if (m_wasClosed) {
                m_wasClosed = false;
                return true;
        }
        return false; 
}

void Window::update()
{
        Mat img = m_img.clone();

        if (img.empty()) {
                return;
        }

        if (m_roiCenter.x < 0) {
                m_roiCenter.x = img.cols/2;                
        }
        if (m_roiCenter.y < 0) {
                m_roiCenter.y = img.rows/2;
        }

        int width = img.cols*m_roiZoom;
        int height = img.rows*m_roiZoom;
        int x = m_roiCenter.x - width/2;
        int y = m_roiCenter.y - height/2;

        if (x < 0) {
                x = 0;
        }
        if (x > img.cols - width) {
                x = img.cols - width;
        }
        if (y < 0) {
                y = 0;
        }
        if (y > img.rows - height) {
                y = img.rows - height;
        }

        m_roi = Rect(x, y, width, height);

        for (auto control : m_controls) {
                control.second->draw(img);
        }
        for (auto button : m_buttons) {
                button.second->draw(img);
        }
        
        Mat zoomedImage = img(m_roi);

        imshow(m_name, zoomedImage);
        m_visible = true;
}

void Window::onMouse(int event, int x, int y, int flags)
{
        switch (event) {
        case EVENT_LBUTTONDOWN:
                m_mouseEvent.init(event, x, y, flags);
                m_roiZoomStart = m_roiZoom;
                break;
        }
        m_mouseEvent.update(event, x, y, flags);
        int xDelta = m_mouseEvent.deltaX();
        int yDelta = m_mouseEvent.deltaY();

        if (flags & EVENT_FLAG_LBUTTON && flags & EVENT_FLAG_CTRLKEY) {
                float zoomDelta = m_roiZoom * 0.001 * yDelta;
                float roiZoom = m_roiZoomStart += zoomDelta;
                if (roiZoom < 0.01) {
                        m_roiZoom = 0.01;
                } else if (roiZoom > 1.0) {
                        m_roiZoom = 1.0;
                } else {
                        m_roiZoom = roiZoom;
                }

        } else if (flags == EVENT_FLAG_LBUTTON) {
                if (m_roiCenter.x - xDelta > 0) {
                        m_roiCenter.x -= xDelta;
                } else {
                        m_roiCenter.x = 0;
                }
                if (m_roiCenter.y - yDelta > 0) {
                        m_roiCenter.y -= yDelta;
                } else {
                        m_roiCenter.y = 0;
                }
        }

        MouseEvent zoomedEvent = m_mouseEvent;
        zoomedEvent.move(m_roi.x, m_roi.y);
        for (auto control : m_controls) {
                control.second->onMouse(zoomedEvent);
        }
        for (auto button : m_buttons) {
                button.second->onMouse(zoomedEvent);
        }

        onMouse(zoomedEvent);
        
        update();
}