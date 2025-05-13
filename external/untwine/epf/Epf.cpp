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

#include "Epf.hpp"
#include "EpfTypes.hpp"
#include "FileProcessor.hpp"
#include "Reprocessor.hpp"
#include "Writer.hpp"
#include "../untwine/Common.hpp"

namespace untwine
{
namespace epf
{

static_assert(MaxBuffers > NumFileProcessors, "MaxBuffers must be greater than NumFileProcessors.");
Epf::Epf(BaseInfo& common) :
    m_b(common), m_grid(m_b.bounds, m_b.numPoints, m_b.opts.level, m_b.opts.doCube),
    m_pool(NumFileProcessors)
{}

Epf::~Epf()
{}

void Epf::run(ProgressWriter& progress, std::vector<FileInfo>& fileInfos)
{
    using namespace pdal;

    BOX3D totalBounds;

    // Sort file infos so the largest files come first. This helps to make sure we don't delay
    // processing big files that take the longest (use threads more efficiently).
    std::sort(fileInfos.begin(), fileInfos.end(), [](const FileInfo& f1, const FileInfo& f2)
        { return f1.numPoints > f2.numPoints; });

    // Make a writer with NumWriters threads.
    m_writer.reset(new Writer(m_b.opts.tempDir, NumWriters, m_b.pointSize));

    progress.setPointIncrementer(m_b.numPoints, 40);

    // Add the files to the processing pool
    m_pool.trap(true, "Unknown error in FileProcessor");
    for (const FileInfo& fi : fileInfos)
    {
        m_pool.add([&fi, &progress, pointSize = m_b.pointSize, xform = m_b.xform, this]()
        {
            FileProcessor fp(fi, pointSize, m_grid, xform, m_writer.get(), progress);
            fp.run();
        });
    }

    // Wait for  all the processors to finish and restart.
    m_pool.join();
    // Tell the writer that it can exit. stop() will block until the writer threads
    // are finished.  stop() will throw if an error occurred during writing.
    m_writer->stop();

    // If the FileProcessors had an error, throw.
    StringList errors = m_pool.clearErrors();
    if (errors.size())
        throw FatalError(errors.front());

    m_pool.go();
    progress.setPercent(.4);

    // Get totals from the current writer that are greater than the MaxPointsPerNode.
    // Each of these voxels that is too large will be reprocessed.
    //ABELL - would be nice to avoid this copy, but it probably doesn't matter much.
    Totals totals = m_writer->totals(MaxPointsPerNode);

    // Progress for reprocessing goes from .4 to .6.
    progress.setPercent(.4);
    progress.setIncrement(.2 / (std::max)((size_t)1, totals.size()));

    // Make a new writer since we stopped the old one. Could restart, but why bother with
    // extra code...
    m_writer.reset(new Writer(m_b.opts.tempDir, 4, m_b.pointSize));
    m_pool.trap(true, "Unknown error in Reprocessor");
    for (auto& t : totals)
    {
        VoxelKey key = t.first;
        int numPoints = t.second;
        int pointSize = m_b.pointSize;
        std::string tempDir = m_b.opts.tempDir;

        // Create a reprocessor thread. Note that the grid is copied by value and
        // its level is re-calculated based on the number of points.
        m_pool.add([&progress, key, numPoints, pointSize, tempDir, this]()
        {
            Reprocessor r(key, numPoints, pointSize, tempDir, m_grid, m_writer.get());
            r.run();
            progress.writeIncrement("Reprocessed voxel " + key.toString());
        });
    }
    m_pool.stop();
    // If the Reprocessors had an error, throw.
    errors = m_pool.clearErrors();
    if (errors.size())
        throw FatalError(errors.front());

    m_writer->stop();
}

} // namespace epf
} // namespace untwine
