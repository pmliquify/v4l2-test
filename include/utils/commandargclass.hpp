#pragma once

#include <commandargs.hpp>


class CommandArgClass
{
public:
    virtual void printArgs() = 0;
    virtual int setup(CommandArgs &args) = 0;

protected:
    void printArgSection(std::string section);
    void printArg(std::string key, std::string desciption);
};