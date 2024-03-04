#include <utils/commandargsconsumer.hpp>


void CommandArgsConsumer::printArgSection(std::string section)
{
        printf("\n");
        printf("%s options:\n", section.c_str());
}

void CommandArgsConsumer::printArg(std::string key, std::string desciption)
{
        printf("  %-20s  %s\n", key.c_str(), desciption.c_str());
}