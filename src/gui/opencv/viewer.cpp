#include <gui/viewer.hpp>
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>

using namespace std;
using namespace cv;

#define WINDOW "v4l2-test"


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

static void onMouseCallback(int event, int x, int y, int flags, void* userdata)
{
        ((Viewer *)userdata)->onMouse(event, x, y, flags);
}

void Viewer::show(ImageSource *imageSource)
{
        m_imageSource = imageSource;

        namedWindow(WINDOW, WINDOW_NORMAL);
        setWindowTitle(WINDOW, WINDOW);
        resizeWindow(WINDOW, 800, 600);
        setMouseCallback(WINDOW, onMouseCallback, this);
        startWindowThread();
}

void Viewer::hide()
{
        destroyWindow(WINDOW);

        m_imageSource = NULL;
}

static void drawText(InputOutputArray img, const String& text, int x, int y)
{
        putText(img, text, Point(20 + x*20, 40 + y*40),
                FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);
}

static void drawImageInfo(InputOutputArray img, Image *image, int x, int y) 
{
        char text[200];
        sprintf(text, "[#%5u, ts: %8lu] (w: %u, h: %u)", 
                image->sequence(), image->timestamp(),
                image->width(), image->height());
        drawText(img, text, x, y);
}

static void drawRoiZoom(InputOutputArray img, Image *image, int roiX, int roiY, float roiZoom, int x, int y) 
{
        int roiWidth = image->width()*roiZoom;
        int roiHeight = image->height()*roiZoom;

        char text[200];
        sprintf(text, "Zoom: %3.1f (x, %u, y: %u, w: %u, h: %u)", 
                100.0/roiZoom, roiX, roiY, roiWidth, roiHeight);
        drawText(img, text, x, y);
}

static void drawControl(InputOutputArray img, const String& name, int value, const String& unit, 
        int x, int y)
{
        char text[100];
        sprintf(text, "%s: %u %s", name.c_str(), value, unit.c_str());
        drawText(img, text, x, y);
}

void Viewer::update(Image *image)
{
        int type = CV_8UC1;
        bool softISP = false;
        int divider = 1;
        int code = 0;

        switch (image->pixelformat()) {
        case V4L2_PIX_FMT_GREY:    type =  CV_8UC1; softISP = false; divider =    1; break;
        case V4L2_PIX_FMT_SRGGB8:  type =  CV_8UC1; softISP = true;  divider =    1; code = COLOR_BayerRG2RGB; break;
        case V4L2_PIX_FMT_SGBRG8:  type =  CV_8UC1; softISP = true;  divider =    1; code = COLOR_BayerGB2RGB; break;
        case V4L2_PIX_FMT_SGRBG8:  type =  CV_8UC1; softISP = true;  divider =    1; code = COLOR_BayerGR2RGB; break;
        case V4L2_PIX_FMT_SBGGR8:  type =  CV_8UC1; softISP = true;  divider =    1; code = COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_Y10:     type = CV_16UC1; softISP = false; divider = 1023; break;
        case V4L2_PIX_FMT_SRGGB10: type = CV_16UC1; softISP = true;  divider = 1023; code = COLOR_BayerRG2RGB; break;
        case V4L2_PIX_FMT_SGBRG10: type = CV_16UC1; softISP = true;  divider = 1023; code = COLOR_BayerGB2RGB; break;
        case V4L2_PIX_FMT_SGRBG10: type = CV_16UC1; softISP = true;  divider = 1023; code = COLOR_BayerGR2RGB; break;
        case V4L2_PIX_FMT_SBGGR10: type = CV_16UC1; softISP = true;  divider = 1023; code = COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_Y12:     type = CV_16UC1; softISP = false; divider = 4095; break;
        case V4L2_PIX_FMT_SRGGB12: type = CV_16UC1; softISP = true;  divider = 4095; code = COLOR_BayerRG2RGB; break;
        case V4L2_PIX_FMT_SGBRG12: type = CV_16UC1; softISP = true;  divider = 4095; code = COLOR_BayerGB2RGB; break;
        case V4L2_PIX_FMT_SGRBG12: type = CV_16UC1; softISP = true;  divider = 4095; code = COLOR_BayerGR2RGB; break;
        case V4L2_PIX_FMT_SBGGR12: type = CV_16UC1; softISP = true;  divider = 4095; code = COLOR_BayerBG2RGB; break;
        case V4L2_PIX_FMT_YUYV:    type =  CV_8UC2; softISP = true;  divider =    1; code = COLOR_YUV2BGR_YUY2; break;
        }

        Mat imageRaw(image->height(), image->width(), type, (char *)image->planes()[0]);
        Mat imageRaw8 = imageRaw.clone();

        if (divider > 1) {
                imageRaw8.convertTo(imageRaw8, CV_8UC1, 255.0/divider/(1 << image->shift()));
        }

        Mat imageShow;
        if (softISP) {
                Mat imageRGB8(image->height(), image->width(), CV_8UC3);
                cvtColor(imageRaw8, imageRGB8, code);
                imageShow = imageRGB8;
                
        } else {
                imageShow = imageRaw8;
        }

        int width = image->width()*m_roiZoom;
        int height = image->height()*m_roiZoom;
        int x = m_roiX/(m_roiZoom*2.0);
        int y = m_roiY/(m_roiZoom*2.0);
        if (x + width > image->width()) {
                x = image->width() - width;
        }
        if (y + height > image->height()) {
                y = image->height() - height;
        }
        Rect roi(x, y, width, height);
        Mat zoomedImage = imageShow(roi);

        drawImageInfo(zoomedImage, image, 0, 0);
        drawRoiZoom(zoomedImage, image, x, y, m_roiZoom, 0, 1);
        drawControl(zoomedImage, "Exposure", m_exposure, "us", 0, 3);
        drawControl(zoomedImage, "Gain", m_gain, "mdB", 0, 4);

        imshow(WINDOW, zoomedImage);
}

void Viewer::onMouse(int event, int x, int y, int flags)
{
        switch (event) {
        case EVENT_LBUTTONDOWN:
                m_xStart = x;
                m_yStart = y;
                m_xLast = x;
                m_yLast = y;
                m_roiZoomStart = m_roiZoom;
                m_exposureStart = m_exposure;
                m_gainStart = m_gain;
                break;
        }

        int xDelta = x - m_xLast;
        m_xLast = x;
        int yDelta = y - m_yLast;
        m_yLast = y;

        int xAbsDelta = abs(x - m_xStart);
        int yAbsDelta = abs(m_yStart - y);

        if (flags & EVENT_FLAG_LBUTTON && flags & EVENT_FLAG_SHIFTKEY) {
                if (abs(xAbsDelta) > abs(yAbsDelta)) {
                        yDelta = 0;
                        m_exposure = m_exposureStart;
                } else {
                        xDelta = 0;
                        m_gain = m_gainStart;
                }

                if (m_exposure - yDelta > 0) {
                        m_exposure -= 100*yDelta;
                } else {
                        m_exposure = 0;
                }
                m_imageSource->setExposure(m_exposure);
                if (m_gain + xDelta > 0) {
                        m_gain += 10*xDelta;
                } else {
                        m_gain = 0;
                }
                m_imageSource->setGain(m_gain);

        } else if (flags & EVENT_FLAG_LBUTTON && flags & EVENT_FLAG_CTRLKEY) {
                float zoomDelta = m_roiZoom * 0.001 * yDelta;
                float roiZoom = m_roiZoomStart += zoomDelta;
                if (roiZoom < 0.01) {
                        m_roiZoom = 0.01;
                } else if (roiZoom > 1.0) {
                        m_roiZoom = 1.0;
                } else {
                        m_roiZoom = roiZoom;
                }

        } else if (flags & EVENT_FLAG_LBUTTON) {
                if (m_roiX - xDelta > 0) {
                        m_roiX -= xDelta;
                } else {
                        m_roiX = 0;
                }
                if (m_roiY - yDelta > 0) {
                        m_roiY -= yDelta;
                } else {
                        m_roiY = 0;
                }
        }
}