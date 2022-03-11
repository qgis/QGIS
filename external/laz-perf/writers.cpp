/*
===============================================================================

  FILE:  io.cpp

  CONTENTS:
    LAZ io

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#include "las.hpp"
#include "lazperf.hpp"
#include "streams.hpp"
#include "vlr.hpp"
#include "writers.hpp"

namespace lazperf
{
namespace writer
{

struct basic_file::Private
{
    Private() : chunk_size(DefaultChunkSize), head12(head14), head13(head14),
        f(nullptr)
    {}

    void close();
    uint64_t newChunk();
    uint64_t firstChunkOffset() const;
    bool compressed() const;
    bool open(std::ostream& out, const header12& h, uint32_t chunk_size);
    void writePoint(const char *p);
    void updateMinMax(const las::point10& p);
    void writeHeader();
    void writeChunks();
    void writeChunkTable();

    uint32_t chunk_point_num;
    uint32_t chunk_size;
    std::vector<chunk> chunks;
    las_compressor::ptr pcompressor;
    header12& head12;
    header13& head13;
    header14 head14;
    std::ostream *f;  // Pointer because we don't have a reference target at construction.
    std::unique_ptr<OutFileStream> stream;
};

struct named_file::Private
{
    using Base = basic_file::Private;

    Private(Base *b) : base(b)
    {}

    void open(const std::string& filename, const named_file::config& c);

    Base *base;
    std::ofstream file;
};

// On compressed output, the data is written in chunks. Normally the chunks are 50,000
// points, but you can request variable sized chunking by using the value "VariableChunkSize".
// When using variable-sized chunks, you must call newChunk() in order to start a new chunk.
//
// An offset to the chunk table is written after the LAS VLRs, where you would normally
// expect points to begin.  The first chunk of points follows the 8 byte chunk table offset.
//
// The chunk table itself is at the end of the point chunks. It has its own
// header that consists of a version number and a chunk count. The chunk table entries
// are compressed with an integer compressor.
// 
// A chunk table entry is created after each chunk of points has been created and written.
// If the chunks are variable size, the chunk table entries consist of a count followed by
// an "offset".  The count is the number of points in the chunk. The offset is the the number
// of bytes written in the chunk.  If the chunks are fixed size, the count entry is not written.
//
// Note that after being read, the table is fixed up to be usable when reading
// points.

bool basic_file::Private::open(std::ostream& out, const header12& h, uint32_t cs)
{
    if (h.version.major != 1 || h.version.minor < 2 || h.version.minor > 4)
        return false;

    f = &out;
    head12 = h;
    chunk_size = cs;
    writeHeader();

    if (compressed())
    {
        // Seek past the chunk table offset.
        out.seekp(sizeof(uint64_t), std::ios_base::cur);
    }
    stream.reset(new OutFileStream(out));
    return true;
}

bool basic_file::Private::compressed() const
{
    return chunk_size > 0;
}

void basic_file::Private::updateMinMax(const las::point10& p)
{
    double x = p.x * head12.scale.x + head12.offset.x;
    double y = p.y * head12.scale.y + head12.offset.y;
    double z = p.z * head12.scale.z + head12.offset.z;

    head12.minx = (std::min)(x, head12.minx);
    head12.miny = (std::min)(y, head12.miny);
    head12.minz = (std::min)(z, head12.minz);

    head12.maxx = (std::max)(x, head12.maxx);
    head12.maxy = (std::max)(y, head12.maxy);
    head12.maxz = (std::max)(z, head12.maxz);
}

uint64_t basic_file::Private::newChunk()
{
    pcompressor->done();

    uint64_t position = (uint64_t)f->tellp();
    chunks.push_back({ chunk_point_num, position });
    pcompressor = build_las_compressor(stream->cb(), head12.pointFormat(), head12.ebCount());
    chunk_point_num = 0;
    return position;
}

uint64_t basic_file::Private::firstChunkOffset() const
{
    // There is a chunk offset where the first point is supposed to be. The first
    // chunk follows that.
    return head12.point_offset + sizeof(uint64_t);
}

void basic_file::Private::writePoint(const char *p)
{
    if (!compressed())
        stream->putBytes(reinterpret_cast<const unsigned char *>(p), head12.point_record_length);
    else
    {
        //ABELL - This first bit can go away if we simply always create compressor.
        if (!pcompressor)
        {
            pcompressor = build_las_compressor(stream->cb(), head12.pointFormat(),
                head12.ebCount());
            chunk_point_num = 0;
        }
        else if ((chunk_point_num == chunk_size) && (chunk_size != VariableChunkSize))
            newChunk();

        // now write the point
        pcompressor->compress(p);
        chunk_point_num++;
        head14.point_count_14++;
    }
    updateMinMax(*(reinterpret_cast<const las::point10*>(p)));
}

void basic_file::Private::close()
{
    if (compressed())
    {
        pcompressor->done();
        chunks.push_back({ chunk_point_num, (uint64_t)f->tellp() });
    }

    writeHeader();
    if (compressed())
        writeChunkTable();
}

void basic_file::Private::writeHeader()
{
    laz_vlr lazVlr(head14.pointFormat(), head14.ebCount(), chunk_size);
    eb_vlr ebVlr(head14.ebCount());

    // Set the version number to 2 in order to write something reasonable.
    if (head14.version.minor < 2 || head14.version.minor > 4)
        head14.version.minor = 2;

    // point_format_id and point_record_length  are set on open().
    head14.header_size = head14.sizeFromVersion();
    head14.point_offset = head14.header_size;
    head14.vlr_count = 0;
    if (compressed())
    {
        head14.vlr_count++;
        head14.point_format_id |= (1 << 7);
        head14.point_offset += lazVlr.size() + lazVlr.header().Size;
    }
    if (head14.ebCount())
    {
        head14.point_offset += ebVlr.size() + ebVlr.header().Size;
        head14.vlr_count++;
    }

    if (head14.version.minor == 4)
    {
        if (head14.point_count_14 > (std::numeric_limits<uint32_t>::max)())
            head14.point_count = 0;
        else
            head14.point_count = (uint32_t)head14.point_count_14;
        // Set the WKT bit.
        head14.global_encoding |= (1 << 4);
    }
    else
        head14.point_count = (uint32_t)head14.point_count_14;
    f->seekp(0);
    if (head14.version.minor == 2)
        head12.write(*f);
    else if (head14.version.minor == 3)
        head13.write(*f);
    else if (head14.version.minor == 4)
        head14.write(*f);

    if (compressed())
    {
        // Write the VLR.
        lazVlr.header().write(*f);
        lazVlr.write(*f);
    }
    if (head14.ebCount())
    {
        ebVlr.header().write(*f);
        ebVlr.write(*f);
    }
}

void basic_file::Private::writeChunkTable()
{
    // move to the end of the file to start emitting our compresed table
    f->seekp(0, std::ios::end);

    // take note of where we're writing the chunk table, we need this later
    int64_t chunk_table_offset = static_cast<int64_t>(f->tellp());

    // Fixup the chunk table to be relative offsets rather than absolute ones.
    uint64_t prevOffset = firstChunkOffset();
    for (chunk& c : chunks)
    {
        uint64_t relOffset = c.offset - prevOffset;
        prevOffset = c.offset;
        c.offset = relOffset;
    }

    // write out the chunk table header (version and total chunks)
    uint32_t version = 0;
    f->write((const char *)&version, sizeof(uint32_t));
    uint32_t numChunks = htole32((uint32_t)chunks.size());
    f->write((const char *)&numChunks, sizeof(uint32_t));

    // Write the chunk table
    OutFileStream w(*f);
    OutCbStream outStream(w.cb());

    compress_chunk_table(w.cb(), chunks, chunk_size == VariableChunkSize);
    // go back to where we're supposed to write chunk table offset
    f->seekp(head12.point_offset);
    f->write(reinterpret_cast<char*>(&chunk_table_offset), sizeof(chunk_table_offset));
}


basic_file::basic_file() : p_(new Private())
{}

basic_file::~basic_file()
{}

bool basic_file::compressed() const
{
    return p_->compressed();
}

bool basic_file::open(std::ostream& out, const header12& h, uint32_t chunk_size)
{
   return  p_->open(out, h, chunk_size);
}

void basic_file::writePoint(const char *buf)
{
    p_->writePoint(buf);
}

uint64_t basic_file::firstChunkOffset() const
{
    return p_->firstChunkOffset();
}

uint64_t basic_file::newChunk()
{
    assert(p_->chunk_size == VariableChunkSize);
    return p_->newChunk();
}

void basic_file::close()
{
    p_->close();
}

// named_file

named_file::config::config() : scale(1.0, 1.0, 1.0), offset(0.0, 0.0, 0.0),
    chunk_size(DefaultChunkSize), pdrf(0), minor_version(3), extra_bytes(0)
{}

named_file::config::config(const vector3& s, const vector3& o, unsigned int cs) :
    scale(s), offset(o), chunk_size(cs), pdrf(0), minor_version(3), extra_bytes(0)
{}

named_file::config::config(const header12& h) : scale(h.scale.x, h.scale.y, h.scale.z),
    offset(h.offset.x, h.offset.y, h.offset.z), chunk_size(DefaultChunkSize),
    pdrf(h.point_format_id), minor_version(h.version.minor), extra_bytes(h.ebCount())
{}

header12 named_file::config::to_header() const
{
    header12 h;

    h.minx = (std::numeric_limits<double>::max)();
    h.miny = (std::numeric_limits<double>::max)();
    h.minz = (std::numeric_limits<double>::max)();
    h.maxx = std::numeric_limits<double>::lowest();
    h.maxy = std::numeric_limits<double>::lowest();
    h.maxz = std::numeric_limits<double>::lowest();

    h.version.minor = minor_version;
    h.point_format_id = pdrf;
    h.point_record_length = baseCount(pdrf) + extra_bytes;

    h.offset.x = offset.x;
    h.offset.y = offset.y;
    h.offset.z = offset.z;

    h.scale.x = scale.x;
    h.scale.y = scale.y;
    h.scale.z = scale.z;

    return h;
}

void named_file::Private::open(const std::string& filename, const named_file::config& c)
{
    header12 h = c.to_header();

    // open the file and move to offset of data, we'll write
    // headers and all other things on file close
    file.open(filename, std::ios::binary | std::ios::trunc);
    if (!file.good())
        throw error("Couldn't open '" + filename + "' for writing.");
    base->open(file, h, c.chunk_size);
}


named_file::named_file(const std::string& filename, const named_file::config& c) :
    p_(new Private(basic_file::p_.get()))
{
    p_->open(filename, c);
}    

named_file::~named_file()
{}

void named_file::close()
{
    basic_file::close();
    if (p_->file.is_open())
        p_->file.close();
}

// Chunk compressor

struct chunk_compressor::Private
{
    las_compressor::ptr pcompressor;
    MemoryStream stream;
};

chunk_compressor::~chunk_compressor()
{}

chunk_compressor::chunk_compressor(int format, int ebCount) : p_(new Private)
{
    p_->pcompressor = build_las_compressor(p_->stream.outCb(), format, ebCount);
}

void chunk_compressor::compress(const char *inbuf)
{
    p_->pcompressor->compress(inbuf);
}

std::vector<unsigned char> chunk_compressor::done()
{
    p_->pcompressor->done();
    return p_->stream.buffer();
}

} // namespace writer
} // namespace lazperf

