/******************************************************************************
* Copyright (c) 2014, Hobu Inc., hobu@hobu.co
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/
#pragma once

#include <cstring>

#include "portable_endian.hpp"

namespace lazperf
{

class Inserter
{
public:
    Inserter(unsigned char *buf, std::size_t size) : m_pbase((char *)buf),
        m_epptr((char *)buf + size), m_pptr((char *)buf)
    {}
    Inserter(char *buf, std::size_t size) : m_pbase(buf),
        m_epptr(buf + size), m_pptr(buf)
    {}

protected:
    // Base pointer - start of buffer (names taken from std::streambuf).
    char *m_pbase;
    // End pointer.
    char *m_epptr;
    // Current position.
    char *m_pptr;

public:
    operator bool() const
        { return good(); }
    bool good() const
        { return m_pptr < m_epptr; }
    void seek(std::size_t pos)
        { m_pptr = m_pbase + pos; }
    void put(const std::string& s)
        { put(s, s.size()); }
    void put(std::string s, size_t len)
    {
        s.resize(len);
        put(s.data(), len);
    }
    void put(const char *c, size_t len)
    {
        memcpy(m_pptr, c, len);
        m_pptr += len;
    }
    void put(const unsigned char *c, size_t len)
    {
        memcpy(m_pptr, c, len);
        m_pptr += len;
    }
    std::size_t position() const
        { return m_pptr - m_pbase; }

    virtual Inserter& operator << (uint8_t v) = 0;
    virtual Inserter& operator << (int8_t v) = 0;
    virtual Inserter& operator << (uint16_t v) = 0;
    virtual Inserter& operator << (int16_t v) = 0;
    virtual Inserter& operator << (uint32_t v) = 0;
    virtual Inserter& operator << (int32_t v) = 0;
    virtual Inserter& operator << (uint64_t v) = 0;
    virtual Inserter& operator << (int64_t v) = 0;
    virtual Inserter& operator << (float v) = 0;
    virtual Inserter& operator << (double v) = 0;
};

/// Stream wrapper for output of binary data that converts from host ordering
/// to little endian format
class LeInserter : public Inserter
{
public:
    LeInserter(char *buf, std::size_t size) : Inserter(buf, size)
    {}
    LeInserter(unsigned char *buf, std::size_t size) : Inserter(buf, size)
    {}

    LeInserter& operator << (uint8_t v)
    {
        *m_pptr++ = (char)v;
        return *this;
    }

    LeInserter& operator << (int8_t v)
    {
        *m_pptr++ = v;
        return *this;
    }

    LeInserter& operator << (uint16_t v)
    {
        v = htole16(v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    LeInserter& operator << (int16_t v)
    {
        v = (int16_t)htole16((uint16_t)v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    LeInserter& operator << (uint32_t v)
    {
        v = htole32(v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    LeInserter& operator << (int32_t v)
    {
        v = (int32_t)htole32((uint32_t)v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    LeInserter& operator << (uint64_t v)
    {
        v = htole64(v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    LeInserter& operator << (int64_t v)
    {
        v = (int64_t)htole64((uint64_t)v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    LeInserter& operator << (float v)
    {
        union
        {
            float f;
            uint32_t u;
        } uu;

        uu.f = v;
        uu.u = htole32(uu.u);
        memcpy(m_pptr, &uu.f, sizeof(uu.f));
        m_pptr += sizeof(uu.f);
        return *this;
    }

    LeInserter& operator << (double v)
    {
        union
        {
            double d;
            uint64_t u;
        } uu;

        uu.d = v;
        uu.u = htole64(uu.u);
        memcpy(m_pptr, &uu.d, sizeof(uu.d));
        m_pptr += sizeof(uu.d);
        return *this;
    }
};


/// Stream wrapper for output of binary data that converts from host ordering
/// to big endian format
class BeInserter : public Inserter
{
public:
    BeInserter(char *buf, std::size_t size) : Inserter(buf, size)
    {}
    BeInserter(unsigned char *buf, std::size_t size) : Inserter(buf, size)
    {}

    BeInserter& operator << (uint8_t v)
    {
        *m_pptr++ = (char)v;
        return *this;
    }

    BeInserter& operator << (int8_t v)
    {
        *m_pptr++ = v;
        return *this;
    }

    BeInserter& operator << (uint16_t v)
    {
        v = htobe16(v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    BeInserter& operator << (int16_t v)
    {
        v = (int16_t)htobe16((uint16_t)v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    BeInserter& operator << (uint32_t v)
    {
        v = htobe32(v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    BeInserter& operator << (int32_t v)
    {
        v = (int32_t)htobe32((uint32_t)v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    BeInserter& operator << (uint64_t v)
    {
        v = htobe64(v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    BeInserter& operator << (int64_t v)
    {
        v = (int64_t)htobe64((uint64_t)v);
        memcpy(m_pptr, &v, sizeof(v));
        m_pptr += sizeof(v);
        return *this;
    }

    BeInserter& operator << (float v)
    {
        union
        {
            float f;
            uint32_t u;
        } uu;

        uu.f = v;
        uu.u = htobe32(uu.u);
        memcpy(m_pptr, &uu.f, sizeof(uu.f));
        m_pptr += sizeof(uu.f);
        return *this;
    }

    BeInserter& operator << (double v)
    {
        union
        {
            double d;
            uint64_t u;
        } uu;

        uu.d = v;
        uu.u = htobe64(uu.u);
        memcpy(m_pptr, &uu.d, sizeof(uu.d));
        m_pptr += sizeof(uu.d);
        return *this;
    }
};

} // namespace lazperf
