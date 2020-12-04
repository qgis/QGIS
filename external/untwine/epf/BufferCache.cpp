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


#include <mutex>

#include "BufferCache.hpp"

namespace untwine
{
namespace epf
{

// If we have a buffer in the cache, return it. Otherwise create a new one and return that.
DataVecPtr BufferCache::fetch()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_cv.wait(lock, [this](){ return m_buffers.size() || m_count < MaxBuffers; });
    if (m_buffers.size())
    {
        DataVecPtr buf(std::move(m_buffers.back()));
        m_buffers.pop_back();
        return buf;
    }

    // m_count tracks the number of created buffers. We only create MaxBuffers buffers.
    // If we've created that many, we wait until one is available.
    m_count++;
    return DataVecPtr(new DataVec(BufSize));
}

// Put a buffer back in the cache.
void BufferCache::replace(DataVecPtr&& buf)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    //ABELL - Fix this.
//    buf->resize(BufSize);
    m_buffers.push_back(std::move(buf));

    if (m_count == MaxBuffers)
    {
        lock.unlock();
        m_cv.notify_one();
    }
}

} // namespace epf
} // namespace untwine
