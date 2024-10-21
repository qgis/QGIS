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

namespace
{

// The dimension IDs need to come from the *source* layout because it's possible in the
// case of user-defined dimensions that the IDs could vary for the same-named dimension
// in different input files. The "dim" field represents the ID of the dimension
// we're reading from. "offset" is the corresponding notion in the output packed point data.
void setDimensions(pdal::PointLayoutPtr layout, FileInfo& fi)
{
    for (FileDimInfo& di : fi.dimInfo)
    {
        di.dim = layout->findDim(di.name);
        assert(di.dim != pdal::Dimension::Id::Unknown);

        // If we have a bit offset, then we're really writing to the classflags field in the
        // output point. Fetch that offset and set it. We will probably do this several
        // times (once for each bit), but the value should always be the same.
        if (di.shift != -1)
            fi.untwineBitsOffset = di.offset;
    }
}

} // unnamed namespace


FileProcessor::FileProcessor(const FileInfo& fi, size_t pointSize, const Grid& grid,
        Writer *writer, ProgressWriter& progress) :
    m_fi(fi), m_cellMgr(pointSize, writer), m_grid(grid), m_progress(progress)
{}

class BasePointProcessor
{
public:
    BasePointProcessor(const FileInfo& fi) : m_fi(fi)
    {}
    virtual ~BasePointProcessor()
    {}

    virtual void fill(const pdal::PointRef& src, Point& dst) = 0;

protected:
    const FileInfo& m_fi;
};
using PointProcessorPtr = std::unique_ptr<BasePointProcessor>;

// These processors could probably be improved performance-wise by breaking the dimensions
// up into types in the ctor to avoid the conditionals in fill().
// Could also make FileProcessor take these as a template type to avoid having fill() be
// virtual.
class StdPointProcessor : public BasePointProcessor
{
public:
    using BasePointProcessor::BasePointProcessor;

    void fill(const pdal::PointRef& src, Point& dst) override
    {
        uint8_t untwineBits = 0;
        for (const FileDimInfo& fdi : m_fi.dimInfo)
        {
            if (fdi.shift == -1)
                src.getField(reinterpret_cast<char *>(dst.data() + fdi.offset),
                    fdi.dim, fdi.type);
            else
                untwineBits |= (src.getFieldAs<uint8_t>(fdi.dim) << fdi.shift);
        }

        // We pack all the bitfields into the "untwine bits" field.
        if (m_fi.untwineBitsOffset > -1)
            memcpy(dst.data() + m_fi.untwineBitsOffset, &untwineBits, 1);
    }
};

class LegacyLasPointProcessor : public BasePointProcessor
{
public:
    using BasePointProcessor::BasePointProcessor;

    void fill(const pdal::PointRef& src, Point& dst) override
    {
        uint8_t untwineBits = 0;
        for (const FileDimInfo& fdi : m_fi.dimInfo)
        {
            if (fdi.dim == pdal::Dimension::Id::Classification)
            {
                uint8_t classification = src.getFieldAs<uint8_t>(fdi.dim);
                if (classification == 12)
                    untwineBits |= 0x08;  // Set the overlap bit.
                untwineBits |= (classification >> 5);
                classification &= 0x1F;
                memcpy(dst.data() + fdi.offset, &classification, 1);
            }
            else if (fdi.shift == -1)
                src.getField(reinterpret_cast<char *>(dst.data() + fdi.offset),
                    fdi.dim, fdi.type);
            else
                untwineBits |= (src.getFieldAs<uint8_t>(fdi.dim) << fdi.shift);
        }

        // We pack all the bitfields into the "untwine bits" field.
        if (m_fi.untwineBitsOffset > -1)
            memcpy(dst.data() + m_fi.untwineBitsOffset, &untwineBits, 1);
    }
};


void FileProcessor::run()
{
    pdal::Options opts;
    opts.add("filename", m_fi.filename);
    opts.add("count", m_fi.numPoints);
    if (m_fi.driver == "readers.las")
    {
        opts.add("nosrs", m_fi.no_srs);
        opts.add("use_eb_vlr", "true");
#ifdef PDAL_LAS_START
        opts.add("start", m_fi.start);
#endif
    }

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

    PointProcessorPtr ptProcessor;
    if (m_fi.driver == "readers.las" && m_fi.fileVersion < 14)
        ptProcessor = std::make_unique<LegacyLasPointProcessor>(m_fi);
    else
        ptProcessor = std::make_unique<StdPointProcessor>(m_fi);

    pdal::StreamCallbackFilter f;
    f.setCallback([this, &count, &cell, ptProcessor = ptProcessor.get()](pdal::PointRef& point)
        {
            // Write the data into the point buffer in the cell.  This is the *last*
            // cell buffer that we used. We're hoping that it's the right one.
            Point p = cell->point();
            ptProcessor->fill(point, p);

            // Find the actual cell that this point belongs in. If it's not the one
            // we chose, copy the data to the correct cell.
            VoxelKey cellIndex = m_grid.key(p.x(), p.y(), p.z());
            if (cellIndex != cell->key())
            {
                // Make sure that we exclude the current cell from any potential flush so
                // that no other thread can claim its data buffer and overwrite it before
                // we have a chance to copy from it in copyPoint().
                cell = m_cellMgr.get(cellIndex, cell);
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
        setDimensions(t.layout(), m_fi);
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
