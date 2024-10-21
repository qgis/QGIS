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


// This include is necessary for released PDAL 2.0 and earlier, as it wasn't included in
// FileUtils.hpp.
#include <vector>
#include <pdal/util/FileUtils.hpp>

#include "Reprocessor.hpp"
#include "../untwine/Common.hpp"

#include <mapfile.hpp>  // untwine/os

namespace untwine
{
namespace epf
{

Reprocessor::Reprocessor(const VoxelKey& k, int numPoints, int pointSize,
        const std::string& outputDir, Grid grid, Writer *writer) :
    m_pointSize(pointSize), m_numPoints(numPoints), m_fileSize(pointSize * (size_t)numPoints),
    m_grid(grid), m_mgr(pointSize, writer)
{
    // We make an assumption that at most twice the number of points will be in a cell
    // than there would be if the distribution was uniform, so we calculate based on
    // each level breaking the points into 4.
    // So, to find the number of levels, we need to solve for n:
    //
    // numPoints / (4^n) = MaxPointsPerNode
    //  =>
    // numPoints / MaxPointsPerNode = 2^(2n)
    //  =>
    // log2(numPoints / MaxPointsPerNode) = 2n

    m_levels = (int)std::ceil(log2((double)numPoints / MaxPointsPerNode) / 2);

    // We're going to steal points from the leaf nodes for sampling, so unless the
    // spatial distribution is really off, this should be fine and pretty conservative.

    m_grid.resetLevel(m_grid.maxLevel() + m_levels);
    m_filename = outputDir + "/" + k.toString() + ".bin";
}

void Reprocessor::run()
{
    auto ctx = os::mapFile(m_filename, true, 0, m_fileSize);
    if (ctx.addr() == nullptr)
    {
        std::cerr << "FATAL: " + m_filename + ": " + ctx.what();
        exit(-1);
    }

    // Wow, this is simple. How nice. The writer should get invoked automatically.
    uint8_t *pos = reinterpret_cast<uint8_t *>(ctx.addr());
    for (size_t i = 0; i < m_numPoints; ++i)
    {
        Point p(pos);
        VoxelKey k = m_grid.key(p.x(), p.y(), p.z());
        Cell *cell = m_mgr.get(k);
        cell->copyPoint(p);
        cell->advance();
        pos += m_pointSize;
    }
    os::unmapFile(ctx);
    pdal::FileUtils::deleteFile(m_filename);
}

} // namespace epf
} // namespace untwine
