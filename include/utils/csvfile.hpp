#pragma once

#include <fstream>
#include <string>

class CSVFile : public std::ofstream
{
public:
        CSVFile(std::string fileName) { std::ofstream::open(fileName); }
        ~CSVFile() { close(); }

        friend std::ofstream& operator<<(std::ofstream& os, std::string &value) { os << value << ","; return os; }
        friend std::ofstream& operator<<(std::ofstream& os, int value) { os << value << ","; return os; }
        friend std::ofstream& operator<<(std::ofstream& os, float value) { os << value << ","; return os; }
        friend std::ofstream& operator<<(std::ofstream& os, unsigned short value) { os << value << ","; return os; }
};