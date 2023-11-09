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
#include "Point.hpp"
#include "TileKey.hpp"

namespace untwine
{
namespace epf
{

class Writer;
class CellMgr;

// A cell represents a voxel that contains points. All cells are the same size. A cell has
// a buffer which is filled by points. When the buffer is filled, it's passed to the writer
// and a new buffer is created.
class Cell
{
public:
    using FlushFunc = std::function<void(Cell *)>;

    Cell(const TileKey& key, int pointSize, Writer *writer, CellMgr *mgr, const Cell *lastCell) :
        m_key(key), m_pointSize(pointSize), m_writer(writer), m_cellMgr(mgr)
    {
        assert(pointSize < BufSize);
        initialize(lastCell);
    }
    ~Cell()
    {
        write();
    }

    void initialize(const Cell *exclude);
    Point point()
        { return Point(m_pos); }
    TileKey key() const
        { return m_key; }
    void copyPoint(Point& b)
        { std::copy(b.data(), b.data() + m_pointSize, m_pos); }
    void advance();

private:
    DataVecPtr m_buf;
    TileKey m_key;
    uint8_t *m_pos;
    uint8_t *m_endPos;
    int m_pointSize;
    Writer *m_writer;
    CellMgr *m_cellMgr;

    void write();
};

class CellMgr
{
    friend class Cell;
public:
    CellMgr(int pointSize, Writer *writer);

    Cell *get(const TileKey& key, const Cell *lastCell = nullptr);

private:
    using CellMap = std::unordered_map<TileKey, std::unique_ptr<Cell>>;
    int m_pointSize;
    Writer *m_writer;
    CellMap m_cells;

    DataVecPtr getBuffer(const Cell* exclude);
    void flush(const Cell *exclude);
};


} // namespace epf
} // namespace untwine
