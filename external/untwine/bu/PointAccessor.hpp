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

#include "../untwine/Common.hpp"
#include "../untwine/MapFile.hpp"
#include "../untwine/Point.hpp"

#include "FileInfo.hpp"

namespace untwine
{
namespace bu
{

class PointAccessor
{
public:
    PointAccessor(const BaseInfo& b) : m_b(b)
    {}

    ~PointAccessor()
    {
        for (FileInfo *fi : m_fileInfos)
            unmapFile(fi->context());
    }

    void read(FileInfo& fi)
    {
        std::string filename = m_b.opts.tempDir + "/" + fi.filename();
        auto ctx = mapFile(filename, true, 0, fi.numPoints() * m_b.pointSize);
        if (ctx.m_addr == nullptr)
            throw FatalError(filename + ": " + ctx.m_error);
        fi.setContext(ctx);
        fi.setStart(size());
        m_fileInfos.push_back(&fi);
    }

    Point operator[](size_t offset)
    {
        for (FileInfo *fi : m_fileInfos)
            if (offset >= (size_t)fi->start() &&
                offset < (size_t)fi->start() + fi->numPoints())
                return Point(fi->address() + ((offset - fi->start()) * m_b.pointSize));
        return Point();
    }

    size_t size()
    {
        if (m_fileInfos.empty())
            return 0;
        else
            return m_fileInfos.back()->start() + m_fileInfos.back()->numPoints();
    }

    void dump()
    {
        std::cerr << "Accessor infos:\n";
        for (FileInfo *fi : m_fileInfos)
            std::cerr << fi->filename() << "/" << fi->start() << "/" << fi->numPoints() << "!\n";
        std::cerr << "\n";
    }

private:
    const BaseInfo& m_b;
    std::vector<FileInfo *> m_fileInfos;
};

} // namespace bu
} // namespace untwine
