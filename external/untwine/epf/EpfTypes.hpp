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

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "untwine/VoxelKey.hpp"

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

} // namespace epf
} // namespace untwine
