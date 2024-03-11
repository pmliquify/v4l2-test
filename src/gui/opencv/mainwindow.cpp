#include "mainwindow.hpp"


MainWindow::MainWindow(const char *name) :
        Window(name),
        m_imageSource(NULL),
        m_ccmWindow("ColorChecker")
{

}

void MainWindow::init(ImageSource *imageSource)
{
        m_imageSource = imageSource;
        addControl(new Control("Exposure",   "us", 0, 10000, 1000, 0, 1000000, 1, 3));
        addControl(new Control("Gain",      "mdB", 0,     0,   10, 0,    1024, 1, 4));
        addControl(new Control("Blacklevel",   "", 0,     0,   10, 0,    1024, 1, 5));
        addControl(new Control("Framerate", "mHz", 0,     0, 1000, 0, 1000000, 1, 6));

        addButton(new Button("ColorChecker", -1, 1));
}

void MainWindow::show(Mat img)
{
        Window::show(img);

        Button *colorCheckerButton = button("ColorChecker");
        assert(colorCheckerButton != NULL);
        if (colorCheckerButton->pressed()) {
                m_ccmWindow.show(img);
        } else {
                m_ccmWindow.hide();
        }
}

void MainWindow::onMouse(const MouseEvent &event)
{
        Window::onMouse(event);

        assert(m_imageSource != NULL);
        
        Control *exposure = control("Exposure");
        assert(exposure != NULL);
        if (exposure->hasChanged()) {
                m_imageSource->setExposure(exposure->value());
        }
        Control *gain = control("Gain");
        assert(gain != NULL);
        if (gain->hasChanged()) {
                m_imageSource->setGain(gain->value());
        }
        Control *blacklevel = control("Blacklevel");
        assert(blacklevel != NULL);
        if (blacklevel->hasChanged()) {
                m_imageSource->setBlackLevel(blacklevel->value());
        }
        Control *framerate = control("Framerate");
        assert(framerate != NULL);
        if (framerate->hasChanged()) {
                m_imageSource->setFrameRate(framerate->value());
        }
}