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

#include <cmath>
#include <filesystem>
#include <unordered_set>

#include <pdal/pdal_features.hpp>
#include <pdal/Dimension.hpp>
#include <pdal/Metadata.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/io/LasHeader.hpp>
#include <pdal/io/LasReader.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/Bounds.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include <dirlist.hpp>  // untwine/os

namespace untwine
{
namespace epf
{

/// Epf

static_assert(MaxBuffers > NumFileProcessors, "MaxBuffers must be greater than NumFileProcessors.");
Epf::Epf(BaseInfo& common) : m_b(common), m_pool(NumFileProcessors)
{}


Epf::~Epf()
{}

void Epf::run(ProgressWriter& progress)
{
    using namespace pdal;

    BOX3D totalBounds;

    m_grid.setCubic(m_b.opts.doCube);

    // Create the file infos.
    std::vector<FileInfo> fileInfos;
    createFileInfos(m_b.opts.inputFiles, fileInfos);

    pdal::PointLayout outputLayout;

    filterDims(fileInfos, m_b.opts.dimNames);
    determineDims(fileInfos, outputLayout);
    determineOffset(fileInfos);
    m_b.srs = determineSrs(fileInfos);

    // Setup grid and point count. For each file info the N x N x N grid is expanded to
    // hold all the points. If the number of points seems too large, N is expanded to N + 1.
    // The correct N is often wrong, especially for some areas where things are more dense.
    PointCount totalPoints = 0;
    for (const FileInfo& info : fileInfos)
    {
        m_grid.expand(info.bounds, info.numPoints);
        totalPoints += info.numPoints;
    }
    if (m_b.opts.level != -1)
        m_grid.resetLevel(m_b.opts.level);

    // This is just a debug thing that will allow the number of input files to be limited.
    if (fileInfos.size() > m_b.opts.fileLimit)
        fileInfos.resize(m_b.opts.fileLimit);

    // Fill in dim info.
    for (FileInfo& fi : fileInfos)
    {
        for (FileDimInfo& di : fi.dimInfo)
        {
            // If this dimension is one of the bit dimensions, set the shift and offset accordingly.
            int bitPos = getUntwineBitPos(di.name);
            if (bitPos != -1)
            {
                di.type = UntwineBitsType;
                di.offset = outputLayout.dimOffset(outputLayout.findDim(UntwineBitsDimName));
                di.shift = bitPos;
            }
            else
            {
                Dimension::Id dim = outputLayout.findDim(di.name);
                di.type = outputLayout.dimType(dim);
                di.offset = outputLayout.dimOffset(dim);
            }
        }
    }

    // Make a writer with NumWriters threads.
    m_writer.reset(new Writer(m_b.opts.tempDir, NumWriters, outputLayout.pointSize()));

    // Sort file infos so the largest files come first. This helps to make sure we don't delay
    // processing big files that take the longest (use threads more efficiently).
    std::sort(fileInfos.begin(), fileInfos.end(), [](const FileInfo& f1, const FileInfo& f2)
        { return f1.numPoints > f2.numPoints; });

    progress.setPointIncrementer(totalPoints, 40);

    // Add the files to the processing pool
    m_pool.trap(true, "Unknown error in FileProcessor");
    for (const FileInfo& fi : fileInfos)
    {
        int pointSize = outputLayout.pointSize();
        m_pool.add([&fi, &progress, pointSize, this]()
        {
            FileProcessor fp(fi, pointSize, m_grid, m_writer.get(), progress);
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
    m_writer.reset(new Writer(m_b.opts.tempDir, 4, outputLayout.pointSize()));
    m_pool.trap(true, "Unknown error in Reprocessor");
    for (auto& t : totals)
    {
        VoxelKey key = t.first;
        int numPoints = t.second;
        int pointSize = outputLayout.pointSize();
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

    fillMetadata(outputLayout);
}

void Epf::fillMetadata(const pdal::PointLayout& layout)
{
    using namespace pdal;

    // Info to be passed to sampler.
    m_b.bounds = m_grid.processingBounds();
    m_b.trueBounds = m_grid.conformingBounds();

    m_b.pointSize = 0;

    // Set the pointFormatId based on whether or not colors exist in the file
    if (layout.hasDim(Dimension::Id::Infrared))
        m_b.pointFormatId = 8;
    else if (layout.hasDim(Dimension::Id::Red) ||
             layout.hasDim(Dimension::Id::Green) ||
             layout.hasDim(Dimension::Id::Blue))
        m_b.pointFormatId = 7;
    else
        m_b.pointFormatId = 6;

    for (Dimension::Id id : layout.dims())
    {
        FileDimInfo di;
        di.name = layout.dimName(id);
        di.type = layout.dimType(id);
        di.offset = layout.dimOffset(id);
        di.extraDim = isExtraDim(di.name);
        m_b.pointSize += pdal::Dimension::size(di.type);
        m_b.dimInfo.push_back(di);
    }

    auto calcScale = [](double scale, double low, double high)
    {
        if (scale > 0)
            return scale;

        // 2 billion is a little less than the int limit.  We center the data around 0 with the
        // offset, so we're applying the scale to half the range of the data.
        double val = high / 2 - low / 2;
        double power = std::ceil(std::log10(val / 2000000000.0));

        // Set an arbitrary limit on scale of 1e10-4.
        return std::pow(10, (std::max)(power, -4.0));
    };

    m_b.scale[0] = calcScale(m_b.scale[0], m_b.trueBounds.minx, m_b.trueBounds.maxx);
    m_b.scale[1] = calcScale(m_b.scale[1], m_b.trueBounds.miny, m_b.trueBounds.maxy);
    m_b.scale[2] = calcScale(m_b.scale[2], m_b.trueBounds.minz, m_b.trueBounds.maxz);

    // The hope is that raw input values are written as the same raw values
    // on output. This may not be possible if the input files have different
    // scaling or incompatible offsets.
    auto calcOffset = [](double minval, double maxval, double scale)
    {
        double interval = maxval - minval;
        double spacings = interval / scale;  // Number of quantized values in our range.
        double halfspacings = spacings / 2;  // Half of that number.
        double offset = (int32_t)halfspacings * scale; // Round to an int value and scale down.
        return std::round(minval + offset);  // Add the base (min) value and round to an integer.
    };

    // Preserve offsets if we have them and --single_file with single input is used
    if (!m_b.preserveHeaderFields() ||
        std::isnan(m_b.offset[0]) || std::isnan(m_b.offset[1]) || std::isnan(m_b.offset[2]))
    {
        m_b.offset[0] = calcOffset(m_b.trueBounds.minx, m_b.trueBounds.maxx, m_b.scale[0]);
        m_b.offset[1] = calcOffset(m_b.trueBounds.miny, m_b.trueBounds.maxy, m_b.scale[1]);
        m_b.offset[2] = calcOffset(m_b.trueBounds.minz, m_b.trueBounds.maxz, m_b.scale[2]);
    }
}

void Epf::createFileInfos(const StringList& input, std::vector<FileInfo>& fileInfos)
{
    using namespace pdal;

    StringList filenames;

    // If any of the specified input files is a directory, get the names of the files
    // in the directory and add them.
    for (const std::string& filename : input)
    {
        if (FileUtils::isDirectory(filename))
        {
            StringList dirfiles = os::directoryList(filename);
            filenames.insert(filenames.end(), dirfiles.begin(), dirfiles.end());
        }
        else
            filenames.push_back(filename);
    }

    // Determine a driver for each file and get a preview of the file.  If we couldn't
    // Create a FileInfo object containing the file bounds, dimensions, filename and
    // associated driver.  Expand our grid by the bounds and file point count.
    for (std::string& filename : filenames)
    {
        StageFactory factory;
        std::string driver = factory.inferReaderDriver(filename);
        if (driver.empty())
            throw FatalError("Can't infer reader for '" + filename + "'.");
        // Use LAS reader for COPC files.
        if (driver == "readers.copc")
            driver = "readers.las";
        Stage *s = factory.createStage(driver);

        pdal::Options opts;
        opts.add("filename", filename);
        if (driver == "readers.las")
        {
            opts.add("nosrs", m_b.opts.no_srs);
            opts.add("use_eb_vlr", "true");
        }
        s->setOptions(opts);

        FileInfo fi;
        fi.filename = filename;
        fi.driver = driver;
        fi.no_srs = m_b.opts.no_srs;
        if (driver == "readers.las")
        {
            const std::vector<FileInfo>& infos = processLas(*dynamic_cast<LasReader *>(s), fi);
            fileInfos.insert(fileInfos.begin(), infos.begin(), infos.end());
        }
        else
            fileInfos.push_back(processGeneric(*s, fi));
    }
}

void Epf::filterDims(std::vector<FileInfo>& infos, StringList allowedDims)
{
    if (allowedDims.empty())
        return;

    // If there are some dim names specified, make sure they contain X, Y and Z and that
    // they're all uppercase.
    for (std::string& d : allowedDims)
        d = pdal::Utils::toupper(d);
    for (const std::string xyz : { "X", "Y", "Z" })
        if (!pdal::Utils::contains(allowedDims, xyz))
            allowedDims.push_back(xyz);

    // Remove dimensions not in the allowed list.
    for (FileInfo& info : infos)
        for (auto it = info.dimInfo.begin(); it != info.dimInfo.end(); ++it)
        {
            FileDimInfo& fdi = *it;
            if (!pdal::Utils::contains(allowedDims, fdi.name))
                it = info.dimInfo.erase(it);
        }
}

void Epf::determineDims(std::vector<FileInfo>& infos, pdal::PointLayout& layout)
{
    using namespace pdal;

    StringList untypedDimNames;

    // Register dimensions in the OUTPUT layout, transforming dimensions that are packed
    // into the "untwine bits" dimension.
    for (FileInfo& info : infos)
        for (FileDimInfo dim : info.dimInfo)
        {
            std::string name = dim.name;
            Dimension::Type type = dim.type;

            if (type == Dimension::Type::None)
                untypedDimNames.push_back(name);
            else
            {
                if (isUntwineBitsDim(name))
                {
                    name = UntwineBitsDimName;
                    type = UntwineBitsType;
                }
                layout.registerOrAssignDim(name, type);
            }
        }

    // Register dimensions that didn't come from LAS files if their names don't match
    // those that are already in the output layout.
    for (std::string name : untypedDimNames)
    {
        if (layout.findDim(name) == Dimension::Id::Unknown)
        {
            Dimension::Type type = Dimension::Type::Double;
            if (isUntwineBitsDim(name))
            {
                name = UntwineBitsDimName;
                type = UntwineBitsType;
            }
            else
            {
                try
                {
                    type = Dimension::defaultType(Dimension::id(name));
                }
                catch (pdal_error&)
                {}
            }
            layout.registerOrAssignDim(name, type);
        }
    }

    layout.finalize();
}

// Determine a reasonable offset for the output data.
void Epf::determineOffset(const std::vector<FileInfo>& infos)
{
    std::vector<double> xOffsets;
    std::vector<double> yOffsets;
    std::vector<double> zOffsets;

    for (const auto& fi : infos)
    {
        xOffsets.push_back(fi.offsets[0]);
        yOffsets.push_back(fi.offsets[1]);
        zOffsets.push_back(fi.offsets[2]);
    }
    std::sort(xOffsets.begin(), xOffsets.end());
    std::sort(yOffsets.begin(), yOffsets.end());
    std::sort(zOffsets.begin(), zOffsets.end());
    if (xOffsets.size())
        m_b.offset[0] = xOffsets[xOffsets.size() / 2];
    if (yOffsets.size())
        m_b.offset[1] = yOffsets[yOffsets.size() / 2];
    if (zOffsets.size())
        m_b.offset[2] = zOffsets[zOffsets.size() / 2];
}

pdal::SpatialReference Epf::determineSrs(const std::vector<FileInfo>& infos)
{
    // Here FileInfo is just convenient as it has both SRS and filename.
    FileInfo srsFileInfo;

    for (const auto& fi : infos)
    {
        if (fi.srs.valid())
        {
            if (!srsFileInfo.srs.valid())
                srsFileInfo = fi;
            else if (srsFileInfo.srs != fi.srs)
                std::cerr << "Files have mismatched SRS values. Using SRS from '" <<
                    srsFileInfo.filename << "'.\n";
        }
    }
    return srsFileInfo.srs;
}

std::vector<FileInfo> Epf::processLas(pdal::LasReader& r, FileInfo fi)
{
    using namespace pdal;

    PointTable t;
    r.prepare(t);

    const LasHeader& h = r.header();

    fi.bounds = h.getBounds();
    fi.numPoints = h.pointCount();

    m_b.scale[0] = (std::max)(m_b.scale[0], h.scaleX());
    m_b.scale[1] = (std::max)(m_b.scale[1], h.scaleY());
    m_b.scale[2] = (std::max)(m_b.scale[2], h.scaleZ());

    fi.offsets[0] = h.offsetX();
    fi.offsets[1] = h.offsetY();
    fi.offsets[2] = h.offsetZ();

    if (m_b.preserveHeaderFields())
    {
        m_b.globalEncoding = h.globalEncoding();
        m_b.creationDoy = h.creationDOY();
        m_b.creationYear = h.creationYear();
        m_b.generatingSoftware = h.softwareId();
        m_b.systemId = h.systemId();
        m_b.fileSourceId = h.fileSourceId();
    }
    else
        calcCreationDay();

    // Detect LAS input with LAS version < 4 so that we can handle the legacy
    // classification bits.
    fi.fileVersion = 10 * h.versionMajor() + h.versionMinor();

    PointLayoutPtr layout = t.layout();
    for (pdal::Dimension::Id id : layout->dims())
    {
        const std::string& name = layout->dimName(id);
        Dimension::Type type = layout->dimType(id);
        fi.dimInfo.push_back(FileDimInfo(name, type));
    }

    std::vector<FileInfo> fileInfos;
    // If we have LAS start capability, break apart file info into chunks of size 5 million.
#ifdef PDAL_LAS_START
    PointCount ChunkSize = 5'000'000;

    PointCount remaining = fi.numPoints;
    PointId start = 0;

    while (remaining)
    {
        FileInfo lasFi(fi);
        lasFi.numPoints = (std::min)(ChunkSize, remaining);
        lasFi.start = start;
        fileInfos.push_back(lasFi);

        start += ChunkSize;
        remaining -= lasFi.numPoints;
    }
#else
    fileInfos.push_back(fi);
#endif
    return fileInfos;
}

FileInfo Epf::processGeneric(pdal::Stage& s, FileInfo fi)
{
    pdal::QuickInfo qi = s.preview();

    if (!qi.valid())
        throw FatalError("Couldn't get quick info for '" + fi.filename + "'.");

    fi.bounds = qi.m_bounds;
    fi.numPoints = qi.m_pointCount;

    for (const std::string& name : qi.m_dimNames)
        fi.dimInfo.push_back(FileDimInfo(name));

    calcCreationDay();
    return fi;
}

void Epf::calcCreationDay()
{
    std::time_t now;
    std::time(&now);
    std::tm* ptm = std::gmtime(&now);
    if (ptm)
    {
        m_b.creationDoy = ptm->tm_yday + 1;
        m_b.creationYear = ptm->tm_year + 1900;
    }
}

} // namespace epf
} // namespace untwine
