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

#include <vector>

#include <pdal/util/Bounds.hpp>

#include "untwine/Common.hpp"

namespace pdal
{
class LasReader;
class PointLayout;
class Stage;
}

namespace untwine
{

struct Options;
struct FileInfo;

namespace prep
{

class Writer;

class FilePrep
{
public:
    FilePrep(BaseInfo& common);
    ~FilePrep();

    std::vector<FileInfo> run();

private:
    void createFileInfos(const StringList& input, std::vector<FileInfo>& fileInfos);
    void filterDims(std::vector<FileInfo>& infos, StringList allowedDims);
    void determineDims(std::vector<FileInfo>& infos, pdal::PointLayout& layout);
    void determineSrs(const std::vector<FileInfo>& infos);
    void determineScale(const std::vector<FileInfo>& infos);
    void determineBounds();
    void determineOffset(const std::vector<FileInfo>& infos);
    bool determineOffsetFromInfos(const std::vector<FileInfo>& infos);
    void determineOffsetFromBounds();
    std::vector<FileInfo> processLas(pdal::LasReader& reader, FileInfo fi);
    FileInfo processGeneric(pdal::Stage& reader, FileInfo fi);
    void fillMetadata(const pdal::PointLayout& layout);
    void calcCreationDay();

    BaseInfo& m_b;
    pdal::BOX3D m_trueBounds;
};

} // namespace prep
} // namespace untwine
