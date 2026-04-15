#include <runners/singletriggertestrunner.hpp>
#include <unistd.h>
#include <chrono>


SingleTriggerTestRunner::SingleTriggerTestRunner() :
        SocketClientRunner(),
        m_executionTime(0),
        m_startTime(0)
{
}

void SingleTriggerTestRunner::printArgs()
{
        SocketClientRunner::printArgs();
}

int SingleTriggerTestRunner::setup(CommandArgs &args)
{
        SocketClientRunner::setup(args);
        return 0;
}

void SingleTriggerTestRunner::prepareNextImage(ImageSource *imageSource)
{
        printf("Set single trigger for next image\n");
        uint64_t startTriggerTime = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        imageSource->setSingleTrigger();
        m_startTime = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        m_executionTime = m_startTime - startTriggerTime;
}

int SingleTriggerTestRunner::processImage(ImageSource *imageSource, Image *image)
{
        uint64_t elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() - m_startTime;
        printf("Elapsed time between trigger and image reception: %lu us (execution time: %lu us)\n", 
                elapsedTime, m_executionTime);

        return SocketClientRunner::processImage(imageSource, image);
}