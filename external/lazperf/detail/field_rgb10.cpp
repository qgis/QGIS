/*
===============================================================================

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

#include "../las.hpp"
//#include "../types.hpp"

namespace lazperf
{
namespace detail
{

namespace
{

unsigned int color_diff_bits(const las::rgb& this_val, const las::rgb& last)
{
    const las::rgb& a = last;
    const las::rgb& b = this_val;

#define __flag_diff(x,y,f) ((((x) ^ (y)) & (f)) != 0)
                unsigned int r =
                    (__flag_diff(a.r, b.r, 0x00FF) << 0) |
                    (__flag_diff(a.r, b.r, 0xFF00) << 1) |
                    (__flag_diff(a.g, b.g, 0x00FF) << 2) |
                    (__flag_diff(a.g, b.g, 0xFF00) << 3) |
                    (__flag_diff(a.b, b.b, 0x00FF) << 4) |
                    (__flag_diff(a.b, b.b, 0xFF00) << 5) |
                    (__flag_diff(b.r, b.g, 0x00FF) |
                     __flag_diff(b.r, b.b, 0x00FF) |
                     __flag_diff(b.r, b.g, 0xFF00) |
                     __flag_diff(b.r, b.b, 0xFF00)) << 6;
#undef __flag_diff

    return r;
}

} // unnamed namespace

Rgb10Base::Rgb10Base() : have_last_(false), last(), m_byte_used(128), m_rgb_diff_0(256),
    m_rgb_diff_1(256), m_rgb_diff_2(256), m_rgb_diff_3(256), m_rgb_diff_4(256),
    m_rgb_diff_5(256)
{}

// COMPRESSOR

Rgb10Compressor::Rgb10Compressor(encoders::arithmetic<OutCbStream>& encoder) : enc_(encoder)
{}

const char *Rgb10Compressor::compress(const char *buf)
{
    las::rgb this_val(buf);

    if (!have_last_) {
        // don't have the first data yet, just push it to our
        // have last stuff and move on
        have_last_ = true;
        last = this_val;

        enc_.getOutStream().putBytes((const unsigned char*)buf, sizeof(las::rgb));
        return buf + sizeof(las::rgb);
    }

    // compress color
    int diff_l = 0;
    int diff_h = 0;
    int corr;

    unsigned int sym = detail::color_diff_bits(this_val, last);

    enc_.encodeSymbol(m_byte_used, sym);

    // high and low R
    if (sym & (1 << 0))
    {
        diff_l = (this_val.r & 0xFF) - (last.r & 0xFF);
        enc_.encodeSymbol(m_rgb_diff_0, uint8_t(diff_l));
    }
    if (sym & (1 << 1))
    {
        diff_h = static_cast<int>(this_val.r >> 8) - (last.r >> 8);
        enc_.encodeSymbol(m_rgb_diff_1, uint8_t(diff_h));
    }

    if (sym & (1 << 6))
    {
        if (sym & (1 << 2))
        {
            corr = static_cast<int>(this_val.g & 0xFF) -
                utils::clamp<uint8_t>(diff_l + (last.g & 0xFF));
            enc_.encodeSymbol(m_rgb_diff_2, uint8_t(corr));
        }

        if (sym & (1 << 4))
        {
            diff_l = (diff_l + (this_val.g & 0xFF) - (last.g & 0xFF)) / 2;
            corr = static_cast<int>(this_val.b & 0xFF) -
                utils::clamp<uint8_t>(diff_l + (last.b & 0xFF));
            enc_.encodeSymbol(m_rgb_diff_4, uint8_t(corr));
        }

        if (sym & (1 << 3))
        {
            corr = static_cast<int>(this_val.g >> 8) -
                utils::clamp<uint8_t>(diff_h + (last.g >> 8));
            enc_.encodeSymbol(m_rgb_diff_3, uint8_t(corr));
        }

        if (sym & (1 << 5))
        {
            diff_h = (diff_h + ((this_val.g >> 8)) - (last.g >> 8)) / 2;
            corr = static_cast<int>(this_val.b >> 8) -
                utils::clamp<uint8_t>(diff_h + (last.b >> 8));
            enc_.encodeSymbol(m_rgb_diff_5, uint8_t(corr));
        }
    }

    last = this_val;
    return buf + sizeof(las::rgb);
}

// DECOMPRESSOR

Rgb10Decompressor::Rgb10Decompressor(decoders::arithmetic<InCbStream>& decoder) : dec_(decoder)
{}

char *Rgb10Decompressor::decompress(char *buf)
{
    if (!have_last_) {
        // don't have the first data yet, read the whole point out of the stream
        have_last_ = true;

        dec_.getInStream().getBytes((unsigned char*)buf, sizeof(las::rgb));

        last.unpack(buf);
        return buf + sizeof(las::rgb);
    }

    unsigned char corr;
    int diff = 0;
    unsigned int sym = dec_.decodeSymbol(m_byte_used);

    las::rgb this_val;

    if (sym & (1 << 0))
    {
        corr = static_cast<unsigned char>(dec_.decodeSymbol(m_rgb_diff_0));
        this_val.r = static_cast<unsigned short>(uint8_t(corr + (last.r & 0xFF)));
    }
    else
    {
        this_val.r = last.r & 0xFF;
    }

    if (sym & (1 << 1))
    {
        corr = static_cast<unsigned char>(dec_.decodeSymbol(m_rgb_diff_1));
        this_val.r |= (static_cast<unsigned short>(uint8_t(corr + (last.r >> 8))) << 8);
    }
    else
    {
        this_val.r |= last.r & 0xFF00;
    }

    if (sym & (1 << 6))
    {
        diff = (this_val.r & 0xFF) - (last.r & 0xFF);

        if (sym & (1 << 2))
        {
            corr = static_cast<unsigned char>(dec_.decodeSymbol(m_rgb_diff_2));
            this_val.g = static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (last.g & 0xFF))));
        }
        else
        {
            this_val.g = last.g & 0xFF;
        }

        if (sym & (1 << 4))
        {
            corr = static_cast<unsigned char>(dec_.decodeSymbol(m_rgb_diff_4));
            diff = (diff + (this_val.g & 0xFF) - (last.g & 0xFF)) / 2;
            this_val.b = static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (last.b & 0xFF))));
        }
        else
        {
            this_val.b = last.b & 0xFF;
        }

        diff = (this_val.r >> 8) - (last.r >> 8);
        if (sym & (1 << 3))
        {
            corr = static_cast<unsigned char>(dec_.decodeSymbol(m_rgb_diff_3));
            this_val.g |= static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (last.g >> 8)))) << 8;
        }
        else {
            this_val.g |= last.g & 0xFF00;
        }

        if (sym & (1 << 5))
        {
            corr = static_cast<unsigned char>(dec_.decodeSymbol(m_rgb_diff_5));
            diff = (diff + (this_val.g >> 8) - (last.g >> 8)) / 2;

            this_val.b |= static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (last.b >> 8)))) << 8;
        }
        else {
            this_val.b |= (last.b & 0xFF00);
        }
    }
    else
    {
        this_val.g = this_val.r;
        this_val.b = this_val.r;
    }

    last = this_val;
    last.pack(buf);
    return buf + sizeof(las::rgb);
}

} // namespace detail
} // namespace lazperf
