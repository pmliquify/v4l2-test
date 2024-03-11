#include <gui/viewer.hpp>
#include <opencv2/opencv.hpp>
#include "convert.hpp"
#include "draw.hpp"
#include "mainwindow.hpp"

MainWindow mainWindow("v4l2-test");


Viewer::Viewer()
{
}

Viewer::~Viewer()
{
}

void Viewer::init(ImageSource *imageSource)
{
        mainWindow.init(imageSource);
}

void Viewer::show(Image *image)
{
        Mat img = convert(image);
        drawImageInfo(img, image, 1, 1);

        mainWindow.show(img);
}

void Viewer::hide()
{
        mainWindow.hide();
}