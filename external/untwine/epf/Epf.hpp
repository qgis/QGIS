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

#include <map>
#include <vector>

#include <pdal/SpatialReference.hpp>
#include <pdal/util/ThreadPool.hpp>

#include "EpfTypes.hpp"
#include "Grid.hpp"
#include "../untwine/ProgressWriter.hpp"

namespace untwine
{

struct Options;
class ProgressWriter;

namespace epf
{

struct FileInfo;
class Writer;

class Epf
{
public:
    Epf();
    ~Epf();

    void run(const Options& options, ProgressWriter& progress);

private:
    void createFileInfo(const pdal::StringList& input, std::vector<FileInfo>& fileInfos);

    Grid m_grid;
    std::unique_ptr<Writer> m_writer;
    pdal::ThreadPool m_pool;
    size_t m_fileLimit;
    FileInfo m_srsFileInfo;
};

} // namespace epf
} // namespace untwine
