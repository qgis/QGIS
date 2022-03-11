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

#include <string>

#include "readers.hpp"
#include "charbuf.hpp"
#include "decoder.hpp"
#include "decompressor.hpp"
#include "excepts.hpp"
#include "filestream.hpp"
#include "streams.hpp"
#include "vlr.hpp"

namespace lazperf
{
namespace reader
{

struct basic_file::Private
{
    Private() : head12(head14), head13(head14), compressed(false), current_chunk(nullptr)
    {}

    bool open(std::istream& f);
    uint64_t firstChunkOffset() const;
    void readPoint(char *out);
    bool loadHeader();
    uint64_t pointCount() const;
    void parseVLRs();
    bool extractVlr(const std::string& user_id, uint16_t record_id, uint64_t data_length);
    std::vector<char> vlrData(const std::string& user_id, uint16_t record_id);
    void parseChunkTable();
    void validateHeader();

    std::istream *f;
    std::unique_ptr<InFileStream> stream;
    header12& head12;
    header13& head13;
    header14 head14;
    bool compressed;
    las_decompressor::ptr pdecompressor;
    laz_vlr laz;
    eb_vlr eb;
    chunk *current_chunk;
    uint32_t chunk_point_num;
    std::vector<chunk> chunks;
    std::vector<vlr_index_rec> vlr_index;
};

struct mem_file::Private
{
    Private(char *buf, size_t count) : sbuf(buf, count), f(&sbuf)
    {}

    charbuf sbuf;
    std::istream f;
};

struct named_file::Private
{
    Private(const std::string& filename) : f(filename, std::ios::binary)
    {}

    std::ifstream f;
};

// reader::basic_file

bool basic_file::Private::open(std::istream& in)
{
    f = &in;
    //ABELL - move to loadHeader() in order to avoid the reset on InFileStream.
    stream.reset(new InFileStream(in));
    return loadHeader();
}

uint64_t basic_file::Private::firstChunkOffset() const
{
    // There is a chunk offset where the first point is supposed to be. The first
    // chunk follows that.
    return head12.point_offset + sizeof(uint64_t);
}

void basic_file::Private::readPoint(char *out)
{
    if (!compressed)
        stream->cb()(reinterpret_cast<unsigned char *>(out), head12.point_record_length);

    // read the next point
    else
    {
        if (!pdecompressor || chunk_point_num == current_chunk->count)
        {
            pdecompressor = build_las_decompressor(stream->cb(), head12.point_format_id,
                head12.ebCount());

            // reset chunk state
            if (current_chunk == nullptr)
                current_chunk = chunks.data();
            else
                current_chunk++;
            chunk_point_num = 0;
        }

        pdecompressor->decompress(out);
        chunk_point_num++;
    }
}

bool basic_file::Private::loadHeader()
{
    std::vector<char> buf(header14::Size);

    f->seekg(0);
    head12.read(*f);

    if (std::string(head12.magic, head12.magic + 4) != "LASF")
        throw error("Invalid LAS file. Incorrect magic number.");

    // If we're version 3 or 4, back up and do it again.
    if (head12.version.minor == 3)
    {
        f->seekg(0);
        head13.read(*f);
    }
    else if (head12.version.minor == 4)
    {
        f->seekg(0);
        head14.read(*f);
    }
    if (head12.version.minor < 2 || head12.version.minor > 4)
        return false;

    if (head12.compressed())
        compressed = true;

    parseVLRs();

    if (compressed)
    {
        validateHeader();
        parseChunkTable();
    }

    // set the file pointer to the beginning of data to start reading
    // may have treaded past the EOL, so reset everything before we start reading
    f->clear();
    uint64_t offset = head12.point_offset;
    if (compressed)
        offset += sizeof(int64_t);
    f->seekg(offset);
    stream->reset();
    return true;
}


uint64_t basic_file::Private::pointCount() const
{
    if (head12.version.major > 1 || head12.version.minor > 3)
        return head14.point_count_14;
    return head12.point_count;
}


void basic_file::Private::parseVLRs()
{
    // move the pointer to the begining of the VLRs
    f->seekg(head12.header_size);

    // Search VLRs
    size_t count = 0;
    while (count < head12.vlr_count && f->good() && !f->eof())
    {
        vlr_header h = vlr_header::create(*f);
        vlr_index.emplace_back(h, f->tellg());

        // If we don't read the VLR, seek past it.
        if (!extractVlr(h.user_id, h.record_id, h.data_length))
            f->seekg(h.data_length, std::ios::cur); // jump forward
        count++;
    }

    // Search EVLRs
    if (head14.evlr_count && head14.evlr_offset != 0)
    {
        f->seekg(head14.evlr_offset);

        size_t count = 0;
        while (count < head14.evlr_count && f->good() && !f->eof())
        {
            evlr_header h = evlr_header::create(*f);
            vlr_index.emplace_back(h, f->tellg());

            // If we don't read the VLR, seek past it.
            if (!extractVlr(h.user_id, h.record_id, h.data_length))
                f->seekg(h.data_length, std::ios::cur); // jump forward
            count++;
        }
    }

    if (compressed && !laz.valid())
        throw error("Couldn't find LASZIP VLR");
}

bool basic_file::Private::extractVlr(const std::string& user_id, uint16_t record_id,
    uint64_t data_length)
{
    // Extract LASzip VLR
    if (user_id == "laszip encoded" && record_id == 22204)
    {
        laz.read(*f);
        if ((head12.pointFormat() <= 5 && laz.compressor != 2) ||
                (head12.pointFormat() > 5 && laz.compressor != 3))
            throw error(std::string("Mismatch between point format of ") +
                    std::to_string(head12.pointFormat()) + " and compressor version of " +
                    std::to_string((int)laz.compressor) + ".");
        return true;
    }
    // Extract EB VLR
    else if (user_id == "LASF_Spec" && record_id == 4)
    {
        eb.read(*f, data_length);
        return true;
    }
    return false;
}

std::vector<char> basic_file::Private::vlrData(const std::string& user_id, uint16_t record_id)
{
    std::vector<char> data;

    // Go through the VLR index looking for the one we want. If we find it, seek to the data
    // location, read the data into the vector, then seek pack where we were.
    for (vlr_index_rec& rec : vlr_index)
        if (rec.user_id == user_id && rec.record_id == record_id)
        {
            auto position = f->tellg();
            f->seekg(rec.byte_offset);
            data.resize(rec.data_length);
            f->read(data.data(), rec.data_length);
            f->seekg(position);
            break;
        }
    return data;
}

void basic_file::Private::parseChunkTable()
{
    // Move to the begining of the data
    f->seekg(head12.point_offset);

    int64_t chunkoffset = 0;
    f->read((char*)&chunkoffset, sizeof(chunkoffset));
    if (!f->good())
        throw error("Couldn't read chunk table.");

    if (chunkoffset == -1)
        throw error("Chunk table offset == -1 is not supported at this time");

    // Go to the chunk offset and read in the table
    f->seekg(chunkoffset);
    if (!f->good())
        throw error("Error reading chunk table.");

    // Now read in the chunk table
#pragma pack(push, 1)
    struct
    {
        uint32_t version;
        uint32_t chunk_count;
    } chunk_table_header;
#pragma pack(pop)

    f->read((char *)&chunk_table_header, sizeof(chunk_table_header));
    if (!f->good())
        throw error("Error reading chunk table.");

    if (chunk_table_header.version != 0)
        throw error("Bad chunk table. Invalid version.");

    // Allocate enough room for the chunk table plus one because of the crazy way that
    // the chunk table is written. Once it is fixed up, we resize back to the correct size.
    chunks.resize(chunk_table_header.chunk_count + 1);

    // decode the index out
    InFileStream fstream(*f);

    InCbStream stream(fstream.cb());
    decoders::arithmetic<InCbStream> decoder(stream);
    decompressors::integer decomp(32, 2);

    // start decoder
    decoder.readInitBytes();
    decomp.init();

    uint32_t prev_count = 0;
    uint32_t prev_offset = 0;
    uint64_t total_points = pointCount();
    chunks[0] = { 0, firstChunkOffset() };
    for (size_t i = 0; i < chunk_table_header.chunk_count; i++)
    {
        uint32_t count;

        if (laz.chunk_size == VariableChunkSize)
        {
            count = decomp.decompress(decoder, prev_count, 0);
            prev_count = count;
        }
        else
        {
            if (total_points < laz.chunk_size)
            {
                count = total_points;
                assert(i == chunk_table_header.chunk_count - 1);
            }
            else
            {
                count = laz.chunk_size;
                total_points -= laz.chunk_size;
            }
        }

        uint32_t offset = decomp.decompress(decoder, prev_offset, 1);
        prev_offset = offset;

        chunks[i].count = count;
        chunks[i + 1].offset = offset + chunks[i].offset;
    }

    // This discards the last offset, which we don't care about. The last count is
    // never filled in.
    chunks.resize(chunk_table_header.chunk_count);
    /**
    for (auto& chunk : chunks)
        std::cerr << "Count/offset = " << chunk.count << "/" << chunk.offset << "!\n";
    std::cerr << "\n";
    **/
}

void basic_file::Private::validateHeader()
{
    int bit_7 = (head12.point_format_id >> 7) & 1;
    int bit_6 = (head12.point_format_id >> 6) & 1;

    if (bit_7 == 1 && bit_6 == 1)
        throw error("Header bits indicate unsupported old-style compression.");
    if ((bit_7 ^ bit_6) == 0)
        throw error("Header indicates the file is not compressed.");
    head12.point_format_id &= 0x3f;
}


basic_file::basic_file() : p_(new Private)
{}

basic_file::~basic_file()
{}

bool basic_file::open(std::istream& f)
{
    return p_->open(f);
}

void basic_file::readPoint(char *out)
{
    p_->readPoint(out);
}

const header14& basic_file::header() const
{
    return p_->head14;
}

uint64_t basic_file::pointCount() const
{
    return p_->pointCount();
}

laz_vlr basic_file::lazVlr() const
{
    return p_->laz;
}

std::vector<char> basic_file::vlrData(const std::string& user_id, uint16_t record_id)
{
    return p_->vlrData(user_id, record_id);
}

// reader::mem_file

mem_file::mem_file(char *buf, size_t count) : p_(new Private(buf, count))
{
    if (!open(p_->f))
        throw error("Couldn't open mem_file as LAS/LAZ");
}

mem_file::~mem_file()
{}

// reader::generic_file

generic_file::~generic_file()
{}

generic_file::generic_file(std::istream& in)
{
    if (!open(in))
        throw error("Couldn't open generic_file as LAS/LAZ");
}

// reader::named_file

named_file::named_file(const std::string& filename) : p_(new Private(filename))
{
    if (!open(p_->f))
        throw error("Couldn't open named_file as LAS/LAZ");;
}

named_file::~named_file()
{}

// Chunk decompressor

struct chunk_decompressor::Private
{
    las_decompressor::ptr pdecompressor;
    const unsigned char *buf;

    void getBytes(unsigned char *b, int len)
    {
        while (len--)
            *b++ = *buf++;
    }
};

chunk_decompressor::chunk_decompressor(int format, int ebCount, const char *srcbuf) :
    p_(new Private)
{
    using namespace std::placeholders;

    p_->buf = reinterpret_cast<const unsigned char *>(srcbuf);
    InputCb cb = std::bind(&Private::getBytes, p_.get(), _1, _2);
    p_->pdecompressor = build_las_decompressor(cb, format, ebCount);
}

chunk_decompressor::~chunk_decompressor()
{}

void chunk_decompressor::decompress(char *outbuf)
{
    p_->pdecompressor->decompress(outbuf);
}

} // namespace reader
} // namespace lazperf

