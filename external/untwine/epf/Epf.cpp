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

#include <unordered_set>

#include <pdal/Dimension.hpp>
#include <pdal/Metadata.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/Bounds.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

namespace untwine
{
namespace epf
{

/// Epf

Epf::Epf(BaseInfo& common) : m_b(common), m_pool(NumFileProcessors)
{}


Epf::~Epf()
{}


void Epf::run(const Options& options, ProgressWriter& progress)
{
    using namespace pdal;

    BOX3D totalBounds;

    if (pdal::FileUtils::fileExists(options.tempDir + "/" + MetadataFilename))
        fatal("Output directory already contains EPT data.");

    m_grid.setCubic(options.doCube);

    std::vector<FileInfo> fileInfos;
    progress.m_total = createFileInfo(options.inputFiles, options.dimNames, fileInfos);

    if (options.level != -1)
        m_grid.resetLevel(options.level);

    // This is just a debug thing that will allow the number of input files to be limited.
    if (fileInfos.size() > options.fileLimit)
        fileInfos.resize(options.fileLimit);

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
        Dimension::Type type = Dimension::defaultType(Dimension::id(dimName));
        if (type == Dimension::Type::None)
            type = Dimension::Type::Double;
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
    m_writer.reset(new Writer(options.tempDir, NumWriters, layout->pointSize()));

    // Sort file infos so the largest files come first. This helps to make sure we don't delay
    // processing big files that take the longest (use threads more efficiently).
    std::sort(fileInfos.begin(), fileInfos.end(), [](const FileInfo& f1, const FileInfo& f2)
        { return f1.numPoints > f2.numPoints; });

    progress.m_threshold = progress.m_total / 40;
    progress.setIncrement(.01);
    progress.m_current = 0;

    // Add the files to the processing pool
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
    m_pool.cycle();
    progress.setPercent(.4);

    // Tell the writer that it can exit. stop() will block until the writer threads
    // are finished.
    m_writer->stop();

    // Get totals from the current writer.
    //ABELL - would be nice to avoid this copy, but it probably doesn't matter much.
    Totals totals = m_writer->totals(MaxPointsPerNode);

    // Progress for reprocessing goes from .4 to .6.
    progress.setPercent(.4);
    progress.setIncrement(.2 / totals.size());

    // Make a new writer.
    m_writer.reset(new Writer(options.tempDir, 4, layout->pointSize()));
    for (auto& t : totals)
    {
        VoxelKey key = t.first;
        int numPoints = t.second;
        int pointSize = layout->pointSize();
        std::string tempDir = options.tempDir;
        m_pool.add([&progress, key, numPoints, pointSize, tempDir, this]()
        {
            Reprocessor r(key, numPoints, pointSize, tempDir, m_grid, m_writer.get());
            r.run();
            progress.writeIncrement("Reprocessed voxel " + key.toString());
        });
    }
    m_pool.stop();
    m_writer->stop();

    fillMetadata(layout);
}

void Epf::fillMetadata(const pdal::PointLayoutPtr layout)
{
    // Info to be passed to sampler.
    m_b.bounds = m_grid.processingBounds();
    m_b.trueBounds = m_grid.conformingBounds();
    if (m_srsFileInfo.valid())
        m_b.srs = m_srsFileInfo.srs;
    m_b.pointSize = 0;
    for (pdal::Dimension::Id id : layout->dims())
    {
        FileDimInfo di;
        di.name = layout->dimName(id);
        di.type = layout->dimType(id);
        di.offset = layout->dimOffset(id);
        m_b.pointSize += pdal::Dimension::size(di.type);
        m_b.dimInfo.push_back(di);
    }
    m_b.offset[0] = m_b.bounds.maxx / 2 + m_b.bounds.minx / 2;
    m_b.offset[1] = m_b.bounds.maxy / 2 + m_b.bounds.miny / 2;
    m_b.offset[2] = m_b.bounds.maxz / 2 + m_b.bounds.minz / 2;

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

    m_b.scale[0] = calcScale(m_b.scale[0], m_b.bounds.minx, m_b.bounds.maxx);
    m_b.scale[1] = calcScale(m_b.scale[1], m_b.bounds.minx, m_b.bounds.maxx);
    m_b.scale[2] = calcScale(m_b.scale[2], m_b.bounds.minx, m_b.bounds.maxx);
}

PointCount Epf::createFileInfo(const StringList& input, StringList dimNames,
    std::vector<FileInfo>& fileInfos)
{
    using namespace pdal;

    std::vector<std::string> filenames;
    PointCount totalPoints = 0;

    // If there are some dim names specified, make sure they contain X, Y and Z and that
    // they're all uppercase.
    if (!dimNames.empty())
    {
        for (std::string& d : dimNames)
            d = Utils::toupper(d);
        for (const std::string& xyz : { "X", "Y", "Z" })
            if (!Utils::contains(dimNames, xyz))
                dimNames.push_back(xyz);
    }

    // If any of the specified input files is a directory, get the names of the files
    // in the directory and add them.
    for (const std::string& filename : input)
    {
        if (FileUtils::isDirectory(filename))
        {
            std::vector<std::string> dirfiles = FileUtils::directoryList(filename);
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
            fatal("Can't infer reader for '" + filename + "'.");
        Stage *s = factory.createStage(driver);
        pdal::Options opts;
        opts.add("filename", filename);
        s->setOptions(opts);

        QuickInfo qi = s->preview();
        if (!qi.valid())
            throw "Couldn't get quick info for '" + filename + "'.";

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
        fileInfos.push_back(fi);
        if (!m_srsFileInfo.valid() && qi.m_srs.valid())
            m_srsFileInfo = fi;

        m_grid.expand(qi.m_bounds, qi.m_pointCount);
        totalPoints += fi.numPoints;
    }
    return totalPoints;
}

} // namespace epf
} // namespace untwine
