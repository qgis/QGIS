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


#include "EpfTypes.hpp"
#include "TileGrid.hpp"
#include "Cell.hpp"

struct ProgressBar;

namespace untwine
{

namespace epf
{

class Writer;

// Processes a single input file (FileInfo) and writes data to the Writer.
class FileProcessor
{
public:
    FileProcessor(const FileInfo& fi, size_t pointSize, const TileGrid& grid, Writer *writer,
        ProgressBar& progressBar);

    Cell *getCell(const TileKey& key);
    void run();

private:
    FileInfo m_fi;
    CellMgr m_cellMgr;
    TileGrid m_grid;
    ProgressBar& m_progressBar;
};

} // namespace epf
} // namespace untwine
