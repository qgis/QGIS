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

#include "TileKey.hpp"


namespace untwine
{
namespace epf
{

class TileGrid
{
public:

    void setTileLength( double len ) { m_tileLength = len; }

    void expand(const pdal::BOX3D& bounds, size_t points);
    TileKey key(double x, double y, double z) const;
    pdal::BOX3D conformingBounds() const
        { return m_bounds; }

private:
    double m_tileLength = 100;
    int m_gridSizeX = 0;
    int m_gridSizeY = 0;
    pdal::BOX3D m_bounds;
    size_t m_millionPoints = 0;
};

} // namespace epf
} // namespace untwine
