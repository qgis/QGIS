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

// NOTE - After write(), the cell is invalid and must be initialized or destroyed.
void Cell::write()
{
    size_t size = m_pos - m_buf->data();
    if (size)
        m_writer->enqueue(m_key, std::move(m_buf), size);
    else
        m_writer->replace(std::move(m_buf));
}

void Cell::initialize(const Cell *exclude)
{
    m_buf = m_cellMgr->getBuffer(exclude);
    if (!m_buf)
        throw FatalError("Stopping due to writer failure.");
    m_pos = m_buf->data();
    m_endPos = m_pos + m_pointSize * (BufSize / m_pointSize);
}

void Cell::advance()
{
    m_pos += m_pointSize;
    if (m_pos >= m_endPos)
    {
        write();
        initialize(this);
    }
}

///
/// CellMgr
///

CellMgr::CellMgr(int pointSize, Writer *writer) : m_pointSize(pointSize), m_writer(writer)
{}

Cell *CellMgr::get(const VoxelKey& key, const Cell *lastCell)
{
    auto it = m_cells.find(key);
    if (it == m_cells.end())
    {
        std::unique_ptr<Cell> cell(new Cell(key, m_pointSize, m_writer, this, lastCell));
        it = m_cells.insert( {key, std::move(cell)} ).first;
    }

    return it->second.get();
}

DataVecPtr CellMgr::getBuffer(const Cell *exclude)
{
    DataVecPtr b = m_writer->fetchBuffer();

    // If we couldn't fetch a buffer, flush all the the buffers for this processor and
    // try again, but block.
    if (!b)
    {
        flush(exclude);
        b = m_writer->fetchBufferBlocking();
    }
    return b;
}

// Eliminate all the cells and their associated data buffers except the `exclude`
// cell.
void CellMgr::flush(const Cell *exclude)
{
    CellMap::iterator it = m_cells.end();

    if (exclude)
        it = m_cells.find(exclude->key());

    // If there was no exclude cell just clear the cells.
    // Otherwise, save the exclude cell, clear the list, and reinsert.
    // Cells are written when they are destroyed.
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
