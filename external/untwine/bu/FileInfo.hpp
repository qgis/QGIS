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
    FileInfo(const std::string& filename, int numPoints) :
        m_filename(filename), m_numPoints(numPoints)
    {}

    std::string filename() const
        { return m_filename; }

    int numPoints() const
        { return m_numPoints; }

    // When sampling we give a single set of indices to the points in the various
    // input files. m_start is the starting index of the points in this file.
    void setStart(int start)
        { m_start = start; }
    int start() const
        { return m_start; }

    char *address() const
        { return reinterpret_cast<char *>(m_ctx.addr()); }

    MapContext context() const
        { return m_ctx; }
    void setContext(const MapContext& ctx)
        { m_ctx = ctx; }

private:
    std::string m_filename;
    int m_numPoints;
    int m_start;
    MapContext m_ctx;
};
using FileInfoList = std::list<FileInfo>;

} // namespace bu
} // namespace untwine
