/*
===============================================================================

  FILE:  vlr.hpp

  CONTENTS:
    LAZ vlr

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

#include <vector>

#include "lazperf.hpp"

namespace lazperf
{

struct vlr
{
public:

#pragma pack(push, 1)
    struct vlr_header
    {
        uint16_t reserved;
        char user_id[16];
        uint16_t record_id;
        uint16_t record_length_after_header;
        char description[32];

        size_t size() const;
    };
#pragma pack(pop)

    LAZPERF_EXPORT virtual size_t size() const = 0;
    LAZPERF_EXPORT virtual std::vector<char> data() const = 0;
    LAZPERF_EXPORT virtual vlr_header header() const = 0;
};

struct laz_vlr : public vlr
{
public:
    struct laz_item
    {
        uint16_t type;
        uint16_t size;
        uint16_t version;
    };

    uint16_t compressor;
    uint16_t coder;
    uint8_t ver_major;
    uint8_t ver_minor;
    uint16_t revision;
    uint32_t options;
    uint32_t chunk_size;
    uint64_t num_points;
    uint64_t num_bytes;
    std::vector<laz_item> items;

    LAZPERF_EXPORT laz_vlr();
    LAZPERF_EXPORT laz_vlr(int format, int ebCount, uint32_t chunksize);
    LAZPERF_EXPORT laz_vlr(const char *c);
    LAZPERF_EXPORT ~laz_vlr();

    LAZPERF_EXPORT virtual size_t size() const;
    LAZPERF_EXPORT virtual std::vector<char> data() const;
    LAZPERF_EXPORT virtual vlr_header header() const;
    LAZPERF_EXPORT void fill(const char *c);
};

struct eb_vlr : public vlr
{
public:
    struct ebfield
    {
        uint8_t reserved[2];
        uint8_t data_type;
        uint8_t options;
        char name[32];
        uint8_t unused[4];
        double no_data[3];
        double minval[3];
        double maxval[3];
        double scale[3];
        double offset[3];
        char description[32];

        ebfield();
    };

    std::vector<ebfield> items;

    LAZPERF_EXPORT eb_vlr(size_t bytes);

    LAZPERF_EXPORT virtual size_t size() const;
    LAZPERF_EXPORT virtual std::vector<char> data() const;
    LAZPERF_EXPORT virtual vlr_header header() const;
    LAZPERF_EXPORT void addField();
};

} // namesapce lazperf

