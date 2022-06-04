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

#include "CopcSupport.hpp"
#include "OctantInfo.hpp"
#include "Stats.hpp"
#include "../untwine/ThreadPool.hpp"

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
    void queueWithError(const OctantInfo& o, const std::string& error);
    void run();
    void logOctant(const VoxelKey& k, int cnt, const IndexedStats& istats);
    uint64_t totalPoints() const
        { return m_totalPoints; }
    Stats *stats(pdal::Dimension::Id id);
    uint64_t newChunk(const VoxelKey& key, uint32_t size, uint32_t count);

private:
    const int LevelBreak = 4;
    const int MinHierarchySize = 50;
    const BaseInfo& m_b;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::unordered_map<VoxelKey, OctantInfo> m_completes;
    std::queue<OctantInfo> m_queue;
    ThreadPool m_pool;
    uint64_t m_totalPoints;
    StatsMap m_stats;
    ProgressWriter *m_progress;
    CopcSupport m_copc;
    std::string m_error;
    //
    std::unordered_map<VoxelKey, int> m_written;
    std::unordered_map<VoxelKey, int> m_childCounts;

    void getInputFiles();
    void process(const OctantInfo& o);
    void addComplete(const OctantInfo& o);
    bool childrenComplete(const VoxelKey& parent);
    OctantInfo removeComplete(const VoxelKey& k);
    size_t extraByteSize();
    //
    void createHierarchy();
    void writeHierarchy();
    std::deque<VoxelKey> emitRoot(const VoxelKey& root);
    std::deque<VoxelKey> emit(const VoxelKey& p, int stopLevel, Entries& entries);
};

} // namespace bu
} // namespace untwine
