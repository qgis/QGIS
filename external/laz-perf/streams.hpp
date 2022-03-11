/*
===============================================================================

  FILE:  streams.hpp

  CONTENTS:
    Stream abstractions

  PROGRAMMERS:

    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#ifndef __streams_hpp__
#define __streams_hpp__

#include <vector>
#include <iostream>

#include "lazperf.hpp"
#include "excepts.hpp"
#include "filestream.hpp"
#include "portable_endian.hpp"

namespace lazperf
{

struct OutCbStream
{
    OutCbStream(OutputCb outCb) : outCb_(outCb)
    {}

    void putBytes(const unsigned char *b, size_t len)
    {
        outCb_(b, len);
    }

    void putByte(const unsigned char b)
    {
        outCb_(&b, 1);
    }

    OutputCb outCb_;
};

struct InCbStream
{
    InCbStream(InputCb inCb) : inCb_(inCb)
    {}

    unsigned char getByte()
    {
        unsigned char c;
        inCb_(&c, 1);
        return c;
    }

    void getBytes(unsigned char *b, size_t len)
    {
        inCb_(b, len);
    }

    InputCb inCb_;
};

struct MemoryStream
{
    MemoryStream() : buf(), idx(0)
    {}

    void putBytes(const unsigned char* b, size_t len)
    {
        while(len --)
            buf.push_back(*b++);
    }

    void putByte(const unsigned char b)
    {
        buf.push_back(b);
    }

    OutputCb outCb()
    {
        using namespace std::placeholders;

        return std::bind(&MemoryStream::putBytes, this, _1, _2);
    }

    unsigned char getByte()
    {
        return buf[idx++];
    }

    void getBytes(unsigned char *b, int len)
    {
        for (int i = 0 ; i < len ; i ++)
            b[i] = getByte();
    }

    InputCb inCb()
    {
        using namespace std::placeholders;

        return std::bind(&MemoryStream::getBytes, this, _1, _2);
    }

    uint32_t numBytesPut() const
    {
        return buf.size();
    }

    // Copy bytes from the source stream to this stream.
    template <typename TSrc>
    void copy(TSrc& in, size_t bytes)
    {
        buf.resize(bytes);
        in.getBytes(buf.data(), bytes);
    }

    const uint8_t *data() const
    { return buf.data(); }

    const std::vector<unsigned char>& buffer() const
    { return buf; }

    std::vector<unsigned char>& buffer()
    { return buf; }

    std::vector<unsigned char> buf; // cuz I'm ze faste
    size_t idx;
};

template <typename TStream>
TStream& operator << (TStream& stream, uint32_t u)
{
    uint32_t uLe = htole32(u);
    stream.putBytes(reinterpret_cast<const unsigned char *>(&uLe), sizeof(uLe));
    return stream;
}

template <typename TStream>
TStream& operator >> (TStream& stream, uint32_t& u)
{
    uint32_t uLe;
    stream.getBytes(reinterpret_cast<unsigned char *>(&uLe), sizeof(u));
    u = le32toh(uLe);
    return stream;
}

} // namespace lazperf

#endif // __streams_hpp__
