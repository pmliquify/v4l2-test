#pragma once

#include <vector>


class Plane 
{
public:
        Plane();

        void init(unsigned char *data, unsigned int size);
        void init(unsigned int size);

        const unsigned char * data() const { return m_data; }
        unsigned char * data() { return m_data; }
        unsigned int size() const { return m_size; }

private:
        unsigned char * m_data;
        unsigned int    m_size;
};

typedef std::vector<Plane> Planes;