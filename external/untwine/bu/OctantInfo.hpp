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

#include "../untwine/VoxelKey.hpp"

#include "FileInfo.hpp"

namespace untwine
{
namespace bu
{

class OctantInfo
{
public:
    OctantInfo() : m_mustWrite(false)
    {}

    OctantInfo(const VoxelKey& key) : m_mustWrite(false)
        { m_key = key; }

    void appendFileInfo(const FileInfo& fi)
        { m_fileInfos.push_back(fi); }

    void prependFileInfo(const FileInfo& fi)
        { m_fileInfos.push_front(fi); }

    void appendFileInfos(OctantInfo& o)
        { m_fileInfos.splice(m_fileInfos.end(), o.m_fileInfos); }

    size_t numPoints() const
    {
        size_t cnt = 0;
        for (const FileInfo& fi : m_fileInfos)
            cnt += fi.numPoints();
        return cnt;
    }

    bool hasPoints() const
    {
        for (const FileInfo& fi : m_fileInfos)
            if (fi.numPoints())
                return true;
        return false;
    }

    void mergeSmallFiles(const std::string tempDir, size_t pointSize);

    std::list<FileInfo>& fileInfos()
        { return m_fileInfos; }
    const std::list<FileInfo>& fileInfos() const
        { return m_fileInfos; }
    VoxelKey key() const
        { return m_key; }
    void setKey(VoxelKey k)
        { m_key = k; }
    bool mustWrite() const
        { return m_mustWrite; }
    void setMustWrite(bool mustWrite)
        { m_mustWrite = mustWrite; }

private:
    FileInfoList m_fileInfos;
    VoxelKey m_key;
    bool m_mustWrite;
};

} // namespace bu
} // namespace untwine
