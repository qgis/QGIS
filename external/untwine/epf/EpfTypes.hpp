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

#include <pdal/Dimension.hpp>
#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../untwine/FileDimInfo.hpp"
#include "../untwine/VoxelKey.hpp"

namespace untwine
{
namespace epf
{

using DataVec = std::vector<uint8_t>;
using DataVecPtr = std::unique_ptr<DataVec>;
using Totals = std::unordered_map<VoxelKey, size_t>;
constexpr int MaxPointsPerNode = 100000;
constexpr int BufSize = 4096 * 10;
constexpr int MaxBuffers = 1000;
constexpr int NumWriters = 4;
constexpr int NumFileProcessors = 8;

struct FileInfo
{
    FileInfo() : numPoints(0), start(0)
    {}

    std::string filename;
    std::string driver;
    DimInfoList dimInfo;
    uint64_t numPoints;
    uint64_t start;
    pdal::BOX3D bounds;
    pdal::SpatialReference srs;

    bool valid() const
    { return filename.size(); }
};

} // namespace epf
} // namespace untwine
