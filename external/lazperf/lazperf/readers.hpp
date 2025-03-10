/*
===============================================================================

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

===============================================================================
*/

#pragma once

#include "header.hpp"
#include "vlr.hpp"

namespace lazperf
{
namespace reader
{

class basic_file
{
    FRIEND_TEST(io_tests, parses_laszip_vlr_correctly);
    struct Private;

protected:
    basic_file();
    ~basic_file();

    bool open(std::istream& in);

public:
    LAZPERF_EXPORT uint64_t pointCount() const;
    LAZPERF_EXPORT const header14& header() const;
    LAZPERF_EXPORT void readPoint(char *out);
    LAZPERF_EXPORT laz_vlr lazVlr() const;
    LAZPERF_EXPORT std::vector<char> vlrData(const std::string& user_id, uint16_t record_id);

private:
    // The file object is not copyable or copy constructible
    basic_file(const basic_file&) = delete;
    basic_file& operator = (const basic_file&) = delete;

    std::unique_ptr<Private> p_;
};

class mem_file : public basic_file
{
    struct Private;

public:
    LAZPERF_EXPORT mem_file(char *buf, size_t count);
    LAZPERF_EXPORT ~mem_file();

private:
    std::unique_ptr<Private> p_;
};

class generic_file : public basic_file
{
public:
    LAZPERF_EXPORT ~generic_file();
    LAZPERF_EXPORT generic_file(std::istream& in);
};

class named_file : public basic_file
{
    struct Private;

public:
    LAZPERF_EXPORT named_file(const std::string& filename);
    LAZPERF_EXPORT ~named_file();

private:
    std::unique_ptr<Private> p_;
};

///

class chunk_decompressor
{
    struct Private;
public:
    LAZPERF_EXPORT chunk_decompressor(int format, int ebCount, const char *srcbuf);
    LAZPERF_EXPORT ~chunk_decompressor();
    LAZPERF_EXPORT void decompress(char *outbuf);

private:
    std::unique_ptr<Private> p_;
};

} // namespace reader
} // namespace lazperf

