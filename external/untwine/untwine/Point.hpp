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

#include "Common.hpp"

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

    void quantize(const Transform& xform)
    {
        auto quant = [](double d, double scale, double offset) -> double
        {
            return std::round((d - offset) / scale) * scale + offset;
        };

        x(quant(x(), xform.scale.x, xform.offset.x));
        y(quant(y(), xform.scale.y, xform.offset.y));
        z(quant(z(), xform.scale.z, xform.offset.z));
    }

    char *cdata() const
        { return reinterpret_cast<char *>(m_data); }
    double *ddata() const
        { return reinterpret_cast<double *>(m_data); }

private:
    uint8_t *m_data;

    void x(double d)
    {
        memcpy(ddata(), &d, sizeof(d));
    }

    void y(double d)
    {
        memcpy(ddata() + 1, &d, sizeof(d));
    }

    void z(double d)
    {
        memcpy(ddata() + 2, &d, sizeof(d));
    }

};

} // namespace untwine
