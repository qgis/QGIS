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

#include "Grid.hpp"
#include "untwine/ProgressWriter.hpp"
#include "untwine/ThreadPool.hpp"

namespace untwine
{

struct FileInfo;
class ProgressWriter;

namespace epf
{

class Writer;

class Epf
{
public:
    Epf(BaseInfo& common);
    ~Epf();

    void run(ProgressWriter& progress, std::vector<FileInfo>& fileInfos);

private:
    BaseInfo& m_b;
    Grid m_grid;
    std::unique_ptr<Writer> m_writer;
    ThreadPool m_pool;
};

} // namespace epf
} // namespace untwine
