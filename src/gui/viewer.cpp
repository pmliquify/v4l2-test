#include <gui/viewer.hpp>


Viewer::Viewer() :
        m_imageSource(NULL),
        m_roiX(0),
        m_roiY(0),
        m_roiZoom(1.0),
        m_xStart(0),
        m_yStart(0),
        m_xLast(0),
        m_yLast(0),
        m_exposure(10000),
        m_exposureStart(10000),
        m_gain(0),
        m_gainStart(0)
{
}

Viewer::~Viewer()
{
}

void Viewer::show(ImageSource *imageSource)
{
}

void Viewer::hide()
{
}

void Viewer::update(Image *image)
{
}

void Viewer::onMouse(int event, int x, int y, int flags)
{
}