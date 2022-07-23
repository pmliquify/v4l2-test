#include "commandargs.hpp"
#include <algorithm>


CommandArgs::CommandArgs(int argc, char *argv[]) :
        m_argc(argc),
        m_argv(argv)
{
}

bool CommandArgs::exists(const std::string &option)
{
        char ** begin = m_argv;
        char ** end = m_argv + m_argc;

        return std::find(begin, end, option) != end;
}

std::string CommandArgs::option(const std::string &option, const std::string def)
{
        char ** begin = m_argv;
        char ** end = m_argv + m_argc;

        char ** itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) {
                return *itr;
        }
        return def;
}

int CommandArgs::optionInt(const std::string &option, int def)
{
        if (exists(option)) {
                return std::stoi(CommandArgs::option(option));
        }
        return def;
}