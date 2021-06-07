/*
  COPYRIGHT:

    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/

#pragma once

// NOTE: This file exists to facilitate testing of private code.

#include "io.hpp"
#include "las.hpp"
#include "charbuf.hpp"
#include "streams.hpp"
#include "vlr.hpp"

namespace lazperf
{
namespace reader
{

struct basic_file::Private
{
    Private() : header(header14), compressed(false)
    {}

    void readPoint(char *out);
    void loadHeader();
    void fixMinMax();
    void parseVLRs();
    void parseLASZIPVLR(const char *);
    void parseChunkTable();
    void validateHeader();

    struct ChunkState
    {
        int64_t current;
        int64_t points_read;
        int64_t current_index;

        ChunkState() : current(0), points_read(0), current_index(-1)
        {}
    } chunk_state;
    std::istream *f;
    std::unique_ptr<InFileStream> stream;
    io::header& header;
    io::header14 header14;
    laz_vlr laz;
    std::vector<uint64_t> chunk_table_offsets;
    bool compressed;
    las_decompressor::ptr pdecompressor;
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

} // namespace reader

namespace writer
{

struct basic_file::Private
{
    Private() : header(header14), chunk_size(io::DefaultChunkSize), f(nullptr)
    {}

    void close();
    bool compressed() const;
    void open(std::ostream& out, const io::header& h, uint32_t chunk_size);
    void writePoint(const char *p);
    void updateMinMax(const las::point10& p);
    void writeHeader();
    void writeChunks();
    void writeChunkTable();

    struct ChunkState
    {
        int64_t total_written; // total points written
        int64_t current_chunk_index; //  the current chunk index we're compressing
        unsigned int points_in_chunk;
        std::streamsize last_chunk_write_offset;

        ChunkState() : total_written(0), current_chunk_index(-1),
            points_in_chunk(0), last_chunk_write_offset(0)
        {}
    } chunk_state;
    las_compressor::ptr pcompressor;
    io::header& header;
    io::header14 header14;
    unsigned int chunk_size;
    std::ostream *f;
    std::unique_ptr<OutFileStream> stream;
    std::vector<int64_t> chunk_sizes; // all the places where chunks begin
};

struct named_file::Private
{
    using Base = basic_file::Private;

    Private(Base *b) : base(b)
    {}
    
    void open(const std::string& filename, const named_file::config& c);

    Base *base;
    std::ofstream f;
};

} // namespace writer
} // namespace lazperf

