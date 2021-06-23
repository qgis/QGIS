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

#include <list>

#include "../untwine/MapFile.hpp"

namespace untwine
{
namespace bu
{

class FileInfo
{
public:
    FileInfo(const std::string& filename, size_t numPoints) :
        m_filename(filename), m_numPoints(numPoints)
    {}

    std::string filename() const
        { return m_filename; }

    size_t numPoints() const
        { return m_numPoints; }

    // When sampling we give a single set of indices to the points in the various
    // input files. m_start is the starting index of the points in this file.
    void setStart(size_t start)
        { m_start = start; }
    size_t start() const
        { return m_start; }

    char *address() const
        { return reinterpret_cast<char *>(m_ctx.addr()); }

    MapContext context() const
        { return m_ctx; }
    void setContext(const MapContext& ctx)
        { m_ctx = ctx; }

private:
    std::string m_filename;
    size_t m_numPoints;
    size_t m_start;
    MapContext m_ctx;
};
using FileInfoList = std::list<FileInfo>;

} // namespace bu
} // namespace untwine
