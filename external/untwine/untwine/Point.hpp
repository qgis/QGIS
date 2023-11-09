/*****************************************************************************
 *   Copyright (c) 2020, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/


#pragma once

namespace untwine
{

// Utterly trivial wrapper around a pointer.
class Point
{
public:
    Point() : m_data(nullptr)
    {}

    Point(uint8_t *data) : m_data(data)
    {}
    Point(char *data) : m_data(reinterpret_cast<uint8_t *>(data))
    {}

    uint8_t *data()
        { return m_data; }
    double x() const
        {
            double d;
            memcpy(&d, ddata(), sizeof(d));
            return d;
        }
    double y() const
        {
            double d;
            memcpy(&d, ddata() + 1, sizeof(d));
            return d;
        }
    double z() const
        {
            double d;
            memcpy(&d, ddata() + 2, sizeof(d));
            return d;
        }

    char *cdata() const
        { return reinterpret_cast<char *>(m_data); }
    double *ddata() const
        { return reinterpret_cast<double *>(m_data); }

private:
    uint8_t *m_data;
};

} // namespace untwine
