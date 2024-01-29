#include "isptest.hpp"
#include <framebuffer.hpp>
#include <v4l2autoexposure.hpp>


IspTest::IspTest()
{

}

void IspTest::printArgs()
{
        printArgSection("ISP test");
}

int IspTest::setup(CommandArgs &args)
{
        return 0;
}

int IspTest::exec(V4L2ImageSource &imageSource, V4L2Image &image) 
{
        return 0;
}