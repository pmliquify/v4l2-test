#pragma once

#include <cv/image.hpp>
#include <sources/imagesource.hpp>


class Viewer
{
public:
    Viewer();
    ~Viewer();

    void show(ImageSource *imageSource);
    void hide();
    void update(Image *image);

    void onMouse(int event, int x, int y, int flags);

private:
    ImageSource *m_imageSource;
    int          m_roiX;
    int          m_roiY;
    float        m_roiZoom;
    float        m_roiZoomStart;
    int          m_xStart;
    int          m_yStart;
    int          m_xLast;
    int          m_yLast;
    int          m_exposure;
    int          m_exposureStart;
    int          m_gain;
    int          m_gainStart;
};