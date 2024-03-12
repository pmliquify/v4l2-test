#pragma once

#include "window.hpp"
#include "sources/imagesource.hpp"
#include "ccmwindow.hpp"


class MainWindow : public Window
{
public:
    MainWindow(const char *name);

    void init(ImageSource *imageSource);
    virtual void show(Mat img);

protected:
    virtual void onMouse(const MouseEvent &event);

private:
    ImageSource *m_imageSource;
#ifdef WITH_CCM
    CCMWindow    m_ccmWindow;
#endif
};