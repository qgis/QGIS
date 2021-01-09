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

#include <stdexcept>

#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include "../untwine/FileDimInfo.hpp"

namespace untwine
{
namespace bu
{

struct BaseInfo
{
    pdal::BOX3D bounds;
    pdal::BOX3D trueBounds;
    size_t pointSize;
    std::string inputDir;
    std::string outputDir;
    int maxLevel;
    DimInfoList dimInfo;
    pdal::SpatialReference srs;
    bool stats;
};

} // namespace bu
} // namespace untwine
