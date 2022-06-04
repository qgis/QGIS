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

#include "EpfTypes.hpp"
#include "BufferCache.hpp"
#include "../untwine/ThreadPool.hpp"
#include "../untwine/VoxelKey.hpp"

namespace untwine
{
namespace epf
{

// The writer has some number of threads that actually write data to the files for tiles. When
// a processor has a full tile (or a partial that it needs to discard/finish), it sticks it
// on the queue for one of the writer threads to pick up and process.
//
// We can't have multiple writer threads write to the same file simultaneously, so rather than
// lock (which might stall threads that could otherwise be working), we make sure that only
// one writer thread is working on a file at a time by sticking
// the key of the thread in an "active" list.  A writer thread looking for work will ignore
// any buffer on the queue that's for a file currently being handled by another writer thread.
// 
// The writer owns a buffer cache. The cache manages the actual data buffers that are filled
// by the file processors and written by a writer thread. The buffers are created as needed
// until some predefined number of buffers is hit in order to limit memory use.
// Once a writer is done with a buffer, it sticks it back on the cache
// and then notifes the some processor that a buffer is available in case the processor
// is waiting for a free buffer.
//
// Since processors try to hold onto buffers until they are full, there can be times at
// which the buffers are exhaused and no more are available, but none are ready to be
// written. In this case, the buffers for the processor needing a new buffer flushed to
// the queue even if they aren't full so that they can be reused. The active buffer for a
// flushing processor is reserved, so there need to be at least one more buffer than the
// number of file processors, though typically there are many more buffers than file processors.
//
// Buffers containing no points are never queued, but if a processor flush occurs, they are
// replaced on the buffer cache for reuse. Empty buffers can happen because if a cell has had
// its buffer written, it immediately grabs a new buffer even if it hasn't seen a point
// destined for that cell - we don't want to tear down the cell just to recreate it.
// The thinking is that if we've filled a buffer for a cell, there's
// probably at least one more point going to that cell from the source.
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

    void replace(DataVecPtr data);
    void enqueue(const VoxelKey& key, DataVecPtr data, size_t dataSize);
    void stop();
    const Totals& totals()
        { return m_totals; }
    Totals totals(size_t minSize);
    DataVecPtr fetchBuffer();
    DataVecPtr fetchBufferBlocking();

private:
    std::string path(const VoxelKey& key);
    void run();

    std::string m_directory;
    ThreadPool m_pool;
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
