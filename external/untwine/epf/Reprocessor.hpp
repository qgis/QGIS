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

#include "EpfTypes.hpp"
#include "Grid.hpp"
#include "Cell.hpp"

namespace untwine
{
namespace epf
{

class Writer;

class Reprocessor
{
public:
    Reprocessor(const VoxelKey& k, int numPoints, int pointSize, const std::string& outputDir,
        Grid grid, Writer *writer);

    void run();

private:
    int m_pointSize;
    uint64_t m_numPoints;
    uint64_t m_fileSize;
    Grid m_grid;
    int m_levels;
    CellMgr m_mgr;
    std::string m_filename;
};

} // namespace epf
} // namespace untwine
