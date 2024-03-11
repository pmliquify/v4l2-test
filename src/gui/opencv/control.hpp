#pragma once

#include "text.hpp"
#include "mouseevent.hpp"
#include <map>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef map<int, String> DescriptionMap;


class Control
{
public:
    Control(const char *name, const char *unit, int floatPrecision,
        double defaultValue, double step, double min, double max,
        int x, int y, DescriptionMap descMap = DescriptionMap());

    String name() const { return m_name; }
    double value();

    void draw(Mat img);

    bool hasChanged() const { return m_hasChanged;  }
    void onMouse(const MouseEvent& event);

private:
    String         m_name;
    String         m_unit;
    DescriptionMap m_descMap;
    int            m_floatPrecision;
    Text           m_text;
    bool           m_mouseOver;
    double         m_default;
    double         m_value;
    double         m_step;
    double         m_min;
    double         m_max;
    bool           m_hasChanged;
};

typedef std::map<std::string, Control *> ControlMap;