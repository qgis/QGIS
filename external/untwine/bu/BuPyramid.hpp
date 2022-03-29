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
    BuPyramid(BaseInfo& common);
    void run(ProgressWriter& progress);

private:
    void getInputFiles();
    void readBaseInfo();
    size_t queueWork();
    void writeInfo();

    BaseInfo m_b;
    PyramidManager m_manager;
    std::unordered_map<VoxelKey, FileInfo> m_allFiles;
};

} // namespace bu
} // namespace untwine
