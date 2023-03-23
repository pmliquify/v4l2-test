#pragma once

#include <string>


class CommandArgs
{
public:
        CommandArgs(int argc, const char *argv[]);

        bool exists(const std::string& option);
        std::string option(const std::string &option, const std::string def = "");
        int optionInt(const std::string &option, int def = 0);
        
private:
        int m_argc;
        const char **m_argv;
};