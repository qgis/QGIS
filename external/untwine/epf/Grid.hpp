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

#include <pdal/util/Bounds.hpp>

#include "../untwine/Common.hpp"
#include "../untwine/VoxelKey.hpp"

namespace untwine
{
namespace epf
{

class Grid
{
public:
    Grid(const pdal::BOX3D& bounds, uint64_t numPoints, int level, bool cubic) :
        m_bounds(bounds)
    {
        if (level == -1)
            level = calcLevel(numPoints, cubic);
        resetLevel(level);
    }

    VoxelKey key(double x, double y, double z) const;
    int calcLevel(uint64_t numPoints, bool cubic) const;
    void resetLevel(int level);
    int maxLevel() const
        { return m_maxLevel; }

private:
    pdal::BOX3D m_bounds;
    int m_gridSize;
    int m_maxLevel;
    double m_xsize;
    double m_ysize;
    double m_zsize;
};

} // namespace epf
} // namespace untwine
