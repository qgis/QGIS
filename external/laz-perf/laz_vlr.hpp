/*
===============================================================================

  FILE:  laz_vlr.hpp

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

#pragma once

#include "vlr.hpp"
#include "utils.hpp"

namespace lazperf
{

#pragma pack(push, 1)

// A Single LAZ Item representation
struct laz_vlr : public vlr
{
    enum
    {
        BYTE = 0,
        POINT10 = 6,
        GPSTIME = 7,
        RGB12 = 8,
        POINT14 = 10,
        RGB14 = 11,
        RGBNIR14 = 12,
        BYTE14 = 14
    };


    struct laz_item
    {
        uint16_t type;
        uint16_t size;
        uint16_t version;

        static const laz_item point()
        {
            return laz_item{ POINT10, 20, 2 };
        }
        static const laz_item gpstime()
        {
            return laz_item{ GPSTIME, 8, 2 };
        }
        static const laz_item rgb()
        {
            return laz_item{ RGB12, 6, 2 };
        }
        static const laz_item eb(uint16_t count)
        {
            return laz_item{ BYTE, count, 2 };
        }
        static const laz_item point14()
        {
            return laz_item{ POINT14, 30, 3 };
        }
        static const laz_item rgb14()
        {
            return laz_item{ RGB14, 6, 3 };
        }
        static const laz_item rgbnir14()
        {
            return laz_item{ RGBNIR14, 8, 3 };
        }
        static const laz_item eb14(uint16_t count)
        {
            return laz_item{ BYTE14, count, 3 };
        }
    };


    uint16_t compressor;
    uint16_t coder;

    uint8_t ver_major;
    uint8_t ver_minor;
    uint16_t revision;

    uint32_t options;
    uint32_t chunk_size;
    int64_t num_points;
    int64_t num_bytes;

    std::vector<laz_item> items;

    laz_vlr();
    laz_vlr(int format, int ebCount, uint32_t chunksize);
    laz_vlr(const char *data);
    ~laz_vlr();

    virtual size_t size() const;
    virtual vlr::vlr_header header();
    void fill(const char *data);
    std::vector<uint8_t> data() const;
};
#pragma pack(pop)

} // namesapce lazperf

