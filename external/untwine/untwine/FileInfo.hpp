/*****************************************************************************
 *   Copyright (c) 2024, Hobu, Inc. (info@hobu.co)                           *
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

#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include "FileDimInfo.hpp"

namespace untwine
{

struct FileInfo
{
    FileInfo() :
        numPoints(0), start(0), untwineBitsOffset(-1), fileVersion(0)
    {}

    std::string filename;
    std::string driver;
    bool no_srs;
    DimInfoList dimInfo;
    uint64_t numPoints;
    uint64_t start;
    pdal::BOX3D bounds;
    pdal::SpatialReference srs;
    int untwineBitsOffset;
    // Currently only set for LAS files.
    int fileVersion;
    Transform xform;

    bool valid() const
    { return filename.size(); }
};

} // namespace untwine
