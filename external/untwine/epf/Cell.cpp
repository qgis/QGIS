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
    m_buf = m_writer->bufferCache().fetch();
    m_pos = m_buf->data();

    m_endPos = m_pos + m_pointSize * (BufSize / m_pointSize);
}

void Cell::write()
{
    // Resize the buffer so the writer knows how much to write.
    size_t size = m_pos - m_buf->data();
    if (size)
//    {
//        m_buf->resize(size);
        m_writer->enqueue(m_key, std::move(m_buf), size);
//    }
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
        std::unique_ptr<Cell> cell(new Cell(key, m_pointSize, m_writer));
        it = m_cells.insert( {key, std::move(cell)} ).first;
    }
    Cell& c = *(it->second);
    return &c;
}

void CellMgr::flush()
{
    for (auto& cp : m_cells)
        cp.second->write();
}

} // namespace epf
} // namespace untwine
