#pragma once

#include <sources/imagesource.hpp>
#include <utils/commandargsconsumer.hpp>
#include <map>


class ImageSourceRunner : public CommandArgsConsumer
{
public:
        virtual int run(ImageSource *imageSource) = 0;
};

typedef std::map<std::string, ImageSourceRunner *> ImageSourceRunnerMap;