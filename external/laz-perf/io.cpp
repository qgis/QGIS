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

#include "io.hpp"
#include "io_private.hpp"

namespace lazperf
{

namespace
{

int baseCount(int format)
{
    // Martin screws with the high bits of the format, so we mask down to the low four bits.
    switch (format & 0xF)
    {
    case 0:
        return 20;
    case 1:
        return 28;
    case 2:
        return 26;
    case 3:
        return 34;
    case 6:
        return 30;
    case 7:
        return 36;
    case 8:
        return 38;
    default:
        return 0;
    }
}

} // unnamed namespace

int io::header::ebCount() const
{
    int baseSize = baseCount(point_format_id);
    return (baseSize ? point_record_length - baseSize : 0);
}

namespace reader
{

// reader::basic_file

void basic_file::Private::readPoint(char *out)
{
    if (!compressed)
        stream->cb()(reinterpret_cast<unsigned char *>(out), header.point_record_length);

    // read the next point in
    else
    {
        if (chunk_state.points_read == laz.chunk_size || !pdecompressor)
        {
            pdecompressor = build_las_decompressor(stream->cb(), header.point_format_id,
                header.ebCount());
            // reset chunk state
            chunk_state.current++;
            chunk_state.points_read = 0;
        }

        pdecompressor->decompress(out);
        chunk_state.points_read++;
    }
}

void basic_file::Private::loadHeader()
{
    // Make sure our header is correct
    char magic[4];
    f->read(magic, sizeof(magic));

    if (std::string(magic, magic + 4) != "LASF")
        throw error("Invalid LAS file. Incorrect magic number.");

    // Read the header in
    f->seekg(0);
    f->read((char*)&header, sizeof(header));
    // If we're version 4, back up and do it again.
    if (header.version.minor == 4)
    {
        f->seekg(0);
        f->read((char *)&header14, sizeof(io::header14));
    }
    if (header.point_format_id & 0x80)
        compressed = true;

    // The mins and maxes are in a weird order, fix them
    fixMinMax();

    parseVLRs();

    if (compressed)
    {
        validateHeader();
        parseChunkTable();
    }

    // set the file pointer to the beginning of data to start reading
    // may have treaded past the EOL, so reset everything before we start reading
    f->clear();
    uint64_t offset = header.point_offset;
    if (compressed)
        offset += sizeof(int64_t);
    f->seekg(offset);
    stream->reset();
}

void basic_file::Private::fixMinMax()
{
    double mx, my, mz, nx, ny, nz;

    io::header& h = header;
    mx = h.minimum.x; nx = h.minimum.y;
    my = h.minimum.z; ny = h.maximum.x;
    mz = h.maximum.y; nz = h.maximum.z;

    h.minimum.x = nx; h.maximum.x = mx;
    h.minimum.y = ny; h.maximum.y = my;
    h.minimum.z = nz; h.maximum.z = mz;
}

void basic_file::Private::parseVLRs()
{
    // move the pointer to the begining of the VLRs
    f->seekg(header.header_size);

#pragma pack(push, 1)
    struct {
        unsigned short reserved;
        char user_id[16];
        unsigned short record_id;
        unsigned short record_length;
        char desc[32];
    } vlr_header;
#pragma pack(pop)

    size_t count = 0;
    bool laszipFound = false;
    while (count < header.vlr_count && f->good() && !f->eof())
    {
        f->read((char*)&vlr_header, sizeof(vlr_header));

        const char *user_id = "laszip encoded";

        if (std::equal(vlr_header.user_id, vlr_header.user_id + 14, user_id) &&
            vlr_header.record_id == 22204)
        {
            laszipFound = true;

            std::unique_ptr<char> buffer(new char[vlr_header.record_length]);
            f->read(buffer.get(), vlr_header.record_length);
            parseLASZIPVLR(buffer.get());
            break; // no need to keep iterating
        }
        f->seekg(vlr_header.record_length, std::ios::cur); // jump foward
        count++;
    }

    if (compressed && !laszipFound)
        throw error("Couldn't find LASZIP VLR");
}

void basic_file::Private::parseLASZIPVLR(const char *buf)
{
    laz.fill(buf);
    if (laz.compressor != 2)
        throw error("LASZIP format unsupported - invalid compressor version.");
}

void basic_file::Private::parseChunkTable()
{
    // Move to the begining of the data
    f->seekg(header.point_offset);

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

    // start pushing in chunk table offsets
    chunk_table_offsets.clear();

    if (laz.chunk_size == (std::numeric_limits<unsigned int>::max)())
        throw error("Chunk size too large - unsupported.");

    // Allocate enough room for our chunk
    chunk_table_offsets.resize(chunk_table_header.chunk_count + 1);

    // Add The first one
    chunk_table_offsets[0] = header.point_offset + sizeof(uint64_t);

    //ABELL - Use the standard call for this.
    if (chunk_table_header.chunk_count > 1)
    {
        // decode the index out
        InFileStream fstream(*f);

        InCbStream stream(fstream.cb());
        decoders::arithmetic<InCbStream> decoder(stream);
        decompressors::integer decomp(32, 2);

        // start decoder
        decoder.readInitBytes();
        decomp.init();

        for (size_t i = 1 ; i <= chunk_table_header.chunk_count; i++)
        {
            uint32_t offset = i > 1 ? (int32_t)chunk_table_offsets[i - 1] : 0;
            chunk_table_offsets[i] = static_cast<uint64_t>(
                decomp.decompress(decoder, offset, 1));
        }

        for (size_t i = 1 ; i < chunk_table_offsets.size() ; i ++)
            chunk_table_offsets[i] += chunk_table_offsets[i - 1];
    }
}

void basic_file::Private::validateHeader()
{
    io::header& h = header;

    int bit_7 = (h.point_format_id >> 7) & 1;
    int bit_6 = (h.point_format_id >> 6) & 1;

    if (bit_7 == 1 && bit_6 == 1)
        throw error("Header bits indicate unsupported old-style compression.");
    if ((bit_7 ^ bit_6) == 0)
        throw error("Header indicates the file is not compressed.");
    h.point_format_id &= 0x3f;
}


basic_file::basic_file() : p_(new Private)
{}

basic_file::~basic_file()
{}

void basic_file::open(std::istream& f)
{
    p_->f = &f;
    //ABELL - move to loadHeader() in order to avoid the reset on InFileStream.
    p_->stream.reset(new InFileStream(f));
    p_->loadHeader();
}

void basic_file::readPoint(char *out)
{
    p_->readPoint(out);
}

const io::header& basic_file::header() const
{
    return p_->header;
}

size_t basic_file::pointCount() const
{
    if (p_->header.version.major > 1 || p_->header.version.minor > 3)
        return p_->header14.point_count_14;
    return p_->header.point_count;
}

// reader::mem_file

mem_file::mem_file(char *buf, size_t count) : p_(new Private(buf, count))
{
    open(p_->f);
}

mem_file::~mem_file()
{}

// reader::generic_file

generic_file::~generic_file()
{}

generic_file::generic_file(std::istream& in)
{
    open(in);
}

// reader::named_file

named_file::named_file(const std::string& filename) : p_(new Private(filename))
{
    open(p_->f);
}

named_file::~named_file()
{}

} // namespace reader

// WRITER

namespace writer
{

void basic_file::Private::open(std::ostream& out, const io::header& h, uint32_t cs)
{
    header = h;
    chunk_size = cs;
    f = &out;

    size_t preludeSize = h.version.minor == 4 ? sizeof(io::header14) : sizeof(io::header);
    if (compressed())
    {
        preludeSize += sizeof(int64_t);  // Chunk table offset.
        laz_vlr vlr(h.point_format_id, h.ebCount(), chunk_size);
        preludeSize += vlr.size() + vlr.header().size();
    }
    if (h.ebCount())
    {
        eb_vlr vlr(h.ebCount());
        preludeSize += vlr.size() + vlr.header().size();
    }

    std::vector<char> junk(preludeSize);
    f->write(junk.data(), preludeSize);
    // the first chunk begins at the end of prelude
    stream.reset(new OutFileStream(out));
}

bool basic_file::Private::compressed() const
{
    return chunk_size > 0;
}

void basic_file::Private::updateMinMax(const las::point10& p)
{
    double x = p.x * header.scale.x + header.offset.x;
    double y = p.y * header.scale.y + header.offset.y;
    double z = p.z * header.scale.z + header.offset.z;

    header.minimum.x = (std::min)(x, header.minimum.x);
    header.minimum.y = (std::min)(y, header.minimum.y);
    header.minimum.z = (std::min)(z, header.minimum.z);

    header.maximum.x = (std::max)(x, header.maximum.x);
    header.maximum.y = (std::max)(y, header.maximum.y);
    header.maximum.z = (std::max)(z, header.maximum.z);
}

void basic_file::Private::writePoint(const char *p)
{
    if (!compressed())
        stream->putBytes(reinterpret_cast<const unsigned char *>(p), header.point_record_length);
    else
    {
        //ABELL - This first bit can go away if we simply always create compressor.
        if (!pcompressor)
        {
            pcompressor = build_las_compressor(stream->cb(), header.point_format_id,
                header.ebCount());
        }
        else if (chunk_state.points_in_chunk == chunk_size)
        {
            pcompressor->done();
            chunk_state.points_in_chunk = 0;
            std::streamsize offset = f->tellp();
            chunk_sizes.push_back(offset - chunk_state.last_chunk_write_offset);
            chunk_state.last_chunk_write_offset = offset;
            pcompressor = build_las_compressor(stream->cb(), header.point_format_id,
                header.ebCount());
        }

        // now write the point
        pcompressor->compress(p);
    }
    chunk_state.total_written++;
    chunk_state.points_in_chunk++;
    updateMinMax(*(reinterpret_cast<const las::point10*>(p)));
}

void basic_file::Private::close()
{
    if (compressed())
    {
        pcompressor->done();

        // Note down the size of the offset of this last chunk
        chunk_sizes.push_back((std::streamsize)f->tellp() - chunk_state.last_chunk_write_offset);
    }

    writeHeader();
    if (compressed())
        writeChunkTable();
}

void basic_file::Private::writeHeader()
{
    laz_vlr lazVlr(header.point_format_id, header.ebCount(), chunk_size);
    eb_vlr ebVlr(header.ebCount());

    // point_format_id and point_record_length  are set on open().
    header.header_size = (header.version.minor == 4 ? sizeof(io::header14) : sizeof(io::header));
    header.point_offset = header.header_size;
    header.vlr_count = 0;
    if (compressed())
    {
       header.point_offset += lazVlr.size() + lazVlr.header().size();
       header.vlr_count++;
       header.point_format_id |= (1 << 7);
    }
    if (header.ebCount())
    {
        header.point_offset += ebVlr.size() + ebVlr.header().size();
        header.vlr_count++;
    }

    header.point_count = static_cast<unsigned int>(chunk_state.total_written);

    //HUH?
    // make sure we re-arrange mins and maxs for writing
    double mx, my, mz, nx, ny, nz;
    nx = header.minimum.x; mx = header.maximum.x;
    ny = header.minimum.y; my = header.maximum.y;
    nz = header.minimum.z; mz = header.maximum.z;

    header.minimum.x = mx; header.minimum.y = nx;
    header.minimum.z = my; header.maximum.x = ny;
    header.maximum.y = mz; header.maximum.z = nz;

    if (header.version.minor == 4)
    {
        header14.point_count_14 = header.point_count;
        // Set the WKT bit.
        header.global_encoding |= (1 << 4);
    }

    f->seekp(0);
    f->write(reinterpret_cast<char*>(&header), header.header_size);

    if (compressed())
    {
        // Write the VLR.
        vlr::vlr_header h = lazVlr.header();
        f->write(reinterpret_cast<char *>(&h), sizeof(h));

        std::vector<char> vlrbuf = lazVlr.data();
        f->write(vlrbuf.data(), vlrbuf.size());
    }
    if (header.ebCount())
    {
        vlr::vlr_header h = ebVlr.header();
        f->write(reinterpret_cast<char *>(&h), sizeof(h));

        std::vector<char> vlrbuf = ebVlr.data();
        f->write(vlrbuf.data(), vlrbuf.size());
    }
}

void basic_file::Private::writeChunkTable()
{
    // move to the end of the file to start emitting our compresed table
    f->seekp(0, std::ios::end);

    // take note of where we're writing the chunk table, we need this later
    int64_t chunk_table_offset = static_cast<int64_t>(f->tellp());

    // write out the chunk table header (version and total chunks)
#pragma pack(push, 1)
    struct
    {
        unsigned int version,
        chunks_count;
    } chunk_table_header = { 0, static_cast<unsigned int>(chunk_sizes.size()) };
#pragma pack(pop)

    f->write(reinterpret_cast<char*>(&chunk_table_header), sizeof(chunk_table_header));

    // Now compress and write the chunk table
    OutFileStream w(*f);
    OutCbStream outStream(w.cb());

    encoders::arithmetic<OutCbStream> encoder(outStream);
    compressors::integer comp(32, 2);

    comp.init();

    for (size_t i = 0 ; i < chunk_sizes.size() ; i ++)
    {
        comp.compress(encoder, i ? static_cast<int>(chunk_sizes[i - 1]) : 0,
            static_cast<int>(chunk_sizes[i]), 1);
    }
    encoder.done();

    // go back to where we're supposed to write chunk table offset
    f->seekp(header.point_offset);
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

void basic_file::open(std::ostream& out, const io::header& h, uint32_t chunk_size)
{
    p_->open(out, h, chunk_size);
}

void basic_file::writePoint(const char *buf)
{
    p_->writePoint(buf);
}

void basic_file::close()
{
    p_->close();
}

// named_file

named_file::config::config() : scale(1.0, 1.0, 1.0), offset(0.0, 0.0, 0.0),
    chunk_size(io::DefaultChunkSize), pdrf(0), minor_version(3), extra_bytes(0)
{}

named_file::config::config(const io::vector3& s, const io::vector3& o, unsigned int cs) :
    scale(s), offset(o), chunk_size(cs), pdrf(0), minor_version(3), extra_bytes(0)
{}

named_file::config::config(const io::header& h) : scale(h.scale.x, h.scale.y, h.scale.z),
    offset(h.offset.x, h.offset.y, h.offset.z), chunk_size(io::DefaultChunkSize),
    pdrf(h.point_format_id), extra_bytes(h.ebCount())
{}

io::header named_file::config::to_header() const
{
    io::header h;

    h.minimum = { (std::numeric_limits<double>::max)(), (std::numeric_limits<double>::max)(),
        (std::numeric_limits<double>::max)() };
    h.maximum = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest()};
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
    io::header h = c.to_header();

    // open the file and move to offset of data, we'll write
    // headers and all other things on file close
    f.open(filename, std::ios::binary | std::ios::trunc);
    if (!f.good())
        throw error("Couldn't open '" + filename + "' for writing.");
    base->open(f, h, c.chunk_size);
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
    if (p_->f.is_open())
        p_->f.close();
}

} // namespace writer
} // namespace lazperf

