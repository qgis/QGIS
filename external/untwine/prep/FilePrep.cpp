/*****************************************************************************
 *   Copyright (c) 2024, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include "FilePrep.hpp"

#include "untwine/Common.hpp"
#include "untwine/FileInfo.hpp"

#include <pdal/PointLayout.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/io/LasHeader.hpp>
#include <pdal/io/LasReader.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/Bounds.hpp>

#include <dirlist.hpp>  // untwine/os

namespace untwine
{
namespace prep
{

FilePrep::FilePrep(BaseInfo& common) : m_b(common)
{}


FilePrep::~FilePrep()
{}

std::vector<FileInfo> FilePrep::run()
{
    using namespace pdal;

    BOX3D totalBounds;

    // Create the file infos.
    std::vector<FileInfo> fileInfos;
    createFileInfos(m_b.opts.inputFiles, fileInfos);

    m_b.numPoints = 0;
    for (const FileInfo& info : fileInfos)
    {
        m_trueBounds.grow(info.bounds);
        m_b.numPoints += info.numPoints;
    }

    pdal::PointLayout outputLayout;

    filterDims(fileInfos, m_b.opts.dimNames);
    determineDims(fileInfos, outputLayout);
    determineScale(fileInfos);
    determineBounds();
    determineOffset(fileInfos);
    determineSrs(fileInfos);

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

    fillMetadata(outputLayout);
    return fileInfos;
}

void FilePrep::determineBounds()
{
    m_b.bounds = m_trueBounds;
    if (m_b.opts.doCube)
    {
        double side =
            (std::max)(
            (std::max)(m_b.bounds.maxx - m_b.bounds.minx, m_b.bounds.maxy - m_b.bounds.miny),
                       m_b.bounds.maxz - m_b.bounds.minz);
        m_b.bounds.maxx = m_b.bounds.minx + side;
        m_b.bounds.maxy = m_b.bounds.miny + side;
        m_b.bounds.maxz = m_b.bounds.minz + side;
    }

    // Expand bounds to make sure that the octree will contain all points once output LAS scaling
    // is applied.
    m_b.bounds.minx -= m_b.xform.scale.x;
    m_b.bounds.miny -= m_b.xform.scale.y;
    m_b.bounds.minz -= m_b.xform.scale.z;
    m_b.bounds.maxx += m_b.xform.scale.x;
    m_b.bounds.maxy += m_b.xform.scale.y;
    m_b.bounds.maxz += m_b.xform.scale.z;
}

void FilePrep::fillMetadata(const pdal::PointLayout& layout)
{
    using namespace pdal;

    // Set the pointFormatId based on whether or not colors exist in the file
    if (layout.hasDim(Dimension::Id::Infrared))
        m_b.pointFormatId = 8;
    else if (layout.hasDim(Dimension::Id::Red) ||
             layout.hasDim(Dimension::Id::Green) ||
             layout.hasDim(Dimension::Id::Blue))
        m_b.pointFormatId = 7;
    else
        m_b.pointFormatId = 6;

    m_b.pointSize = 0;
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
}

void FilePrep::createFileInfos(const StringList& input, std::vector<FileInfo>& fileInfos)
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

void FilePrep::filterDims(std::vector<FileInfo>& infos, StringList allowedDims)
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

void FilePrep::determineDims(std::vector<FileInfo>& infos, pdal::PointLayout& layout)
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

void FilePrep::determineOffset(const std::vector<FileInfo>& infos)
{
    if (!determineOffsetFromInfos(infos))
        determineOffsetFromBounds();
}

void FilePrep::determineOffsetFromBounds()
{
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

    m_b.xform.offset.x = calcOffset(m_trueBounds.minx, m_trueBounds.maxx, m_b.xform.scale.x);
    m_b.xform.offset.y = calcOffset(m_trueBounds.miny, m_trueBounds.maxy, m_b.xform.scale.y);
    m_b.xform.offset.z = calcOffset(m_trueBounds.minz, m_trueBounds.maxz, m_b.xform.scale.z);
}

bool FilePrep::determineOffsetFromInfos(const std::vector<FileInfo>& infos)
{
    std::vector<double> xOffsets;
    std::vector<double> yOffsets;
    std::vector<double> zOffsets;

    for (const auto& fi : infos)
    {
        if (std::isnan(fi.xform.offset.x) || std::isnan(fi.xform.offset.y) ||
            std::isnan(fi.xform.offset.z))
            return false;
        xOffsets.push_back(fi.xform.offset.x);
        yOffsets.push_back(fi.xform.offset.y);
        zOffsets.push_back(fi.xform.offset.z);
    }

    std::sort(xOffsets.begin(), xOffsets.end());
    std::sort(yOffsets.begin(), yOffsets.end());
    std::sort(zOffsets.begin(), zOffsets.end());

    // Size should never be zero, but the checks here avoids any divide-by-zero possibility.
    if (xOffsets.size())
        m_b.xform.offset.x = xOffsets[xOffsets.size() / 2];
    if (yOffsets.size())
        m_b.xform.offset.y = yOffsets[yOffsets.size() / 2];
    if (zOffsets.size())
        m_b.xform.offset.z = zOffsets[zOffsets.size() / 2];
    return true;
}

void FilePrep::determineScale(const std::vector<FileInfo>& infos)
{
    for (const auto& fi : infos)
    {
        if (std::abs(fi.xform.scale.x) > std::abs(m_b.xform.scale.x))
            m_b.xform.scale.x = fi.xform.scale.x;
        if (std::abs(fi.xform.scale.y) > std::abs(m_b.xform.scale.y))
            m_b.xform.scale.y = fi.xform.scale.y;
        if (std::abs(fi.xform.scale.z) > std::abs(m_b.xform.scale.z))
            m_b.xform.scale.z = fi.xform.scale.z;
    }

    if (m_b.xform.scale.x != 0.0 && m_b.xform.scale.y != 0.0 && m_b.xform.scale.z != 0.0)
        return;

    auto calcScale = [](double low, double high)
    {
        // 2 billion is a little less than the int limit.  We center the data around 0 with the
        // offset, so we're applying the scale to half the range of the data.
        double val = high / 2 - low / 2;
        double power = std::ceil(std::log10(val / 2'000'000'000.0));

        // Set an arbitrary limit on scale of 1e10-4.
        return std::pow(10, (std::max)(power, -4.0));
    };

    m_b.xform.scale.x = calcScale(m_trueBounds.minx, m_trueBounds.maxx);
    m_b.xform.scale.y = calcScale(m_trueBounds.miny, m_trueBounds.maxy);
    m_b.xform.scale.z = calcScale(m_trueBounds.minz, m_trueBounds.maxz);
}

void FilePrep::determineSrs(const std::vector<FileInfo>& infos)
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
    m_b.srs = srsFileInfo.srs;
}

std::vector<FileInfo> FilePrep::processLas(pdal::LasReader& r, FileInfo fi)
{
    using namespace pdal;

    PointTable t;
    r.prepare(t);

    const LasHeader& h = r.header();

    fi.bounds = h.getBounds();
    fi.numPoints = h.pointCount();

    fi.xform.scale.x = h.scaleX();
    fi.xform.scale.y = h.scaleY();
    fi.xform.scale.z = h.scaleZ();

    fi.xform.offset.x = h.offsetX();
    fi.xform.offset.y = h.offsetY();
    fi.xform.offset.z = h.offsetZ();

    fi.srs = h.srs();

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

FileInfo FilePrep::processGeneric(pdal::Stage& s, FileInfo fi)
{
    pdal::QuickInfo qi = s.preview();

    if (!qi.valid())
        throw FatalError("Couldn't get quick info for '" + fi.filename + "'.");

    fi.bounds = qi.m_bounds;
    fi.numPoints = qi.m_pointCount;
    fi.srs = qi.m_srs;

    for (const std::string& name : qi.m_dimNames)
        fi.dimInfo.push_back(FileDimInfo(name));

    calcCreationDay();
    return fi;
}

void FilePrep::calcCreationDay()
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

} // namespace prep
} // namespace untwine
