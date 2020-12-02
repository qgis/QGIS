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

#include <string>
#include <unordered_map>
#include <vector>

#include "PyramidManager.hpp"
#include "BuTypes.hpp"

namespace pdal
{
    class ProgramArgs;
}

namespace untwine
{

struct Options;
class ProgressWriter;

namespace bu
{

class FileInfo;

class BuPyramid
{
public:
    BuPyramid();
    void run(const Options& options, ProgressWriter& progress);

private:
    void getInputFiles();
    void readBaseInfo();
    size_t queueWork();
    void writeInfo();

    PyramidManager m_manager;
    BaseInfo m_b;
    std::unordered_map<VoxelKey, FileInfo> m_allFiles;
};

} // namespace bu
} // namespace untwine
