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

#include <cstdint>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <memory>

#include "EpfTypes.hpp"
#include "../untwine/Point.hpp"
#include "../untwine/VoxelKey.hpp"

namespace untwine
{
namespace epf
{

class Writer;

// A cell represents a voxel that contains points. All cells are the same size. A cell has
// a buffer which is filled by points. When the buffer is filled, it's passed to the writer
// and a new buffer is created.
class Cell
{
public:
    using FlushFunc = std::function<void(Cell *)>;

    Cell(const VoxelKey& key, int pointSize, Writer *writer, FlushFunc flush) :
        m_key(key), m_pointSize(pointSize), m_writer(writer), m_flush(flush)
    {
        assert(pointSize < BufSize);
        initialize();
    }
    ~Cell()
    {
        write();
    }

    void initialize();
    Point point()
        { return Point(m_pos); }
    VoxelKey key() const
        { return m_key; }
    void copyPoint(Point& b)
        { std::copy(b.data(), b.data() + m_pointSize, m_pos); }
    void advance();

private:
    DataVecPtr m_buf;
    VoxelKey m_key;
    int m_pointSize;
    Writer *m_writer;
    uint8_t *m_pos;
    uint8_t *m_endPos;
    FlushFunc m_flush;

    void write();
};

class CellMgr
{
public:
    CellMgr(int pointSize, Writer *writer);

    Cell *get(const VoxelKey& key);
    void flush(Cell *exclude);

private:
    using CellMap = std::unordered_map<VoxelKey, std::unique_ptr<Cell>>;
    int m_pointSize;
    Writer *m_writer;
    CellMap m_cells;
};


} // namespace epf
} // namespace untwine
