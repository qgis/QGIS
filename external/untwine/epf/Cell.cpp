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


#include "Cell.hpp"
#include "Writer.hpp"

namespace untwine
{
namespace epf
{

void Cell::initialize()
{
    m_buf = m_writer->fetchBuffer();

    // If we couldn't fetch a buffer, flush all the the buffers for this processor and
    // try again, but block.
    if (!m_buf)
    {
        m_flush(this);
        m_buf = m_writer->fetchBufferBlocking();
    }
    m_pos = m_buf->data();

    m_endPos = m_pos + m_pointSize * (BufSize / m_pointSize);
}

// NOTE - After write(), the cell is invalid and must be initialized or destroyed.
void Cell::write()
{
    size_t size = m_pos - m_buf->data();
    if (size)
        m_writer->enqueue(m_key, std::move(m_buf), size);
    else
        m_writer->replace(std::move(m_buf));
}

void Cell::advance()
{
    m_pos += m_pointSize;
    if (m_pos >= m_endPos)
    {
        write();
        initialize();
    }
}

///
/// CellMgr
///

CellMgr::CellMgr(int pointSize, Writer *writer) : m_pointSize(pointSize), m_writer(writer)
{}


Cell *CellMgr::get(const VoxelKey& key)
{
    auto it = m_cells.find(key);
    if (it == m_cells.end())
    {
        Cell::FlushFunc f = [this](Cell *exclude)
        {
            flush(exclude);
        };
        std::unique_ptr<Cell> cell(new Cell(key, m_pointSize, m_writer, f));
        it = m_cells.insert( {key, std::move(cell)} ).first;
    }
    Cell& c = *(it->second);
    return &c;
}

// Eliminate all the cells and their associated data buffers except the `exclude`
// cell.
void CellMgr::flush(Cell *exclude)
{
    CellMap::iterator it = m_cells.end();
    if (exclude)
        it = m_cells.find(exclude->key());

    // If there was no exclude cell or it isn't in our list, just clear the cells.
    // Otherwise, save the exclude cell, clear the list, and reinsert.
    if (it == m_cells.end())
        m_cells.clear();
    else
    {
        std::unique_ptr<Cell> c = std::move(it->second);
        m_cells.clear();
        m_cells.insert({c->key(), std::move(c)});
    }
}

} // namespace epf
} // namespace untwine
