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

#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>

#include "BuTypes.hpp"
#include "PointAccessor.hpp"
#include "VoxelInfo.hpp"

namespace untwine
{

class GridKey;

namespace bu
{

class OctantInfo;
class PyramidManager;

class Processor
{
public:
    Processor(PyramidManager& manager, const VoxelInfo&, const BaseInfo& b);

    void run();

private:
    using Index = std::deque<int>;
    using IndexIter = Index::const_iterator;

    void sample(Index& accepted, Index& rejected);
    void write(Index& accepted, Index& rejected);
    bool acceptable(int pointId, GridKey key);
    bool tooClose(pdal::PointId id1, pdal::PointId id2);

    void appendRemainder(Index& index);
    void writeBinOutput(Index& index);
    void writeCompressedOutput(Index& index);
    IndexIter writeOctantCompressed(const OctantInfo& o, Index& index, IndexIter pos);
    void appendCompressed(pdal::PointViewPtr view, const DimInfoList& dims, const FileInfo& fi,
        IndexIter begin, IndexIter end);
    void flushCompressed(pdal::PointTableRef table, pdal::PointViewPtr view,
        const OctantInfo& oi);

    VoxelInfo m_vi;
    const BaseInfo& m_b;
    PyramidManager& m_manager;
    PointAccessor m_points;
};

} // namespace bu
} // namespace untwine
