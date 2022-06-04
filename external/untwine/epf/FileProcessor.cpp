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


#include "FileProcessor.hpp"
#include "../untwine/ProgressWriter.hpp"

#include <pdal/pdal_features.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/filters/StreamCallbackFilter.hpp>

namespace untwine
{
namespace epf
{

FileProcessor::FileProcessor(const FileInfo& fi, size_t pointSize, const Grid& grid,
        Writer *writer, ProgressWriter& progress) :
    m_fi(fi), m_cellMgr(pointSize, writer), m_grid(grid), m_progress(progress)
{}

void FileProcessor::run()
{

    pdal::Options opts;
    opts.add("filename", m_fi.filename);
    opts.add("count", m_fi.numPoints);
#ifdef PDAL_LAS_START
    if (m_fi.driver == "readers.las")
        opts.add("start", m_fi.start);
#endif

    pdal::StageFactory factory;
    pdal::Stage *s = factory.createStage(m_fi.driver);
    s->setOptions(opts);


    PointCount count = 0;

    // We need to move the data from the PointRef to some output buffer. We copy the data
    // to the end of the *last* output buffer we used in hopes that it's the right one.
    // If it's not we lose and we're forced to move that data to the another cell,
    // which then becomes the active cell.

    // This is some random cell that ultimately won't get used, but it contains a buffer
    // into which we can write data.
    Cell *cell = m_cellMgr.get(VoxelKey());

    pdal::StreamCallbackFilter f;
    f.setCallback([this, &count, &cell](pdal::PointRef& point)
        {
            // Write the data into the point buffer in the cell.  This is the *last*
            // cell buffer that we used. We're hoping that it's the right one.
            Point p = cell->point();
            for (const FileDimInfo& fdi : m_fi.dimInfo)
                point.getField(reinterpret_cast<char *>(p.data() + fdi.offset),
                    fdi.dim, fdi.type);

            // Find the actual cell that this point belongs in. If it's not the one
            // we chose, copy the data to the correct cell.
            VoxelKey cellIndex = m_grid.key(p.x(), p.y(), p.z());
            if (cellIndex != cell->key())
            {
                cell = m_cellMgr.get(cellIndex);
                cell->copyPoint(p);
            }
            // Advance the cell - move the buffer pointer so when we refer to the cell's
            // point, we're referring to the next location in the cell's buffer.
            cell->advance();
            count++;

            if (count == ProgressWriter::ChunkSize)
            {
                m_progress.update();
                count = 0;
            }

            return true;
        }
    );
    f.setInput(*s);

    pdal::FixedPointTable t(1000);

    try
    {
        f.prepare(t);
        f.execute(t);
    }
    catch (const pdal::pdal_error& err)
    {
        throw FatalError(err.what());
    }

    // We normally call update for every CountIncrement points, but at the end, just
    // tell the progress writer the number that we've done since the last update.
    m_progress.update(count);
}

} // namespace epf
} // namespace untwine
