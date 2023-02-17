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

  CHANGE HISTORY:

===============================================================================
*/

#include <iostream>

#include "header.hpp"
#include "Extractor.hpp"
#include "Inserter.hpp"

namespace lazperf
{

const size_t header12::Size = 227;
const size_t header13::Size = 235;
const size_t header14::Size = 375;

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

base_header::base_header()
{}

int base_header::ebCount() const
{
    int baseSize = baseCount(point_format_id);
    return (baseSize ? point_record_length - baseSize : 0);
}

// Get the real point format - strip off any silly LAZ encoding bits
int base_header::pointFormat() const
{
    return point_format_id & 0x3F;
}

// Check if the compression bit is set.
bool base_header::compressed() const
{
    return point_format_id & 0x80;
}

size_t base_header::sizeFromVersion() const
{
    size_t size = 0;
    if (version.minor == 2)
        size = header12::Size;
    else if (version.minor == 3)
        size = header13::Size;
    else if (version.minor == 4)
        size = header14::Size;
    return size;
}

int base_header::minorVersion(std::istream& in)
{
    auto pos = in.tellg();
    in.seekg(25);
    int8_t version;
    in >> version;
    in.seekg(pos);
    return (in.good() ? version : 0);
}

///

header12 header12::create(std::istream& in)
{
    header12 h;
    h.read(in);
    return h;
}

void header12::read(std::istream& in)
{
    std::vector<char> buf(header12::Size);
    in.read(buf.data(), buf.size());
    LeExtractor s(buf.data(), buf.size());

    s.get(magic, 4);
    s >> file_source_id >> global_encoding;
    s.get(guid, 16);
    s >> version.major >> version.minor;
    s.get(system_identifier, 32);
    s.get(generating_software, 32);
    s >> creation.day >> creation.year;
    s >> header_size >> point_offset >> vlr_count;
    s >> point_format_id >> point_record_length;
    s >> point_count;
    for (int i = 0; i < 5; ++i)
        s >> points_by_return[i];
    s >> scale.x >> scale.y >> scale.z;
    s >> offset.x >> offset.y >> offset.z;
    s >> maxx >> minx >> maxy >> miny >> maxz >> minz;
}

void header12::write(std::ostream& out) const
{
    std::vector<char> buf(header12::Size);
    LeInserter s(buf.data(), buf.size());

    s.put(magic, 4);
    s << file_source_id << global_encoding;
    s.put(guid, 16);
    s << version.major << version.minor;
    s.put(system_identifier, 32);
    s.put(generating_software, 32);
    s << creation.day << creation.year;
    s << header_size << point_offset << vlr_count;
    s << point_format_id << point_record_length;
    s << point_count;
    for (int i = 0; i < 5; ++i)
        s << points_by_return[i];
    s << scale.x << scale.y << scale.z;
    s << offset.x << offset.y << offset.z;
    s << maxx << minx << maxy << miny << maxz << minz;
    out.write(buf.data(), buf.size());
}

///

header13 header13::create(std::istream& in)
{
    header13 h;
    h.read(in);
    return h;
}

void header13::read(std::istream& in)
{
    ((header12 *)this)->read(in);

    std::vector<char> buf(header13::Size - header12::Size);
    in.read(buf.data(), buf.size());
    LeExtractor s(buf.data(), buf.size());

    s >> wave_offset;
}

void header13::write(std::ostream& out) const
{
    ((const header12 *)this)->write(out);

    std::vector<char> buf(header13::Size - header12::Size);
    LeInserter s(buf.data(), buf.size());

    s << wave_offset;

    out.write(buf.data(), buf.size());
}

///

header14 header14::create(std::istream& in)
{
    header14 h;
    h.read(in);
    return h;
}

void header14::read(std::istream& in)
{
    ((header13 *)this)->read(in);

    std::vector<char> buf(header14::Size - header13::Size);
    in.read(buf.data(), buf.size());
    LeExtractor s(buf.data(), buf.size());

    s >> evlr_offset;
    s >> evlr_count;
    s >> point_count_14;
    for (int i = 0; i < 15; ++i)
        s >> points_by_return_14[i];
}

void header14::write(std::ostream& out) const
{
    ((const header13 *)this)->write(out);

    std::vector<char> buf(header14::Size - header13::Size);
    LeInserter s(buf.data(), buf.size());

    s << evlr_offset;
    s << evlr_count;
    s << point_count_14;
    for (int i = 0; i < 15; ++i)
        s << points_by_return_14[i];

    out.write(buf.data(), buf.size());
}

} // namespace lazperf

