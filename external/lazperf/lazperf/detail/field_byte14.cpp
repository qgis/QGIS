/*
===============================================================================

  FILE:  field_byte14.cpp
  
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

Byte14Base::Byte14Base(size_t count) : count_(count), last_channel_(-1),
    chan_ctxs_ { count_, count_, count_, count_ }
{}

size_t Byte14Base::count() const
{
    return count_;
}

// COMPRESSOR

Byte14Compressor::Byte14Compressor(OutCbStream& stream, size_t count) :
    Byte14Base(count), stream_(stream), valid_(count_),
    byte_enc_(count, encoders::arithmetic<MemoryStream>(true))
{}

void Byte14Compressor::writeSizes()
{
    for (size_t i = 0; i < count_; ++i)
    {
        if (valid_[i])
        {
            byte_enc_[i].done();
            stream_ << byte_enc_[i].num_encoded();
        }
        else
            stream_ << (uint32_t)0;
    }
}

void Byte14Compressor::writeData()
{
    [[maybe_unused]] int32_t total = 0;
    for (size_t i = 0; i < count_; ++i)
    {
        if (valid_[i])
        {
            stream_.putBytes(byte_enc_[i].encoded_bytes(), byte_enc_[i].num_encoded());
            total += utils::sum(byte_enc_[i].encoded_bytes(), byte_enc_[i].num_encoded());
        }
    }
    LAZDEBUG(std::cerr << "BYTE      : " << total << "\n");
}

const char *Byte14Compressor::compress(const char *buf, int& sc)
{
    // don't have the first data yet, just push it to our
    // have last stuff and move on
    if (last_channel_ == -1)
    {
        ChannelCtx& c = chan_ctxs_[sc];
        stream_.putBytes((const unsigned char *)buf, count_);
        c.last_.assign(buf, buf + count_);
        c.have_last_ = true;
        last_channel_ = sc;
        return buf + count_;
    }
    ChannelCtx& c = chan_ctxs_[sc];
    las::byte14 *pLastBytes = &chan_ctxs_[last_channel_].last_;
    if (!c.have_last_)
    {
        c.have_last_ = true;
        c.last_ = *pLastBytes;
        pLastBytes = &c.last_;
    }
    // This mess is because of the broken-ness of the handling for last in v3, where
    // 'last_point' only gets updated on the first context switch in the LASzip code.
    las::byte14& lastBytes = *pLastBytes;

    for (size_t i = 0; i < count_; ++i, ++buf)
    {
        int32_t diff = *(const uint8_t *)buf - lastBytes[i];
        byte_enc_[i].encodeSymbol(c.byte_model_[i], (uint8_t)diff);
        if (diff)
        {
            valid_[i] = true;
            lastBytes[i] = *buf;
        }
    }

    last_channel_ = sc;
    return buf + count_;
}

// DECOMPRESSOR

Byte14Decompressor::Byte14Decompressor(InCbStream& stream, size_t count) : Byte14Base(count),
    stream_(stream), byte_cnt_(count_), byte_dec_(count_, decoders::arithmetic<MemoryStream>())
{}

void Byte14Decompressor::readSizes()
{
    for (size_t i = 0; i < count_; ++i)
        stream_ >> byte_cnt_[i];
}

void Byte14Decompressor::readData()
{
    for (size_t i = 0; i < count_; ++i)
        byte_dec_[i].initStream(stream_, byte_cnt_[i]);
}

void Byte14Decompressor::dumpSums()
{
    std::cout << "BYTE     : " << sumByte.value() << "\n";
}

char *Byte14Decompressor::decompress(char *buf, int& sc)
{
    if (last_channel_ == -1)
    {
        ChannelCtx& c = chan_ctxs_[sc];
        stream_.getBytes((unsigned char *)buf, count_);
        c.last_.assign(buf, buf + count_);
        c.have_last_ = true;
        last_channel_ = sc;
        return buf + count_;
    }

    ChannelCtx& c = chan_ctxs_[sc];
    las::byte14 *pLastByte = &chan_ctxs_[last_channel_].last_;
    if (sc != last_channel_)
    {
        last_channel_ = sc;
        if (!c.have_last_)
        {
            c.have_last_ = true;
            c.last_ = *pLastByte;
            pLastByte = &chan_ctxs_[last_channel_].last_;
        }
    }
    las::byte14& lastByte = *pLastByte;

    for (size_t i = 0; i < count_; ++i, buf++)
    {
        if (byte_cnt_[i])
        {
            *buf = lastByte[i] + byte_dec_[i].decodeSymbol(c.byte_model_[i]);
            lastByte[i] = *buf;
        }
        else
            *buf = lastByte[i];
    }
    LAZDEBUG(sumByte.add(lastByte.data(), count_));

    return buf;
}

} // namespace detail
} // namespace lazperf
