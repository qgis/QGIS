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

#include <iostream>
#include <limits>

#include "CopcSupport.hpp"

#include <pdal/PointLayout.hpp>
#include <pdal/util/Algorithm.hpp>
#include <pdal/util/OStream.hpp>
#include <pdal/util/Extractor.hpp>

#include <lazperf/filestream.hpp>

#include "../untwine/Common.hpp"
#include "../untwine/FileDimInfo.hpp"
#include "../untwine/Las.hpp"

namespace untwine
{
namespace bu
{

CopcSupport::CopcSupport(const BaseInfo& b) : m_b(b),
    m_lazVlr(b.pointFormatId, extraByteSize(), lazperf::VariableChunkSize),
    m_ebVlr(),
    m_wktVlr(b.srs.getWKT())
{
    m_f.open(toNative(b.opts.outputName), std::ios::out | std::ios::binary);

    //ABELL
    m_header.file_source_id = 0;
    m_header.global_encoding = (1 << 4); // Set for WKT
    //ABELL
    m_header.creation.day = 1;
    m_header.creation.year = 1;
    m_header.header_size = lazperf::header14::Size;
    m_header.point_format_id = m_b.pointFormatId;
    m_header.point_format_id |= (1 << 7);    // Bit for laszip
    m_header.point_record_length = lazperf::baseCount(m_b.pointFormatId) + extraByteSize();
    m_header.scale.x = b.scale[0];
    m_header.scale.y = b.scale[1];
    m_header.scale.z = b.scale[2];
    m_header.offset.x = b.offset[0];
    m_header.offset.y = b.offset[1];
    m_header.offset.z = b.offset[2];
    m_header.vlr_count = 3;

    //IMPORTANT: We have to calculate the point offset here so that we can start writing
    // points to the proper location immediately. This means knowing the sizes of the VLRs
    // we're going to write at this time as well.
    m_header.point_offset = lazperf::header14::Size +
        lazperf::vlr_header::Size + m_copcVlr.size() +
        lazperf::vlr_header::Size + m_lazVlr.size() +
        lazperf::vlr_header::Size + m_wktVlr.size();
    if (m_header.ebCount())
    {
        addEbFields();
        m_header.vlr_count++;
        m_header.point_offset += (uint32_t)(lazperf::vlr_header::Size + m_ebVlr.size());
    }

    // The chunk table offset is written as the first 8 bytes of the point data in LAZ.
    m_chunkOffsetPos = m_header.point_offset;
    // The actual point data comes after the chunk table offset.
    m_pointPos = m_chunkOffsetPos + sizeof(uint64_t);
}

int CopcSupport::extraByteSize() const
{
    int size = 0;
    for (const FileDimInfo& fdi : m_b.dimInfo)
        if (fdi.extraDim)
            size += pdal::Dimension::size(fdi.type);
    return size;
}

void CopcSupport::addEbFields()
{
    using DT = pdal::Dimension::Type;

    auto lasType = [](DT type) -> uint8_t
    {
        switch (type)
        {
        case DT::Unsigned8:
            return 1;
        case DT::Signed8:
            return 2;
        case DT::Unsigned16:
            return 3;
        case DT::Signed16:
            return 4;
        case DT::Unsigned32:
            return 5;
        case DT::Signed32:
            return 6;
        case DT::Unsigned64:
            return 7;
        case DT::Signed64:
            return 8;
        case DT::Float:
            return 9;
        case DT::Double:
            return 10;
        default:
            return 0;
        }
    };

    for (const FileDimInfo& fdi : m_b.dimInfo)
    {
        if (fdi.extraDim)
        {
            lazperf::eb_vlr::ebfield f;
            f.data_type = lasType(fdi.type);
            f.name = fdi.name;
            m_ebVlr.addField(f);
        }
    }
}

/// \param  size  Size of the chunk in bytes
/// \param  count  Number of points in the chunk
/// \param  key  Key of the voxel the chunk represents
/// \return  The offset of the chunk in the file.
uint64_t CopcSupport::newChunk(const VoxelKey& key, int32_t size, int32_t count)
{
    // Chunks of size zero are a special case.
    if (count == 0)
    {
        m_hierarchy[key] = { 0, 0, 0 };
        return 0;
    }

    uint64_t chunkStart = m_pointPos;
    m_pointPos += size;
    assert(count <= (std::numeric_limits<int32_t>::max)() && count >= 0);
    m_chunkTable.push_back({(uint64_t)count, (uint64_t)size});
    m_header.point_count += count;
    m_header.point_count_14 += count;
    m_hierarchy[key] = { chunkStart, size, count };
    return chunkStart;
}

void CopcSupport::updateHeader(const StatsMap& stats)
{
    using namespace pdal::Dimension;

    m_header.maxx = stats.at(Id::X).maximum();
    m_header.maxy = stats.at(Id::Y).maximum();
    m_header.maxz = stats.at(Id::Z).maximum();
    m_header.minx = stats.at(Id::X).minimum();
    m_header.miny = stats.at(Id::Y).minimum();
    m_header.minz = stats.at(Id::Z).minimum();

    m_copcVlr.gpstime_minimum = 0.0f;
    m_copcVlr.gpstime_maximum = 0.0f;
    if (stats.count(Id::GpsTime))
    {
        m_copcVlr.gpstime_minimum = stats.at(Id::GpsTime).minimum();
        m_copcVlr.gpstime_maximum = stats.at(Id::GpsTime).maximum();
    }

    for (int i = 1; i <= 15; ++i)
    {
        PointCount count = 0;
        try
        {
            count = stats.at(Id::ReturnNumber).values().at(i);
        }
        catch (const std::out_of_range&)
        {}

        m_header.points_by_return_14[i - 1] = count;
        if (i <= 5)
        {
            if (m_header.points_by_return_14[i] <= (std::numeric_limits<uint32_t>::max)())
                m_header.points_by_return[i - 1] = (uint32_t)m_header.points_by_return_14[i - 1];
            else
                m_header.points_by_return[i - 1] = 0;
        }
    }

    if (m_header.point_count_14 > (std::numeric_limits<uint32_t>::max)())
        m_header.point_count = 0;
}


void CopcSupport::writeHeader()
{
    std::ostream& out = m_f;

    out.seekp(0);
    m_header.write(out);

    m_copcVlr.header().write(out);
    uint64_t copcPos = out.tellp();
    m_copcVlr.center_x = (m_b.bounds.maxx / 2) + (m_b.bounds.minx / 2);
    m_copcVlr.center_y = (m_b.bounds.maxy / 2) + (m_b.bounds.miny / 2);
    m_copcVlr.center_z = (m_b.bounds.maxz / 2) + (m_b.bounds.minz / 2);
    m_copcVlr.halfsize = (m_b.bounds.maxx - m_b.bounds.minx) / 2;
    m_copcVlr.spacing = 2 * m_copcVlr.halfsize / CellCount;
    m_copcVlr.write(out);

    m_lazVlr.header().write(out);
    m_lazVlr.write(out);

    m_wktVlr.header().write(out);
    m_wktVlr.write(out);

    if (m_header.ebCount())
    {
        m_ebVlr.header().write(out);
        m_ebVlr.write(out);
    }

    uint64_t end = out.tellp();

    // Rewrite the COPC VLR with the updated positions and seek back to the end of the VLRs.
    out.seekp(copcPos);
    m_copcVlr.write(out);
    out.seekp(end);
}

void CopcSupport::writeChunkTable()
{
    m_chunkTable.resize(m_chunkTable.size());

    // Write chunk table offset
    pdal::OLeStream out(&m_f);
    out.seek(m_chunkOffsetPos);
    out << m_pointPos;

    // Write chunk table header.
    out.seek(m_pointPos);
    uint32_t version = 0;
    out << version;
    out << (uint32_t)m_chunkTable.size();

    // Write the chunk table itself.
    lazperf::OutFileStream stream(m_f);
    lazperf::compress_chunk_table(stream.cb(), m_chunkTable, true);

    // The ELVRs start after the chunk table.
    m_header.evlr_count = 1;
    m_header.evlr_offset = out.position();
}

void CopcSupport::writeHierarchy(const CountMap& counts)
{
    // Move to the location *after* the EVLR header.
    m_f.seekp(m_header.evlr_offset + lazperf::evlr_header::Size);

    uint64_t beginPos = m_f.tellp();
    Hierarchy root = emitRoot(VoxelKey(0, 0, 0, 0), counts);
    m_copcVlr.root_hier_offset = root.offset;
    m_copcVlr.root_hier_size = root.byteSize;
    uint64_t endPos = m_f.tellp();

    // Now write VLR header.
    lazperf::evlr_header h { 0, "copc", 1000, (endPos - beginPos), "EPT Hierarchy" };
    m_f.seekp(m_header.evlr_offset);
    h.write(m_f);
}

CopcSupport::Hierarchy CopcSupport::emitRoot(const VoxelKey& root, const CountMap& counts)
{
    const int LevelBreak = 4;
    Entries entries;

    int stopLevel = root.level() + LevelBreak;
    entries.push_back({root, m_hierarchy[root]});
    emitChildren(root, counts, entries, stopLevel);

    pdal::OLeStream out(&m_f);
    uint64_t startPos = out.position();
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        VoxelKey& key = it->first;
        Hierarchy& h = it->second;
        out << key.level() << key.x() << key.y() << key.z();
        out << h.offset << h.byteSize << h.pointCount;
    }
    uint64_t endPos = out.position();

    // This is the information about where the hierarchy node was written, to be
    // written with the parent.
    return { startPos, (int32_t)(endPos - startPos), -1 };
}


void CopcSupport::emitChildren(const VoxelKey& p, const CountMap& counts,
    Entries& entries, int stopLevel)
{
    const int MinHierarchySize = 50;

    for (int i = 0; i < 8; ++i)
    {
        VoxelKey c = p.child(i);
        auto ci = counts.find(c);
        if (ci != counts.end())
        {
            // If we're not at a stop level or the number of child nodes is less than 50,
            // just stick them here.
            if (c.level() != stopLevel || ci->second <= MinHierarchySize)
            {
                entries.push_back({c, m_hierarchy[c]});
                emitChildren(c, counts, entries, stopLevel);
            }
            else
                entries.push_back({c, emitRoot(c, counts)});
        }
    }
}

} // namespace bu
} // namespace untwine
