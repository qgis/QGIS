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

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <pdal/util/ThreadPool.hpp>

#include "BuTypes.hpp"
#include "OctantInfo.hpp"

namespace untwine
{

class ProgressWriter;

namespace bu
{

class OctantInfo;
class Processor;

class PyramidManager
{
    using Entries = std::vector<std::pair<VoxelKey, int>>;
public:
    PyramidManager(const BaseInfo& b);
    ~PyramidManager();

    void setProgress(ProgressWriter *progress);
    void queue(const OctantInfo& o);
    void run();
    void logOctant(const VoxelKey& k, int cnt);
    uint64_t totalPoints() const
        { return m_totalPoints; }

private:
    const int LevelBreak = 4;
    const int MinHierarchySize = 50;
    const BaseInfo& m_b;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::unordered_map<VoxelKey, OctantInfo> m_completes;
    std::queue<OctantInfo> m_queue;
    pdal::ThreadPool m_pool;
    uint64_t m_totalPoints;
    ProgressWriter *m_progress;
    //
    std::unordered_map<VoxelKey, int> m_written;
    std::unordered_map<VoxelKey, int> m_childCounts;

    void getInputFiles();
    void process(const OctantInfo& o);
    void addComplete(const OctantInfo& o);
    bool childrenComplete(const VoxelKey& parent);
    OctantInfo removeComplete(const VoxelKey& k);
    //
    void createHierarchy();
    std::deque<VoxelKey> emitRoot(const VoxelKey& root);
    std::deque<VoxelKey> emit(const VoxelKey& p, int stopLevel, Entries& entries);
};

} // namespace bu
} // namespace untwine
