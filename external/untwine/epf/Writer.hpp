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

#include <condition_variable>
#include <list>
#include <mutex>
#include <string>

#include <pdal/util/ThreadPool.hpp>

#include "EpfTypes.hpp"
#include "BufferCache.hpp"
#include "../untwine/VoxelKey.hpp"

namespace untwine
{
namespace epf
{

class Writer
{
    struct WriteData
    {
        VoxelKey key;
        DataVecPtr data;
        size_t dataSize;
    };

public:
    Writer(const std::string& directory, int numThreads, size_t pointSize);

    void enqueue(const VoxelKey& key, DataVecPtr data, size_t dataSize);
    void stop();
    BufferCache& bufferCache()
        { return m_bufferCache; }
    const Totals& totals()
        { return m_totals; }
    Totals totals(size_t minSize);

private:
    std::string path(const VoxelKey& key);
    void run();

    std::string m_directory;
    pdal::ThreadPool m_pool;
    BufferCache m_bufferCache;
    bool m_stop;
    size_t m_pointSize;
    std::list<WriteData> m_queue;
    std::list<VoxelKey> m_active;
    Totals m_totals;
    std::mutex m_mutex;
    std::condition_variable m_available;
};

} // namespace epf
} // namespace untwine
