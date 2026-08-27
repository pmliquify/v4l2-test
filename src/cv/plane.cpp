/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#include <cv/plane.hpp>
#include <cstdlib>


Plane::Plane() :
        m_data(0),
        m_size(0)
{
        
}

void Plane::init(unsigned char *data, unsigned int size)
{
        m_data = data;
        m_size = size;
}

void Plane::init(unsigned int size)
{
        if (m_size != size) {
                if (m_data) {
                        free(m_data);
                }
                m_data = (unsigned char *)malloc(size);
                m_size = size;
        }
}