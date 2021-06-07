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

#include <stdint.h>

#include "decoder.hpp"
#include "encoder.hpp"
#include "model.hpp"
#include "compressor.hpp"
#include "streams.hpp"
#include "utils.hpp"

namespace lazperf
{
namespace las
{

#pragma pack(push, 1)
struct point10
{
    int x;
    int y;
    int z;
    unsigned short intensity;
    unsigned char return_number : 3;
    unsigned char number_of_returns_of_given_pulse : 3;
    unsigned char scan_direction_flag : 1;
    unsigned char edge_of_flight_line : 1;
    unsigned char classification;
    char scan_angle_rank;
    unsigned char user_data;
    unsigned short point_source_ID;

    point10() : x(0), y(0), intensity(0), return_number(0),
        number_of_returns_of_given_pulse(0), scan_direction_flag(0),
        edge_of_flight_line(0), classification(0),
        scan_angle_rank(0), user_data(0), point_source_ID(0)
    {}

    point10(const char *buf)
    {
        unpack(buf);
    }

    void unpack(const char *c)
    {
        x = utils::unpack<int32_t>(c);                     c += sizeof(int32_t);
        y = utils::unpack<int32_t>(c);                     c += sizeof(int32_t);
        z = utils::unpack<int32_t>(c);                     c += sizeof(int32_t);
        intensity = utils::unpack<uint16_t>(c);            c += sizeof(unsigned short);
        to_bitfields(*c++);
        classification = *c++;
        scan_angle_rank = *c++;
        user_data = *c++;
        point_source_ID = utils::unpack<uint16_t>(c);
    }

    void pack(char *c)
    {
        utils::pack(x, c);                 c += sizeof(int);
        utils::pack(y, c);                 c += sizeof(int);
        utils::pack(z, c);                 c += sizeof(int);
        utils::pack(intensity, c);         c += sizeof(unsigned short);
        *c++ = from_bitfields();
        *c++ = classification;
        *c++ = scan_angle_rank;
        *c++ = user_data;
        utils::pack(point_source_ID, c);
    }

    void to_bitfields(unsigned char d)
    {
        return_number = d & 0x7;
        number_of_returns_of_given_pulse = (d >> 3) & 0x7;
        scan_direction_flag = (d >> 6) & 0x1;
        edge_of_flight_line = (d >> 7) & 0x1;
    }

    unsigned char from_bitfields() const
    {
        return ((edge_of_flight_line & 0x1) << 7) |
               ((scan_direction_flag & 0x1) << 6) |
               ((number_of_returns_of_given_pulse & 0x7) << 3) |
               (return_number & 0x7);
    }
};

struct gpstime
{
public:
    gpstime() : value(0)
    {}
    gpstime(int64_t v) : value(v)
    {}
    gpstime(const char *c)
    {
        unpack(c);
    }

    void unpack(const char *in)
    {
        uint64_t lower = utils::unpack<uint32_t>(in);
        uint64_t upper = utils::unpack<uint32_t>(in + 4);

        value = ((upper << 32) | lower);
    }

    void pack(char *buffer)
    {
        utils::pack(uint32_t(value & 0xFFFFFFFF), buffer);
        utils::pack(uint32_t(value >> 32), buffer + 4);
    }

    // Note that in a LAS file, gps time is a double, not int64_t, but since we always
    // treat it as int here, we just unpack to that form.
    int64_t value;
};

struct rgb
{
public:
    rgb() : r(0), g(0), b(0)
    {}
    rgb(unsigned short r, unsigned short g, unsigned short b) : r(r), g(g), b(b)
    {}
    rgb(const char *buf)
    {
        unpack(buf);
    }

    void unpack(const char *c)
    {
        r = utils::unpack<uint16_t>(c);
        g = utils::unpack<uint16_t>(c + 2);
        b = utils::unpack<uint16_t>(c + 4);
    }

    void pack(char *c)
    {
        utils::pack(r, c);
        utils::pack(g, c + 2);
        utils::pack(b, c + 4);
    }

    uint16_t r;
    uint16_t g;
    uint16_t b;
};

struct rgb14 : public rgb
{
    rgb14()
    {}
    rgb14(const rgb& val) : rgb(val)
    {}
};

struct nir14
{
    uint16_t val;

    nir14() : val(0)
    {}

    nir14(uint16_t v) : val(v)
    {}

    nir14(const char *p)
    {
        unpack(p);
    }

    void pack(char *p)
    {
        utils::pack(val, p);
    }

    void unpack(const char *p)
    {
        val = utils::unpack<uint16_t>(p);
    }
};

using byte14 = std::vector<uint8_t>;


struct extrabytes : public std::vector<uint8_t>
{};

struct point14
{
    int32_t x_;
    int32_t y_;
    int32_t z_;
    uint16_t intensity_;
    uint8_t returns_;
    uint8_t flags_;
    uint8_t classification_;
    uint8_t user_data_;
    int16_t scan_angle_;
    uint16_t point_source_ID_;
    double gpstime_;

    point14()
    {}

    point14(const char *c)
    {
        unpack(c);
    }

    int32_t x() const
    { return x_; }
    void setX(int32_t x)
    { x_ = x; }

    int32_t y() const
    { return y_; }
    void setY(int32_t y)
    { y_ = y; }

    int32_t z() const
    { return z_; }
    void setZ(int32_t z)
    { z_ = z; }

    uint16_t intensity() const
    { return intensity_; }
    void setIntensity(uint16_t intensity)
    { intensity_ = intensity; }

    uint8_t returns() const
    { return returns_; }
    void setReturns(uint8_t returns)
    { returns_ = returns; }

    int returnNum() const
    { return returns_ & 0xF; }
    void setReturnNum(int rn)
    { returns_ = rn | (returns_ & 0xF0); }

    uint8_t numReturns() const
    { return returns_ >> 4; }
    void setNumReturns(int nr)
    { returns_ = (nr << 4) | (returns_ & 0xF); }

    uint8_t flags() const
    { return flags_; }
    void setFlags(uint8_t flags)
    { flags_ = flags; }

    int classFlags() const
    { return (flags_ & 0xF); }
    void setClassFlags(int flags)
    { flags_ = flags | (flags_ & 0xF0); }

    int scannerChannel() const
    { return (flags_ >> 4) & 0x03; }
    void setScannerChannel(int c)
    { flags_ = (c << 4) | (flags_ & ~0x30); }

    int scanDirFlag() const
    { return ((flags_ >> 6) & 1); }
    void setScanDirFlag(int flag)
    { flags_ = (flag << 6) | (flags_ & 0xBF); }

    int eofFlag() const
    { return ((flags_ >> 7) & 1); }
    void setEofFlag(int flag)
    { flags_ = (flag << 7) | (flags_ & 0x7F); }

    uint8_t classification() const
    { return classification_; }
    void setClassification(uint8_t classification)
    { classification_ = classification; }

    uint8_t userData() const
    { return user_data_; }
    void setUserData(uint8_t user_data)
    { user_data_ = user_data; }

    int16_t scanAngle() const
    { return scan_angle_; }
    void setScanAngle(int16_t scan_angle)
    { scan_angle_ = scan_angle; }

    uint16_t pointSourceID() const
    { return point_source_ID_; }
    void setPointSourceID(uint16_t point_source_ID)
    { point_source_ID_ = point_source_ID; }

    double gpsTime() const
    { return gpstime_; }
    void setGpsTime(double gpstime)
    { gpstime_ = gpstime; }

    void unpack(const char *in)
    {
        setX(utils::unpack<int32_t>(in));               in += sizeof(int32_t);
        setY(utils::unpack<int32_t>(in));               in += sizeof(int32_t);
        setZ(utils::unpack<int32_t>(in));               in += sizeof(int32_t);
        setIntensity(utils::unpack<uint16_t>(in));      in += sizeof(uint16_t);
        setReturns(*in++);
        setFlags(*in++);
        setClassification(*in++);
        setUserData(*in++);
        setScanAngle(utils::unpack<int16_t>(in));       in += sizeof(int16_t);
        setPointSourceID(utils::unpack<uint16_t>(in));  in += sizeof(uint16_t);
        setGpsTime(utils::unpack<double>(in));
    }
};
#pragma pack(pop)
} // namespace las
} // namespace lazperf

#include "detail/field_byte10.hpp"
#include "detail/field_point10.hpp"
#include "detail/field_point14.hpp"
#include "detail/field_gpstime10.hpp"
#include "detail/field_rgb10.hpp"
#include "detail/field_rgb14.hpp"
#include "detail/field_nir14.hpp"
#include "detail/field_byte14.hpp"

