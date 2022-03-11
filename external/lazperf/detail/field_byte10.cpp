/*
===============================================================================

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.
    andrew.bell.ia@gmail.com - Hobu Inc.
  
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

#include <deque>

namespace lazperf
{
namespace detail
{

Byte10Base::Byte10Base(size_t count) : count_(count), have_last_(false),
    lasts_(count), diffs_(count), models_(count, models::arithmetic(256))
{}

// COMPRESSOR

Byte10Compressor::Byte10Compressor(encoders::arithmetic<OutCbStream>& encoder, size_t count) :
    Byte10Base(count), enc_(encoder)
{}

const char *Byte10Compressor::compress(const char *buf)
{
    if (count_ == 0)
        return buf;

    auto li = lasts_.begin();
    auto di = diffs_.begin();
    while (di != diffs_.end())
    {
        *di = *buf - *li;
        *li = *buf;
        di++; buf++; li++;
    }

    if (!have_last_)
    {
        enc_.getOutStream().putBytes(lasts_.data(), count_);
        have_last_ = true;
    }
    else
    {
        di = diffs_.begin();
        auto mi = models_.begin();
        while (di != diffs_.end())
            enc_.encodeSymbol(*mi++, *di++);
    }
    return buf;
}

// DECOMPRESSOR

Byte10Decompressor::Byte10Decompressor(decoders::arithmetic<InCbStream>& decoder, size_t count) :
    Byte10Base(count), dec_(decoder)
{}

char *Byte10Decompressor::decompress(char *buf)
{
    if (count_ == 0)
        return buf;

    if (!have_last_)
    {
        dec_.getInStream().getBytes((unsigned char *)buf, count_);
        std::copy(buf, buf + count_, lasts_.data());
        have_last_ = true;
        return buf + count_;
    }
    // Use the diff vector for our current values.
    auto& curs = diffs_;
    auto ci = curs.begin();
    auto li = lasts_.begin();
    auto mi = models_.begin();
    while (li != lasts_.end())
    {
        *ci = (uint8_t)(*li + dec_.decodeSymbol(*mi));
        *li = *buf = *ci;
        li++; buf++; ci++; mi++;
    }
    return buf;
}

} // namespace detail
} // namespace lazperf
