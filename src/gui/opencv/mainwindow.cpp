#include "mainwindow.hpp"

#define CTL_EXPOSURE     "Exposure"
#define CTL_GAIN         "Gain"
#define CTL_BLACKLEVEL   "Blacklevel"
#define CTL_FRAMERATE    "Framerate"
#define BTN_SAVEIMAGE    "Save Image"
#define BTN_COLORCHECKER "ColorChecker"



MainWindow::MainWindow(const char *name) :
        Window(name),
        m_imageSource(NULL)
#ifdef WITH_CCM
        , m_ccmWindow("ColorChecker")
#endif
{

}

void MainWindow::init(ImageSource *imageSource)
{
        m_imageSource = imageSource;
        addControl(new Control(CTL_EXPOSURE,   "us", 0, 10000, 1000, 0, 1000000, 1, 3));
        addControl(new Control(CTL_GAIN,      "mdB", 0,     0,   10, 0,    1024, 1, 4));
        addControl(new Control(CTL_BLACKLEVEL,   "", 0,     0,   10, 0,    1024, 1, 5));
        addControl(new Control(CTL_FRAMERATE, "mHz", 0,     0, 1000, 0, 1000000, 1, 6));
        addButton(new Button(BTN_SAVEIMAGE, 1, 7));

#ifdef WITH_CCM
        addButton(new Button("ColorChecker", -1, 1));
#endif
}

void MainWindow::show(Mat img)
{
        Window::show(img);

#ifdef WITH_CCM
        Button *colorCheckerButton = button("ColorChecker");
        assert(colorCheckerButton != NULL);
        if (m_ccmWindow.wasClosed()) {
                colorCheckerButton->setPressed(false);
        }
        if (colorCheckerButton->pressed()) {
                m_ccmWindow.show(img);
        } else {
                m_ccmWindow.hide();
        }
#endif
}

void MainWindow::onMouse(const MouseEvent &event)
{
        Window::onMouse(event);

        assert(m_imageSource != NULL);
        
        Control *exposure = control(CTL_EXPOSURE);
        assert(exposure != NULL);
        if (exposure->hasChanged()) {
                m_imageSource->setExposure(exposure->value());
        }
        Control *gain = control(CTL_GAIN);
        assert(gain != NULL);
        if (gain->hasChanged()) {
                m_imageSource->setGain(gain->value());
        }
        Control *blacklevel = control(CTL_BLACKLEVEL);
        assert(blacklevel != NULL);
        if (blacklevel->hasChanged()) {
                m_imageSource->setBlackLevel(blacklevel->value());
        }
        Control *framerate = control(CTL_FRAMERATE);
        assert(framerate != NULL);
        if (framerate->hasChanged()) {
                m_imageSource->setFrameRate(framerate->value());
        }
        Button* saveImage = button(BTN_SAVEIMAGE);
        assert(saveImage != NULL);
        if (saveImage->hasChanged()) {
                if (saveImage->pressed()) {
                        Window::saveImage();
                        saveImage->setPressed(false);
                }
        }
}