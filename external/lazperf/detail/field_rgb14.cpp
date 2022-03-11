/*
===============================================================================

  FILE:  field_rgb14.cpp
  
  CONTENTS:
    

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

namespace lazperf
{
namespace detail
{

namespace
{

unsigned int color_diff_bits_1_4(const las::rgb14& color, const las::rgb14& last)
{
    const las::rgb14& a = last;
    const las::rgb14& b = color;

    auto flag_diff = [](unsigned short c1, unsigned short c2, int mask) -> bool
    {
        return ((c1 ^ c2) & mask);
    };

    unsigned int r =
        (flag_diff(a.r, b.r, 0x00FF) << 0) |
        (flag_diff(a.r, b.r, 0xFF00) << 1) |
        (flag_diff(a.g, b.g, 0x00FF) << 2) |
        (flag_diff(a.g, b.g, 0xFF00) << 3) |
        (flag_diff(a.b, b.b, 0x00FF) << 4) |
        (flag_diff(a.b, b.b, 0xFF00) << 5) |
        ((flag_diff(b.r, b.g, 0x00FF) |
          flag_diff(b.r, b.b, 0x00FF) |
          flag_diff(b.r, b.g, 0xFF00) |
          flag_diff(b.r, b.b, 0xFF00)) << 6);
    return r;
}

} // unnamed namespace


void Rgb14Compressor::writeSizes()
{
    rgb_enc_.done();
    stream_ << rgb_enc_.num_encoded();
}

void Rgb14Compressor::writeData()
{
    LAZDEBUG(std::cerr << "RGB       : " <<
        utils::sum(rgb_enc_.encoded_bytes(), rgb_enc_.num_encoded()) << "\n");

    if (rgb_enc_.num_encoded())
        stream_.putBytes(rgb_enc_.encoded_bytes(), rgb_enc_.num_encoded());
}

const char *Rgb14Compressor::compress(const char *buf, int& sc)
{
    const las::rgb14 color(buf);

    // don't have the first data yet, just push it to our
    // have last stuff and move on
    if (last_channel_ == -1)
    {
        ChannelCtx& c = chan_ctxs_[sc];
        stream_.putBytes((const unsigned char*)&color, sizeof(las::rgb));
        c.last_ = color;
        c.have_last_ = true;
        last_channel_ = sc;
        return buf + sizeof(las::rgb);
    }

    ChannelCtx& c = chan_ctxs_[sc];
    las::rgb14 *pLastColor = &chan_ctxs_[last_channel_].last_;
    if (!c.have_last_)
    {
        c.have_last_ = true;
        c.last_ = *pLastColor;
        pLastColor = &c.last_;
    }
    // This mess is because of the broken-ness of the handling for last in v3, where
    // 'last_point' only gets updated on the first context switch in the LASzip code.
    las::rgb& lastColor = *pLastColor;

    // compress color
    int diff_l = 0;
    int diff_h = 0;

    unsigned int sym = detail::color_diff_bits_1_4(color, lastColor);
    if (sym)
        rgb_enc_.makeValid();
    rgb_enc_.encodeSymbol(c.used_model_, sym);

    // high and low R
    if (sym & (1 << 0))
    {
        diff_l = (color.r & 0xFF) - (lastColor.r & 0xFF);
        rgb_enc_.encodeSymbol(c.diff_model_[0], uint8_t(diff_l));
    }
    if (sym & (1 << 1))
    {
        diff_h = static_cast<int>(color.r >> 8) - (lastColor.r >> 8);
        rgb_enc_.encodeSymbol(c.diff_model_[1], uint8_t(diff_h));
    }

    // Only encode green and blue if they are different from red.
    if (sym & (1 << 6))
    {
        if (sym & (1 << 2))
        {
            int corr = static_cast<int>(color.g & 0xFF) -
                utils::clamp<uint8_t>(diff_l + (lastColor.g & 0xFF));
            rgb_enc_.encodeSymbol(c.diff_model_[2], uint8_t(corr));
        }

        if (sym & (1 << 4))
        {
            diff_l = (diff_l + (color.g & 0xFF) - (lastColor.g & 0xFF)) / 2;
            int corr = static_cast<int>(color.b & 0xFF) -
                utils::clamp<uint8_t>(diff_l + (lastColor.b & 0xFF));
            rgb_enc_.encodeSymbol(c.diff_model_[4], uint8_t(corr));
        }

        if (sym & (1 << 3))
        {
            int corr = static_cast<int>(color.g >> 8) -
                utils::clamp<uint8_t>(diff_h + (lastColor.g >> 8));
            rgb_enc_.encodeSymbol(c.diff_model_[3], uint8_t(corr));
        }

        if (sym & (1 << 5))
        {
            diff_h = (diff_h + ((color.g >> 8)) - (lastColor.g >> 8)) / 2;
            int corr = static_cast<int>(color.b >> 8) -
                utils::clamp<uint8_t>(diff_h + (lastColor.b >> 8));
            rgb_enc_.encodeSymbol(c.diff_model_[5], uint8_t(corr));
        }
    }

    lastColor = color;
    last_channel_ = sc;
    return buf + sizeof(las::rgb14);
}

// DECOMPRESSOR

void Rgb14Decompressor::dumpSums()
{
    std::cout << "RGB      : " << sumRgb.value() << "\n";
}

void Rgb14Decompressor::readSizes()
{
    stream_ >> rgb_cnt_;
}

void Rgb14Decompressor::readData()
{
    rgb_dec_.initStream(stream_, rgb_cnt_);
}

char *Rgb14Decompressor::decompress(char *buf, int& sc)
{
    if (last_channel_ == -1)
    {
        ChannelCtx& c = chan_ctxs_[sc];
        stream_.getBytes((unsigned char*)buf, sizeof(las::rgb));
        c.last_.unpack(buf);
        c.have_last_ = true;
        last_channel_ = sc;
        return buf + sizeof(las::rgb14);
    }
    if (rgb_cnt_ == 0)
    {
        las::rgb14 *color = reinterpret_cast<las::rgb14 *>(buf);
        *color = chan_ctxs_[last_channel_].last_;
        return buf + sizeof(las::rgb14);
    }

    ChannelCtx& c = chan_ctxs_[sc];
    las::rgb14 *pLastColor = &chan_ctxs_[last_channel_].last_;
    if (sc != last_channel_)
    {
        last_channel_ = sc;
        if (!c.have_last_)
        {
            c.have_last_ = true;
            c.last_ = *pLastColor;
            pLastColor = &chan_ctxs_[last_channel_].last_;
        }
    }
    las::rgb14& lastColor = *pLastColor;

    uint32_t sym = rgb_dec_.decodeSymbol(c.used_model_);

    las::rgb14 color;

    if (sym & (1 << 0))
    {
        uint8_t corr = (uint8_t)rgb_dec_.decodeSymbol(c.diff_model_[0]);
        color.r = static_cast<unsigned short>(uint8_t(corr + (lastColor.r & 0xFF)));
    }
    else
        color.r = lastColor.r & 0xFF;

    if (sym & (1 << 1))
    {
        uint8_t corr = (uint8_t)rgb_dec_.decodeSymbol(c.diff_model_[1]);
        color.r |= (static_cast<unsigned short>(uint8_t(corr + (lastColor.r >> 8))) << 8);
    }
    else
        color.r |= lastColor.r & 0xFF00;

    if (sym & (1 << 6))
    {
        int diff = (color.r & 0xFF) - (lastColor.r & 0xFF);

        if (sym & (1 << 2))
        {
            uint8_t corr = (uint8_t)rgb_dec_.decodeSymbol(c.diff_model_[2]);
            color.g = static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (lastColor.g & 0xFF))));
        }
        else
            color.g = lastColor.g & 0xFF;

        if (sym & (1 << 4))
        {
            uint8_t corr = (uint8_t)rgb_dec_.decodeSymbol(c.diff_model_[4]);
            diff = (diff + ((color.g & 0xFF) - (lastColor.g & 0xFF))) / 2;
            color.b = static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (lastColor.b & 0xFF))));
        }
        else
            color.b = lastColor.b & 0xFF;

        diff = (color.r >> 8) - (lastColor.r >> 8);
        if (sym & (1 << 3))
        {
            uint8_t corr = (uint8_t)rgb_dec_.decodeSymbol(c.diff_model_[3]);
            color.g |= (static_cast<unsigned short>(uint8_t(corr +
                utils::clamp<uint8_t>(diff + (lastColor.g >> 8))))) << 8;
        }
        else
            color.g |= lastColor.g & 0xFF00;

        if (sym & (1 << 5))
        {
            uint8_t corr = (uint8_t)rgb_dec_.decodeSymbol(c.diff_model_[5]);
            diff = (diff + (color.g >> 8) - (lastColor.g >> 8)) / 2;
            color.b |= (static_cast<unsigned short>(uint8_t(corr +
                            utils::clamp<uint8_t>(diff + (lastColor.b >> 8))))) << 8;
        }
        else
            color.b |= (lastColor.b & 0xFF00);
    }
    else
    {
        color.g = color.r;
        color.b = color.r;
    }

    LAZDEBUG(sumRgb.add(color));
    lastColor = color;
    color.pack(buf);
    return buf + sizeof(las::rgb14);
}

} // namespace detail
} // namespace lazperf
