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

#include <pdal/PointLayout.hpp>
#include <pdal/SpatialReference.hpp>

#include "EpfTypes.hpp"
#include "Grid.hpp"
#include "../untwine/ProgressWriter.hpp"
#include "../untwine/ThreadPool.hpp"

namespace pdal
{
class LasReader;
class Stage;
}

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
    Epf(BaseInfo& common);
    ~Epf();

    void run(ProgressWriter& progress);

private:
    void createFileInfos(const StringList& input, std::vector<FileInfo>& fileInfos);
    void filterDims(std::vector<FileInfo>& infos, StringList allowedDims);
    void determineDims(std::vector<FileInfo>& infos, pdal::PointLayout& layout);
    void determineOffset(const std::vector<FileInfo>& infos);
    pdal::SpatialReference determineSrs(const std::vector<FileInfo>& infos);
    std::vector<FileInfo> processLas(pdal::LasReader& reader, FileInfo fi);
    FileInfo processGeneric(pdal::Stage& reader, FileInfo fi);
    void calcCreationDay();
    void fillMetadata(const pdal::PointLayout& layout);

    BaseInfo& m_b;
    Grid m_grid;
    std::unique_ptr<Writer> m_writer;
    ThreadPool m_pool;
    FileInfo m_srsFileInfo;
};

} // namespace epf
} // namespace untwine
