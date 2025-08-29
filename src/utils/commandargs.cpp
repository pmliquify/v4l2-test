#include <utils/commandargs.hpp>
#include <cctype>

CommandArgs::CommandArgs(int argc, const char *argv[]) :
        m_argc(argc),
        m_argv(argv),
        m_optionMatched(false)
{
}

bool CommandArgs::exists(const std::string &option)
{
        const char ** begin = m_argv;
        const char ** end = m_argv + m_argc;

        bool exists = std::find(begin, end, option) != end;
        if (exists) {
                m_optionMatched = true;
        }
        return exists;
}

bool CommandArgs::noOptionMatched()
{
        return !m_optionMatched;
}

std::string CommandArgs::option(const std::string &option, const std::string defaultValue)
{
        const char ** begin = m_argv;
        const char ** end = m_argv + m_argc;

        const char ** itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) {
                return *itr;
        }
        return defaultValue;
}

int CommandArgs::optionInt(const std::string &option, int defaultValue)
{
        if (exists(option)) {
                return std::stoi(CommandArgs::option(option));
        }
        return defaultValue;
}

double CommandArgs::optionDouble(const std::string &option, double defaultValue)
{
        if (exists(option)) {
                return std::stod(CommandArgs::option(option));
        }
        return defaultValue;
}
