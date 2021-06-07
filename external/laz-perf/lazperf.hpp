/*
===============================================================================

  FILE:  las.hpp

  CONTENTS:
    Point formats for LAS

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

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "lazperf_base.hpp"

namespace lazperf
{

// Called when compressed output is to be written.
using OutputCb = std::function<void(const unsigned char *, size_t)>;

// Called when compressed input is to be read.
using InputCb = std::function<void(unsigned char *, size_t)>;

class las_compressor
{
public:
    typedef std::shared_ptr<las_compressor> ptr;

    LAZPERF_EXPORT virtual const char *compress(const char *in) = 0;
    LAZPERF_EXPORT virtual void done() = 0;
    LAZPERF_EXPORT virtual ~las_compressor();
};

class las_decompressor
{
public:
    typedef std::shared_ptr<las_decompressor> ptr;

    LAZPERF_EXPORT virtual char *decompress(char *in) = 0;
    LAZPERF_EXPORT virtual ~las_decompressor();
};

class point_compressor_base_1_2 : public las_compressor
{
    struct Private;

public:
    LAZPERF_EXPORT void done();

protected:
    point_compressor_base_1_2(OutputCb cb, size_t ebCount);
    virtual ~point_compressor_base_1_2();

    std::unique_ptr<Private> p_;
};

class point_compressor_0 : public point_compressor_base_1_2
{
public:
    LAZPERF_EXPORT point_compressor_0(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_0();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
};

class point_compressor_1 : public point_compressor_base_1_2
{
public:
    LAZPERF_EXPORT point_compressor_1(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_1();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
};

class point_compressor_2 : public point_compressor_base_1_2
{
public:
    LAZPERF_EXPORT point_compressor_2(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_2();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
};

class point_compressor_3 : public point_compressor_base_1_2
{
public:
    LAZPERF_EXPORT point_compressor_3(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_3();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
};

class point_compressor_base_1_4 : public las_compressor
{
    struct Private;

public:
    virtual const char *compress(const char *in) = 0;

protected:
    point_compressor_base_1_4(OutputCb cb, size_t ebCount);

    std::unique_ptr<Private> p_;
};


class point_compressor_6 : public point_compressor_base_1_4
{
public:
    LAZPERF_EXPORT point_compressor_6(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_6();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
    LAZPERF_EXPORT virtual void done();
};

class point_compressor_7 : public point_compressor_base_1_4
{
public:
    LAZPERF_EXPORT point_compressor_7(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_7();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
    LAZPERF_EXPORT virtual void done();
};

class point_compressor_8 : public point_compressor_base_1_4
{
public:
    LAZPERF_EXPORT point_compressor_8(OutputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_compressor_8();

    LAZPERF_EXPORT virtual const char *compress(const char *in);
    LAZPERF_EXPORT virtual void done();
};


// Decompressor

class point_decompressor_base_1_2 : public las_decompressor
{
    struct Private;

public:
    virtual char *decompress(char *in) = 0;
    virtual ~point_decompressor_base_1_2();

protected:
    point_decompressor_base_1_2(InputCb cb, size_t ebCount);
    void handleFirst();

    std::unique_ptr<Private> p_;
};

class point_decompressor_0 : public point_decompressor_base_1_2
{
public:
    LAZPERF_EXPORT point_decompressor_0(InputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_decompressor_0();

    LAZPERF_EXPORT virtual char *decompress(char *in);
};

class point_decompressor_1 : public point_decompressor_base_1_2
{
public:
    LAZPERF_EXPORT point_decompressor_1(InputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_decompressor_1();

    LAZPERF_EXPORT virtual char *decompress(char *out);
};

class point_decompressor_2 : public point_decompressor_base_1_2
{
public:
    LAZPERF_EXPORT point_decompressor_2(InputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_decompressor_2();

    LAZPERF_EXPORT virtual char *decompress(char *out);
};

class point_decompressor_3 : public point_decompressor_base_1_2
{
public:
    LAZPERF_EXPORT point_decompressor_3(InputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_decompressor_3();

    LAZPERF_EXPORT virtual char *decompress(char *out);
};

class point_decompressor_base_1_4 : public las_decompressor
{
    struct Private;

public:
    virtual char *decompress(char *out) = 0;

protected:
    point_decompressor_base_1_4(InputCb cb, size_t ebCount);

    std::unique_ptr<Private> p_;
};

class point_decompressor_6 : public point_decompressor_base_1_4
{
public:
    LAZPERF_EXPORT point_decompressor_6(InputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_decompressor_6();

    LAZPERF_EXPORT virtual char *decompress(char *out);
};

class point_decompressor_7 : public point_decompressor_base_1_4
{
public:
    LAZPERF_EXPORT point_decompressor_7(InputCb cb, size_t ebCount = 0);
    LAZPERF_EXPORT ~point_decompressor_7();

    LAZPERF_EXPORT virtual char *decompress(char *out);
};

struct point_decompressor_8 : public point_decompressor_base_1_4
{
public:
    LAZPERF_EXPORT ~point_decompressor_8();
    LAZPERF_EXPORT point_decompressor_8(InputCb cb, size_t ebCount = 0);

    LAZPERF_EXPORT virtual char *decompress(char *out);
};

// FACTORY

LAZPERF_EXPORT las_compressor::ptr build_las_compressor(OutputCb, int format,
    size_t ebCount = 0);
LAZPERF_EXPORT las_decompressor::ptr build_las_decompressor(InputCb, int format,
    size_t ebCount = 0);

// CHUNK TABLE

// Note that the chunk values are sizes, rather than offsets.
LAZPERF_EXPORT void compress_chunk_table(OutputCb cb, const std::vector<uint32_t>& chunks);
LAZPERF_EXPORT std::vector<uint32_t> decompress_chunk_table(InputCb cb, size_t numChunks);

} // namespace lazperf

