/*
===============================================================================

  FILE:  vlr.cpp

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

#include <string>

#include "utils.hpp"
#include "vlr.hpp"

namespace lazperf
{

size_t vlr::vlr_header::size() const
{ return sizeof(vlr::vlr_header); }

// LAZ VLR

namespace
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

    const laz_vlr::laz_item point_item { POINT10, 20, 2 };
    const laz_vlr::laz_item gps_item { GPSTIME, 8, 2 };
    const laz_vlr::laz_item rgb_item { RGB12, 6, 2 };
    const laz_vlr::laz_item byte_item { BYTE, 0, 2 };
    const laz_vlr::laz_item point14_item { POINT14, 30, 3 };
    const laz_vlr::laz_item rgb14_item{ RGB14, 6, 3 };
    const laz_vlr::laz_item rgbnir14_item{ RGBNIR14, 8, 3 };
    const laz_vlr::laz_item byte14_item { BYTE14, 0, 3 };
}

laz_vlr::laz_vlr()
{}

laz_vlr::~laz_vlr()
{}

laz_vlr::laz_vlr(int format, int ebCount, uint32_t chunksize) :
    compressor(format <= 5 ? 2 : 3), coder(0), ver_major(3), ver_minor(4),
    revision(3), options(0), chunk_size(chunksize), num_points(-1),
    num_bytes(-1)
{
    if (format >= 0 && format <= 5)
    {
        items.push_back(point_item);
        if (format == 1 || format == 3)
            items.push_back(gps_item);
        if (format == 2 || format == 3)
            items.push_back(rgb_item);
        if (ebCount)
        {
            laz_vlr::laz_item item(byte_item);
            item.size = ebCount;
            items.push_back(item);
        }
    }
    else if (format >= 6 && format <= 8)
    {
        items.push_back(point14_item);
        if (format == 7)
            items.push_back(rgb14_item);
        if (format == 8)
            items.push_back(rgbnir14_item);
        if (ebCount)
        {
            laz_vlr::laz_item item(byte14_item);
            item.size = ebCount;
            items.push_back(item);
        }
    }
}

laz_vlr::laz_vlr(const char *data)
{
    fill(data);
}

size_t laz_vlr::size() const
{
    return 34 + (items.size() * 6);
}

vlr::vlr_header laz_vlr::header() const
{
    vlr_header h { 0, "laszip encoded", 22204, (uint16_t)size(), "lazperf variant" };

    return h;
}

void laz_vlr::fill(const char *data)
{
    using namespace utils;

    compressor = unpack<uint16_t>(data);          data += sizeof(compressor);
    coder = unpack<uint16_t>(data);               data += sizeof(coder);
    ver_major = *(const unsigned char *)data++;
    ver_minor = *(const unsigned char *)data++;
    revision = unpack<uint16_t>(data);            data += sizeof(revision);
    options = unpack<uint32_t>(data);             data += sizeof(options);
    chunk_size = unpack<uint32_t>(data);          data += sizeof(chunk_size);
    num_points = unpack<int64_t>(data);           data += sizeof(num_points);
    num_bytes = unpack<int64_t>(data);            data += sizeof(num_bytes);

    uint16_t num_items;
    num_items = unpack<uint16_t>(data);           data += sizeof(num_items);
    items.clear();
    for (int i = 0 ; i < num_items; i ++)
    {
        laz_item item;

        item.type = unpack<uint16_t>(data);       data += sizeof(item.type);
        item.size = unpack<uint16_t>(data);       data += sizeof(item.size);
        item.version = unpack<uint16_t>(data);    data += sizeof(item.version);

        items.push_back(item);
    }
}

std::vector<char> laz_vlr::data() const
{
    using namespace utils;

    std::vector<char> buf(size());
    uint16_t num_items = items.size();

    char *dst = reinterpret_cast<char *>(buf.data());
    pack(compressor, dst);                      dst += sizeof(compressor);
    pack(coder, dst);                           dst += sizeof(coder);
    *dst++ = ver_major;
    *dst++ = ver_minor;
    pack(revision, dst);                        dst += sizeof(revision);
    pack(options, dst);                         dst += sizeof(options);
    pack(chunk_size, dst);                      dst += sizeof(chunk_size);
    pack(num_points, dst);                      dst += sizeof(num_points);
    pack(num_bytes, dst);                       dst += sizeof(num_bytes);
    pack(num_items, dst);                       dst += sizeof(num_items);
    for (size_t k = 0 ; k < items.size() ; k++)
    {
        const laz_item& item = items[k];

        pack(item.type, dst);                   dst += sizeof(item.type);
        pack(item.size, dst);                   dst += sizeof(item.size);
        pack(item.version, dst);                dst += sizeof(item.size);
    }
    return buf;
}

// EB VLR

eb_vlr::ebfield::ebfield() :
    reserved{}, data_type{1}, options{}, name{}, unused{},
    no_data{}, minval{}, maxval{}, scale{}, offset{}, description{}
{}

eb_vlr::eb_vlr(size_t bytes)
{
    for (size_t i = 0; i < bytes; ++i)
        addField();
}

void eb_vlr::addField()
{
    ebfield field;

    std::string name = "FIELD_" + std::to_string(items.size());
    memcpy(field.name, name.data(), 32);

    items.push_back(field);
}

size_t eb_vlr::size() const
{
    return 192 * items.size();
}

// Since all we fill in is a single byte field and a string field, we don't
// need to worry about byte ordering.
std::vector<char> eb_vlr::data() const
{
    const char *start = reinterpret_cast<const char *>(items.data());
    return std::vector<char>(start, start + size());
}

vlr::vlr_header eb_vlr::header() const
{
    return vlr_header { 0, "LASF_Spec", 4, (uint16_t)size(), ""  };
}

} // namespace lazperf
