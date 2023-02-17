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
#include "../untwine/Las.hpp"

#include <unordered_set>
#include <filesystem>

#include <pdal/pdal_features.hpp>
#include <pdal/Dimension.hpp>
#include <pdal/Metadata.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/Bounds.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>


namespace
{

// PDAL's directoryList had a bug, so we've imported a working
// version here so that we can still use older PDAL releases.

#ifndef __APPLE_CC__
std::vector<std::string> directoryList(const std::string& dir)
{
    namespace fs = std::filesystem;

    std::vector<std::string> files;

    try
    {
        fs::directory_iterator it(untwine::toNative(dir));
        fs::directory_iterator end;
        while (it != end)
        {
            files.push_back(untwine::fromNative(it->path()));
            it++;
        }
    }
    catch (fs::filesystem_error&)
    {
        files.clear();
    }
    return files;
}
#else

#include <dirent.h>

// Provide simple opendir/readdir solution for OSX because directory_iterator is
// not available until OSX 10.15
std::vector<std::string> directoryList(const std::string& dir)
{

    DIR *dpdf;
    struct dirent *epdf;

    std::vector<std::string> files;
    dpdf = opendir(dir.c_str());
    if (dpdf != NULL){
       while ((epdf = readdir(dpdf))){
           files.push_back(untwine::fromNative(epdf->d_name));
       }
    }
    closedir(dpdf);

    return files;

}
#endif

} // unnamed namespace

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

    if (pdal::FileUtils::fileExists(m_b.opts.tempDir + "/" + MetadataFilename))
        throw FatalError("Output directory already contains EPT data.");

    m_grid.setCubic(m_b.opts.doCube);

    // Create the file infos. As each info is created, the N x N x N grid is expanded to
    // hold all the points. If the number of points seems too large, N is expanded to N + 1.
    // The correct N is often wrong, especially for some areas where things are more dense.
    std::vector<FileInfo> fileInfos;
    point_count_t totalPoints = createFileInfo(m_b.opts.inputFiles, m_b.opts.dimNames, fileInfos);

    if (m_b.opts.level != -1)
        m_grid.resetLevel(m_b.opts.level);

    // This is just a debug thing that will allow the number of input files to be limited.
    if (fileInfos.size() > m_b.opts.fileLimit)
        fileInfos.resize(m_b.opts.fileLimit);

    // Stick all the dimension names from each input file in a set.
    std::unordered_set<std::string> allDimNames;
    for (const FileInfo& fi : fileInfos)
        for (const FileDimInfo& fdi : fi.dimInfo)
            allDimNames.insert(fdi.name);

    // Register the dimensions, either as the default type or double if we don't know
    // what it is.
    PointLayoutPtr layout(new PointLayout());
    for (const std::string& dimName : allDimNames)
    {
        Dimension::Type type;
        try
        {
            type = Dimension::defaultType(Dimension::id(dimName));
        }
        catch (pdal::pdal_error&)
        {
            type = Dimension::Type::Double;
        }
        layout->registerOrAssignDim(dimName, type);
    }
    layout->finalize();

    // Fill in dim info now that the layout is finalized.
    for (FileInfo& fi : fileInfos)
    {
        for (FileDimInfo& di : fi.dimInfo)
        {
            di.dim = layout->findDim(di.name);
            di.type = layout->dimType(di.dim);
            di.offset = layout->dimOffset(di.dim);
        }
    }

    // Make a writer with NumWriters threads.
    m_writer.reset(new Writer(m_b.opts.tempDir, NumWriters, layout->pointSize()));

    // Sort file infos so the largest files come first. This helps to make sure we don't delay
    // processing big files that take the longest (use threads more efficiently).
    std::sort(fileInfos.begin(), fileInfos.end(), [](const FileInfo& f1, const FileInfo& f2)
        { return f1.numPoints > f2.numPoints; });

    progress.setPointIncrementer(totalPoints, 40);

    // Add the files to the processing pool
    m_pool.trap(true, "Unknown error in FileProcessor");
    for (const FileInfo& fi : fileInfos)
    {
        int pointSize = layout->pointSize();
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
    std::vector<std::string> errors = m_pool.clearErrors();
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
    m_writer.reset(new Writer(m_b.opts.tempDir, 4, layout->pointSize()));
    m_pool.trap(true, "Unknown error in Reprocessor");
    for (auto& t : totals)
    {
        VoxelKey key = t.first;
        int numPoints = t.second;
        int pointSize = layout->pointSize();
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

    fillMetadata(layout);
}

void Epf::fillMetadata(const pdal::PointLayoutPtr layout)
{
    using namespace pdal;

    // Info to be passed to sampler.
    m_b.bounds = m_grid.processingBounds();
    m_b.trueBounds = m_grid.conformingBounds();
    if (m_srsFileInfo.valid())
        m_b.srs = m_srsFileInfo.srs;
    m_b.pointSize = 0;

    // Set the pointFormatId based on whether or not colors exist in the file
    if (layout->hasDim(Dimension::Id::Infrared))
        m_b.pointFormatId = 8;
    else if (layout->hasDim(Dimension::Id::Red) ||
             layout->hasDim(Dimension::Id::Green) ||
             layout->hasDim(Dimension::Id::Blue))
        m_b.pointFormatId = 7;
    else
        m_b.pointFormatId = 6;

    const Dimension::IdList& lasDims = pdrfDims(m_b.pointFormatId);
    for (Dimension::Id id : layout->dims())
    {
        FileDimInfo di;
        di.name = layout->dimName(id);
        di.type = layout->dimType(id);
        di.offset = layout->dimOffset(id);
        di.dim = id;
        di.extraDim = !Utils::contains(lasDims, id);
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

    // Find an offset such that (offset - min) / scale is close to an integer. This helps
    // to eliminate warning messages in lasinfo that complain because of being unable
    // to write nominal double values precisely using a 32-bit integer.
    // The hope is also that raw input values are written as the same raw values
    // on output. This may not be possible if the input files have different scaling or
    // incompatible offsets.
    auto calcOffset = [](double minval, double maxval, double scale)
    {
        double interval = maxval - minval;
        double spacings = interval / scale;  // Number of quantized values in our range.
        double halfspacings = spacings / 2;  // Half of that number.
        double offset = (int32_t)halfspacings * scale; // Round to an int value and scale down.
        return minval + offset;              // Add the base (min) value.
    };

    m_b.offset[0] = calcOffset(m_b.trueBounds.minx, m_b.trueBounds.maxx, m_b.scale[0]);
    m_b.offset[1] = calcOffset(m_b.trueBounds.miny, m_b.trueBounds.maxy, m_b.scale[1]);
    m_b.offset[2] = calcOffset(m_b.trueBounds.minz, m_b.trueBounds.maxz, m_b.scale[2]);
}

PointCount Epf::createFileInfo(const StringList& input, StringList dimNames,
    std::vector<FileInfo>& fileInfos)
{
    using namespace pdal;

    std::vector<FileInfo> tempFileInfos;
    std::vector<std::string> filenames;
    PointCount totalPoints = 0;

    // If there are some dim names specified, make sure they contain X, Y and Z and that
    // they're all uppercase.
    if (!dimNames.empty())
    {
        for (std::string& d : dimNames)
            d = Utils::toupper(d);
        for (const std::string xyz : { "X", "Y", "Z" })
            if (!Utils::contains(dimNames, xyz))
                dimNames.push_back(xyz);
    }

    // If any of the specified input files is a directory, get the names of the files
    // in the directory and add them.
    for (const std::string& filename : input)
    {
        if (FileUtils::isDirectory(filename))
        {
            std::vector<std::string> dirfiles = directoryList(filename);
            filenames.insert(filenames.end(), dirfiles.begin(), dirfiles.end());
        }
        else
            filenames.push_back(filename);
    }

    std::vector<double> xOffsets;
    std::vector<double> yOffsets;
    std::vector<double> zOffsets;

    // Determine a driver for each file and get a preview of the file.  If we couldn't
    // Create a FileInfo object containing the file bounds, dimensions, filename and
    // associated driver.  Expand our grid by the bounds and file point count.
    for (std::string& filename : filenames)
    {
        StageFactory factory;
        std::string driver = factory.inferReaderDriver(filename);
        if (driver.empty())
            throw FatalError("Can't infer reader for '" + filename + "'.");
        Stage *s = factory.createStage(driver);
        pdal::Options opts;
        opts.add("filename", filename);
        s->setOptions(opts);

        QuickInfo qi = s->preview();

        if (!qi.valid())
            throw FatalError("Couldn't get quick info for '" + filename + "'.");

        // Get scale values from the reader if they exist.
        pdal::MetadataNode root = s->getMetadata();
        pdal::MetadataNode m = root.findChild("scale_x");
        if (m.valid())
            m_b.scale[0] = (std::max)(m_b.scale[0], m.value<double>());
        m = root.findChild("scale_y");
        if (m.valid())
            m_b.scale[1] = (std::max)(m_b.scale[1], m.value<double>());
        m = root.findChild("scale_z");
        if (m.valid())
            m_b.scale[2] = (std::max)(m_b.scale[2], m.value<double>());
        m = root.findChild("offset_x");
        if (m.valid())
            xOffsets.push_back(m.value<double>());
        m = root.findChild("offset_y");
        if (m.valid())
            yOffsets.push_back(m.value<double>());
        m = root.findChild("offset_z");
        if (m.valid())
            zOffsets.push_back(m.value<double>());

        FileInfo fi;
        fi.bounds = qi.m_bounds;
        fi.numPoints = qi.m_pointCount;
        fi.filename = filename;
        fi.driver = driver;

        // Accept dimension names if there are no limits or this name is in the list
        // of desired dimensions.
        for (const std::string& name : qi.m_dimNames)
            if (dimNames.empty() || Utils::contains(dimNames, Utils::toupper(name)))
                fi.dimInfo.push_back(FileDimInfo(name));

        if (m_srsFileInfo.valid() && m_srsFileInfo.srs != qi.m_srs)
            std::cerr << "Files have mismatched SRS values. Using SRS from '" <<
                m_srsFileInfo.filename << "'.\n";
        fi.srs = qi.m_srs;
        tempFileInfos.push_back(fi);
        if (!m_srsFileInfo.valid() && qi.m_srs.valid())
            m_srsFileInfo = fi;

        m_grid.expand(qi.m_bounds, qi.m_pointCount);
        totalPoints += fi.numPoints;
    }

    // If we had an offset from the input, choose one in the middle of the list of offsets.
    if (xOffsets.size())
    {
        std::sort(xOffsets.begin(), xOffsets.end());
        m_b.offset[0] = xOffsets[xOffsets.size() / 2];
    }
    if (yOffsets.size())
    {
        std::sort(yOffsets.begin(), yOffsets.end());
        m_b.offset[1] = yOffsets[yOffsets.size() / 2];
    }
    if (zOffsets.size())
    {
        std::sort(zOffsets.begin(), zOffsets.end());
        m_b.offset[2] = zOffsets[zOffsets.size() / 2];
    }

    // If we have LAS start capability, break apart file infos into chunks of size 5 million.
#ifdef PDAL_LAS_START
    PointCount ChunkSize = 5'000'000;
    for (const FileInfo& fi : tempFileInfos)
    {
        if (fi.driver != "readers.las" || fi.numPoints < ChunkSize)
        {
            fileInfos.push_back(fi);
            continue;
        }
        PointCount remaining = fi.numPoints;
        pdal::PointId start = 0;
        while (remaining)
        {
            FileInfo lasFi(fi);
            lasFi.numPoints = (std::min)(ChunkSize, remaining);
            lasFi.start = start;
            fileInfos.push_back(lasFi);

            start += ChunkSize;
            remaining -= lasFi.numPoints;
        }
    }
#else
    fileInfos = std::move(tempFileInfos);
#endif

    return totalPoints;
}

} // namespace epf
} // namespace untwine
