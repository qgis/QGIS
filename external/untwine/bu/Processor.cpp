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

#include <numeric>
#include <random>

#include "../untwine/GridKey.hpp"
#include "../untwine/Las.hpp"

#include <pdal/PDALUtils.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/io/BufferReader.hpp>
#include <pdal/filters/SortFilter.hpp>
#include <pdal/util/Algorithm.hpp>

#include <lazperf/lazperf.hpp>
#include <lazperf/writers.hpp>
#include <lazperf/readers.hpp>

#include "Processor.hpp"
#include "PyramidManager.hpp"

namespace untwine
{
namespace bu
{

static const int MinimumPoints = 100;
static const int MinimumTotalPoints = 1500;

Processor::Processor(PyramidManager& manager, const VoxelInfo& v, const BaseInfo& b) :
    m_vi(v), m_b(b), m_manager(manager), m_points(m_b)
{}


void Processor::run()
{
    // Don't let any exception sneak out of here.
    try
    {
        runLocal();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
        m_manager.queueWithError(m_vi.octant(), ex.what());
        return;
    }
    catch (...)
    {
        std::string msg = std::string("Unexpected error processing ") + m_vi.key().toString() + ".";
        std::cerr << "Exception: " << msg << "\n";
        m_manager.queueWithError(m_vi.octant(), msg);
        return;
    }
    m_manager.queue(m_vi.octant());
}

void Processor::runLocal()
{
    // If we don't merge small files into one, we'll end up trying to deal with too many
    // open files later and run out of file descriptors.
    for (int i = 0; i < 8; ++i)
    {
        OctantInfo& child = m_vi[i];
        if (child.fileInfos().size() >= 4)
            child.mergeSmallFiles(m_b.opts.tempDir, m_b.pointSize);
    }

    size_t totalPoints = 0;
    size_t totalFileInfos = 0;
    for (int i = 0; i < 8; ++i)
    {
        OctantInfo& child = m_vi[i];
        totalFileInfos += child.fileInfos().size();
        totalPoints += child.numPoints();
        if (child.numPoints() < MinimumPoints)
            m_vi.octant().appendFileInfos(child);
    }

    // It's possible that all the file infos have been moved above, but this is cheap.
    if (totalPoints < MinimumTotalPoints)
        for (int i = 0; i < 8; ++i)
            m_vi.octant().appendFileInfos(m_vi[i]);

    // Accepted points are those that will go in this (the parent) cell.
    // Rejected points will remain in the child cell they were in previously.
    Index accepted;
    Index rejected;

    // If the file infos haven't all been hoisted, sample.
    if (m_vi.octant().fileInfos().size() != totalFileInfos)
        sample(accepted, rejected);

    write(accepted, rejected);
}


void Processor::sample(Index& accepted, Index& rejected)
{
    int totalPoints = 0;
    for (int i = 0; i < 8; ++i)
    {
        OctantInfo& child = m_vi[i];
        for (FileInfo& fi : child.fileInfos())
        {
            m_points.read(fi);
            totalPoints += fi.numPoints();
        }
    }

    std::deque<int> index(totalPoints);
    std::iota(index.begin(), index.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    /**
    std::vector<int32_t> v{1234};
    std::seed_seq seed(v.begin(), v.end());
    std::mt19937 g(seed);
    **/

    //ABELL - This may not be the best way to do this. Probably better to work from some
    //  point (center, whatever) out, but this is cheap because you don't have to do
    //  any computation with the point data. And I would think you should still get good
    //  output, but it may be more sparse. Seems you could fix that by just choosing a
    //  smaller radius.  Should be tested.
    std::shuffle(index.begin(), index.end(), g);

    while (index.size())
    {
        int i = index.back();

        const Point& p = m_points[i];
        GridKey k = m_vi.gridKey(p);

        // If we're accepting this point into this voxel from it's child, add it
        // to the accepted list and also stick it in the grid.
        if (acceptable(i, k))
        {
            accepted.push_back(i);
            m_vi.grid().insert( {k, i} );
        }
        else
            rejected.push_back(i);

        index.pop_back();
    }
}


void Processor::write(Index& accepted, Index& rejected)
{
    // If this is the final key, append any remaining file infos as accepted points and
    // write the accepted points as compressed.
    if (m_vi.key() == VoxelKey(0, 0, 0, 0))
    {
        appendRemainder(accepted);
        writeOctantCompressed(m_vi.octant(), accepted, accepted.begin());
    }
    else
        writeBinOutput(accepted);
    writeCompressedOutput(rejected);
}


bool Processor::acceptable(int pointId, GridKey key)
{
    //ABELL - Currently unused - see commented-out code.
    (void)pointId;

    VoxelInfo::Grid& grid = m_vi.grid();

    auto it = grid.find(key);

    // If the cell is already occupied the point is not acceptable.
    if (it != grid.end())
        return false;
    return true;
/**
    // We place points in a voxel grid to reduce the number of tests necessary
    // to determine if a new point can be placed without being too close.
    // We size the voxels such that the diagonal is the length of our radius.
    // This way we KNOW that once a cell is occupied, no other point can
    // be placed there. This means the edge length of the voxel cell is
    // radius / √3 = .577 * radius. So, when trying to place a point on the far
    // right side of a cell, it's possible that there's another point already in
    // a cell 2 cells to the right that's only radius * .577 + ε away.

    // ABELL - This should probably be moved to a Grid class.

    // Ignore cells outside of the area of interest.
    int i0 = std::max(key.i() - 2, 0);
    int j0 = std::max(key.j() - 2, 0);
    int k0 = std::max(key.k() - 2, 0);
    int i1 = std::min(key.i() + 2, m_vi.gridXCount());
    int j1 = std::min(key.j() + 2, m_vi.gridYCount());
    int k1 = std::min(key.k() + 2, m_vi.gridZCount());

    for (int i = i0; i <= i1; ++i)
    for (int j = j0; j <= j1; ++j)
    for (int k = k0; k <= k1; ++k)
    {
        //ABELL - Is it worth skipping key location itself or the corner cells?
        auto gi = grid.find(GridKey(i, j, k));
        if (gi != grid.end() && tooClose(pointId, gi->second))
            return false;
    }
    return true;
**/
}


/**
bool Processor::tooClose(pdal::PointId id1, pdal::PointId id2)
{
    const Point& p1 = m_points[id1];
    const Point& p2 = m_points[id2];

    double dx = p1.x() - p2.x();
    double dy = p1.y() - p2.y();
    double dz = p1.z() - p2.z();

    return dx * dx + dy * dy + dz * dz <= m_vi.squareSpacing();
}
**/


void Processor::writeBinOutput(Index& index)
{
    if (index.empty())
        return;

    // Write the accepted points in binary format. Create a FileInfo to describe the
    // file and it to the octant representing this voxel as it bubbles up.
    // Note that we write the the input directory, as this will be input to a later
    // pass.
    std::string filename = m_vi.key().toString() + ".bin";
    std::string fullFilename = m_b.opts.tempDir + "/" + filename;
    std::ofstream out(toNative(fullFilename), std::ios::binary | std::ios::trunc);
    if (!out)
        throw FatalError("Couldn't open '" + fullFilename + "' for output.");
    for (size_t i = 0; i < index.size(); ++i)
        out.write(m_points[index[i]].cdata(), m_b.pointSize);
    m_vi.octant().appendFileInfo(FileInfo(filename, index.size()));
}


// This is a bit confusing.  When we get to the last node, we have two sets of points that
// need to get written to the final (0-0-0-0) node. Those include the points accepted from
// sampling as well as any points that were simply hoisted here due to small size.
//
// We read the hoisted points to stick them on the PointAccessor then we number the points
// by adding them to the index (accepted list).  Then we move the FileInfos from the
void Processor::appendRemainder(Index& index)
{
    std::sort(index.begin(), index.end());

    // Save the current size;
    size_t offset = m_points.size();

    // Read the points from the remaining FileInfos.
    for (FileInfo& fi : m_vi.octant().fileInfos())
        m_points.read(fi);
    size_t numRead = m_points.size() - offset;
    size_t origIndexSize = index.size();

    // Resize the index to contain the read points.
    index.resize(origIndexSize + numRead);

    // The points in the remaining hoisted FileInfos are just numbered sequentially.
    std::iota(index.begin() + origIndexSize, index.end(), offset);

    // NOTE: We need to maintain the order of the file infos as they were read, which
    //   is why they're prepended in reverse.
    // NOTE: The FileInfo pointers in the PointAccessor should remain valid as the
    //   file infos are spliced from the child octant lists onto the parent list.
    for (int i = 8; i > 0; i--)
    {
        FileInfoList& fil = m_vi[i - 1].fileInfos();
        for (auto fi = fil.rbegin(); fi != fil.rend(); ++fi)
            m_vi.octant().prependFileInfo(*fi);
    }
}


void Processor::writeCompressedOutput(Index& index)
{
    // By sorting the rejected points, they will be ordered to match the FileInfo items --
    // meaning that all points that belong in one file will be consecutive.
    std::sort(index.begin(), index.end());

    IndexIter pos = index.begin();

    // If any of our octants has points, we have to write the parent octant, whether or not
    // it contains points, in order to create a full tree.
    for (int octant = 0; octant < 8; ++octant)
        if (m_vi[octant].hasPoints() || m_vi[octant].mustWrite())
        {
            m_vi.octant().setMustWrite(true);
            pos = writeOctantCompressed(m_vi[octant], index, pos);
        }
}


// o        Octant we're writing.
// index    Index of all rejected points that were rejected and not hoisted into the parent.
// pos      Start position of this octant's point in the index.
// \return  Position of the first point in the next octant of our index.
Processor::IndexIter
Processor::writeOctantCompressed(const OctantInfo& o, Index& index, IndexIter pos)
{
    using namespace pdal;

    auto begin = pos;
    PointTable table;
    IndexedStats stats;

    //ABELL - fixme
    // For now we copy the dimension list so we're sure that it matches the layout, though
    // there's no reason why it should change. We should modify things to use a single
    // layout.

    Dimension::IdList lasDims = pdrfDims(m_b.pointFormatId);
    DimInfoList dims = m_b.dimInfo;
    m_extraDims.clear();
    for (FileDimInfo& fdi : dims)
    {
        fdi.dim = table.layout()->registerOrAssignDim(fdi.name, fdi.type);
        if (m_b.opts.stats)
        {
            // For single file output we need the counts by return number.
            if (fdi.dim == pdal::Dimension::Id::Classification)
                stats.push_back({fdi.dim, Stats(fdi.name, Stats::EnumType::Enumerate, false)});
            else if (fdi.dim == pdal::Dimension::Id::ReturnNumber && m_b.opts.singleFile)
                stats.push_back({fdi.dim, Stats(fdi.name, Stats::EnumType::Enumerate, false)});
            else
                stats.push_back({fdi.dim, Stats(fdi.name, Stats::EnumType::NoEnum, false)});
        }
        if (!Utils::contains(lasDims, fdi.dim))
            m_extraDims.push_back(DimType(fdi.dim, fdi.type));
    }
    table.finalize();

    PointViewPtr view(new pdal::PointView(table));

    // The octant's points can came from one or more FileInfo.  The points are sorted such
    // all the points that come from a single FileInfo are consecutive.
    auto fii = o.fileInfos().begin();
    auto fiiEnd = o.fileInfos().end();
    size_t count = 0;

    if (fii != fiiEnd)
    {
        // We're trying to find the range of points that come from a single FileInfo.
        // If pos is the end of the index of the the current file info, append the points
        // to the view.  Otherwise, advance the position.
        while (true)
        {
            if (pos == index.end() || *pos >= fii->start() + fii->numPoints())
            {
                count += std::distance(begin, pos);
                appendCompressed(view, dims, *fii, begin, pos);
                if (pos == index.end())
                    break;
                begin = pos;

                // Advance through file infos as long as we don't have points that
                // correspond to it.
                do
                {
                    fii++;
                    if (fii == fiiEnd)
                        goto flush;
                } while (*begin >= fii->start() + fii->numPoints());
            }
            pos++;
        }
    }
flush:
    try
    {
        flushCompressed(view, o, stats);
    }
    catch (pdal_error& err)
    {
        throw FatalError(err.what());
    }
    m_manager.logOctant(o.key(), count, stats);
    return pos;
}


// Copy data from the source file to the point view.
void Processor::appendCompressed(pdal::PointViewPtr view, const DimInfoList& dims,
    const FileInfo& fi, IndexIter begin, IndexIter end)
{
    //ABELL - This could be improved by making a point table that handles a bunch
    //  of FileInfos/raw addresses. It would totally avoid the copy.
    pdal::PointId pointId = view->size();
    for (IndexIter it = begin; it != end; ++it)
    {
        char *base = fi.address() + ((*it - fi.start()) * m_b.pointSize);
        for (const FileDimInfo& fdi : dims)
            view->setField(fdi.dim, fdi.type, pointId,
                reinterpret_cast<void *>(base + fdi.offset));
        pointId++;
    }
}

void Processor::flushCompressed(pdal::PointViewPtr view, const OctantInfo& oi, IndexedStats& stats)
{
    // For single file output we need the stats for
    if (m_b.opts.stats)
    {
        for (pdal::PointId id = 0; id < view->size(); ++id)
        {
            for (auto& sp : stats)
            {
                pdal::Dimension::Id dim = sp.first;
                Stats& s = sp.second;
                s.insert(view->getFieldAs<double>(dim, id));
            }
        }
    }

    if (m_b.opts.singleFile)
    {
        createChunk(oi.key(), view);
    }
    else
    {
        std::string filename = m_b.opts.outputName + "/ept-data/" + oi.key().toString() + ".laz";
        writeEptFile(filename, view);
    }
}

void Processor::writeEptFile(const std::string& filename, pdal::PointViewPtr view)
{
    using namespace pdal;

    StageFactory factory;

    BufferReader r;
    r.addView(view);

    Stage *prev = &r;

    if (view->layout()->hasDim(Dimension::Id::GpsTime))
    {
        Stage *f = factory.createStage("filters.sort");
        pdal::Options fopts;
        fopts.add("dimension", "gpstime");
        f->setOptions(fopts);
        f->setInput(*prev);
        prev = f;
    }

    Stage *w = factory.createStage("writers.las");
    pdal::Options wopts;
    wopts.add("extra_dims", "all");
    wopts.add("software_id", "Entwine 1.0");
    wopts.add("compression", "laszip");
    wopts.add("filename", filename);
    wopts.add("offset_x", m_b.offset[0]);
    wopts.add("offset_y", m_b.offset[1]);
    wopts.add("offset_z", m_b.offset[2]);
    wopts.add("scale_x", m_b.scale[0]);
    wopts.add("scale_y", m_b.scale[1]);
    wopts.add("scale_z", m_b.scale[2]);
    wopts.add("minor_version", 4);
    wopts.add("dataformat_id", m_b.pointFormatId);
    if (m_b.opts.a_srs.size())
        wopts.add("a_srs", m_b.opts.a_srs);
    if (m_b.opts.metadata)
        wopts.add("pdal_metadata", m_b.opts.metadata);
    w->setOptions(wopts);
    w->setInput(*prev);

    w->prepare(view->table());
    w->execute(view->table());
}

void Processor::sortChunk(pdal::PointViewPtr view)
{
    pdal::BufferReader r;
    r.addView(view);

    pdal::SortFilter s;
    s.setInput(r);
    pdal::Options o;
    o.add("dimension", "GpsTime");
    s.setOptions(o);

    s.prepare(view->table());
    s.execute(view->table());
}

void Processor::createChunk(const VoxelKey& key, pdal::PointViewPtr view)
{
    using namespace pdal;

    if (view->size() == 0)
    {
        m_manager.newChunk(key, 0, 0);
        return;
    }

    // Sort the chunk on GPS time.
    if (view->layout()->hasDim(Dimension::Id::GpsTime))
        sortChunk(view);

    PointLayoutPtr layout = view->layout();

    int ebCount {0};
    for (DimType dim : m_extraDims)
        ebCount += layout->dimSize(dim.m_id);

    std::vector<char> buf(lazperf::baseCount(m_b.pointFormatId) + ebCount);
    lazperf::writer::chunk_compressor compressor(m_b.pointFormatId, ebCount);
    for (PointId idx = 0; idx < view->size(); ++idx)
    {
        PointRef point(*view, idx);
        fillPointBuf(point, buf);
        compressor.compress(buf.data());
    }
    std::vector<unsigned char> chunk = compressor.done();

    uint64_t location = m_manager.newChunk(key, chunk.size(), (uint32_t)view->size());

    std::ofstream out(toNative(m_b.opts.outputName),
        std::ios::out | std::ios::in | std::ios::binary);
    out.seekp(std::ofstream::pos_type(location));
    out.write(reinterpret_cast<const char *>(chunk.data()), chunk.size());
    out.close();
    if (!out)
        throw FatalError("Failure writing to '" + m_b.opts.outputName + "'.");
}

void Processor::fillPointBuf(pdal::PointRef& point, std::vector<char>& buf)
{
    using namespace pdal;

    LeInserter ostream(buf.data(), buf.size());

    // We only write PDRF 6, 7, or 8.
    bool has14PointFormat = true;
    bool hasTime = true; //  m_lasHeader.hasTime();
    bool hasColor = m_b.pointFormatId == 7 || m_b.pointFormatId == 8;
    bool hasInfrared = m_b.pointFormatId == 8;

    // we always write the base fields
    using namespace Dimension;

    uint8_t returnNumber(1);
    uint8_t numberOfReturns(1);
    if (point.hasDim(Id::ReturnNumber))
        returnNumber = point.getFieldAs<uint8_t>(Id::ReturnNumber);
    if (point.hasDim(Id::NumberOfReturns))
        numberOfReturns = point.getFieldAs<uint8_t>(Id::NumberOfReturns);

    auto converter = [](double d, Dimension::Id dim) -> int32_t
    {
        int32_t i(0);

        if (!Utils::numericCast(d, i))
            throw FatalError("Unable to convert scaled value (" +
                Utils::toString(d) + ") to "
                "int32 for dimension '" + Dimension::name(dim) +
                "' when writing LAS/LAZ file.");
        return i;
    };

    double x = (point.getFieldAs<double>(Id::X) - m_b.offset[0]) / m_b.scale[0];
    double y = (point.getFieldAs<double>(Id::Y) - m_b.offset[1]) / m_b.scale[1];
    double z = (point.getFieldAs<double>(Id::Z) - m_b.offset[2]) / m_b.scale[2];

    ostream << converter(x, Id::X);
    ostream << converter(y, Id::Y);
    ostream << converter(z, Id::Z);

    ostream << point.getFieldAs<uint16_t>(Id::Intensity);

    uint8_t scanChannel = point.getFieldAs<uint8_t>(Id::ScanChannel);
    uint8_t scanDirectionFlag = point.getFieldAs<uint8_t>(Id::ScanDirectionFlag);
    uint8_t edgeOfFlightLine = point.getFieldAs<uint8_t>(Id::EdgeOfFlightLine);
    uint8_t classification = point.getFieldAs<uint8_t>(Id::Classification);

    if (has14PointFormat)
    {
        uint8_t bits = returnNumber | (numberOfReturns << 4);
        ostream << bits;

        uint8_t classFlags;
        if (point.hasDim(Id::ClassFlags))
            classFlags = point.getFieldAs<uint8_t>(Id::ClassFlags);
        else
            classFlags = classification >> 5;
        bits = (classFlags & 0x0F) |
            ((scanChannel & 0x03) << 4) |
            ((scanDirectionFlag & 0x01) << 6) |
            ((edgeOfFlightLine & 0x01) << 7);
        ostream << bits;
    }
    else
    {
        uint8_t bits = returnNumber | (numberOfReturns << 3) |
            (scanDirectionFlag << 6) | (edgeOfFlightLine << 7);
        ostream << bits;
    }

    ostream << classification;

    uint8_t userData = point.getFieldAs<uint8_t>(Id::UserData);
    if (has14PointFormat)
    {
         // Guaranteed to fit if scan angle rank isn't wonky.
        int16_t scanAngleRank =
            static_cast<int16_t>(std::round(
                point.getFieldAs<float>(Id::ScanAngleRank) / .006f));
        ostream << userData << scanAngleRank;
    }
    else
    {
        int8_t scanAngleRank = point.getFieldAs<int8_t>(Id::ScanAngleRank);
        ostream << scanAngleRank << userData;
    }

    ostream << point.getFieldAs<uint16_t>(Id::PointSourceId);

    if (hasTime)
        ostream << point.getFieldAs<double>(Id::GpsTime);

    if (hasColor)
    {
        ostream << point.getFieldAs<uint16_t>(Id::Red);
        ostream << point.getFieldAs<uint16_t>(Id::Green);
        ostream << point.getFieldAs<uint16_t>(Id::Blue);
    }

    if (hasInfrared)
        ostream << point.getFieldAs<uint16_t>(Id::Infrared);

    Everything e;
    for (auto& dim : m_extraDims)
    {
        point.getField((char *)&e, dim.m_id, dim.m_type);
        Utils::insertDim(ostream, dim.m_type, e);
    }
}

} // namespace bu
} // namespace untwine
