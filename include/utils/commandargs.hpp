#pragma once

#include <string>
#include <array>
#include <cstddef>
#include <algorithm>


class CommandArgs
{
public:
    CommandArgs(int argc, const char *argv[]);

    bool exists(const std::string& option);
    bool noOptionMatched();
    std::string option(const std::string &option, const std::string defaultValue = "");
    int optionInt(const std::string &option, int defaultValue = 0);
    double optionDouble(const std::string &option, double defaultValue = 0.0);

    template <typename T, std::size_t N>
    std::array<T, N> optionArray(const std::string &option, std::array<T, N> defaultValue = {}) {
        if (exists(option)) {
            std::array<T, N> result{};
            std::string str = CommandArgs::option(option);
            size_t last = 0;
            size_t idx = 0;
            while (last <= str.length() && idx < N) {
                size_t next = str.find(',', last);
                std::string token;
                if (next == std::string::npos) {
                    token = str.substr(last);
                    
                } else {
                    token = str.substr(last, next - last);
                }
                if (token.empty()) {
                    result[idx++] = 0;

                } else {
                    try {
                        if (std::is_floating_point<T>::value) {
                            result[idx++] = static_cast<T>(std::stod(token));
                        } else if (std::is_integral<T>::value) {
                            result[idx++] = static_cast<T>(std::stoul(token));
                        } else {
                            result[idx++] = 0;
                        }
                    } catch (...) {
                        result[idx++] = 0;
                    }
                }
                if (next == std::string::npos) break;
                last = next + 1;
            }
            return result;
        }
        return defaultValue;
    }
    
private:
    int             m_argc;
    const char **   m_argv;
    bool            m_optionMatched;
};