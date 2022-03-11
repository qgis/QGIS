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
#include <vector>

#include "portable_endian.hpp"

namespace lazperf
{

/**
  Buffer wrapper for input of binary data from a buffer.
*/
class Extractor
{
public:
    /**
      Construct an extractor to operate on a buffer.

      \param buf  Buffer to extract from.
      \param size  Buffer size.
    */
    Extractor(const char *buf, std::size_t size) : m_eback(buf),
        m_egptr(buf + size), m_gptr(buf)
    {}

public:
    /**
      Determine if the buffer is good.

      \return  Whether the buffer is good.
    */
    operator bool ()
        { return good(); }

    /**
      Seek to a position in the buffer.

      \param pos  Position to seek in buffer.
    */
    void seek(std::size_t pos)
        { m_gptr = m_eback + pos; }

    /**
      Advance buffer position.

      \param cnt  Number of bytes to skip in buffer.
    */
    void skip(std::size_t cnt)
        { m_gptr += cnt; }

    /**
      Return the get position of buffer.

      \return  Get position.
    */
    size_t position() const
        { return m_gptr - m_eback; }

    /**
      Determine whether the extractor is good (the get pointer is in the
      buffer).

      \return  Whether the get pointer is valid.
    */
    bool good() const
        { return m_gptr < m_egptr; }

    /**
      Extract a string of a particular size from the buffer.  Trim trailing
      null bytes.

      \param s  String to extract to.
      \param size  Number of bytes to extract from buffer into string.
    */
    void get(std::string& s, size_t size)
    {
        s = std::string(m_gptr, size);
        m_gptr += size;
        while (--size)
        {
            if (s[size] != '\0')
                break;
            else if (size == 0)
            {
                s.clear();
                return;
            }
        }
        s.resize(size + 1);
    }

    /**
      Extract data to char vector.  Vector must be sized to indicate
      number of bytes to extract.

      \param buf  Vector to which bytes should be extracted.
    */
    void get(std::vector<char>& buf)
    {
        memcpy((char *)buf.data(), m_gptr, buf.size());
        m_gptr += buf.size();
    }

    /**
      Extract data to unsigned char vector.  Vector must be sized to
      indicate number of bytes to extract.

      \param buf  Vector to which bytes should be extracted.
    */
    void get(std::vector<unsigned char>& buf)
    {
        memcpy((char *)buf.data(), m_gptr, buf.size());
        m_gptr += buf.size();
    }

    /**
      Extract data into a provided buffer.

      \param buf  Pointer to buffer to which bytes should be extracted.
      \param size  Number of bytes to extract.
    */
    void get(char *buf, size_t size)
    {
        memcpy(buf, m_gptr, size);
        m_gptr += size;
    }

    /**
      Extract data into a provided unsigned buffer.

      \param buf  Pointer to buffer to which bytes should be extracted.
      \param size  Number of bytes to extract.
    */
    void get(unsigned char *buf, size_t size)
    {
        memcpy(buf, m_gptr, size);
        m_gptr += size;
    }

    virtual Extractor& operator >> (uint8_t& v) = 0;
    virtual Extractor& operator >> (int8_t& v) = 0;
    virtual Extractor& operator >> (uint16_t& v) = 0;
    virtual Extractor& operator >> (int16_t& v) = 0;
    virtual Extractor& operator >> (uint32_t& v) = 0;
    virtual Extractor& operator >> (int32_t& v) = 0;
    virtual Extractor& operator >> (uint64_t& v) = 0;
    virtual Extractor& operator >> (int64_t& v) = 0;
    virtual Extractor& operator >> (float& v) = 0;
    virtual Extractor& operator >> (double& v) = 0;

protected:
    const char *m_eback;  ///< Start of the buffer (name from std::streambuf)
    const char *m_egptr;  ///< End of the buffer.
    const char *m_gptr;   ///< Current get position.
};

/**
  Wrapper extraction of little-endian data from a buffer to host ordering.
*/
class LeExtractor : public Extractor
{
public:
    /**
      Construct extractor for a buffer.

      \param buf  Buffer from which to extract.
      \param size  Size of buffer.
    */
    LeExtractor(const char *buf, std::size_t size) : Extractor(buf, size)
    {}

    /**
      Extract an unsigned byte from a buffer.

      \param v  Unsigned byte to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (uint8_t& v)
    {
        v = *(const uint8_t *)m_gptr++;
        return *this;
    }

    /**
      Extract a byte from a buffer.

      \param v  Byte to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (int8_t& v)
    {
        v = *(const int8_t *)m_gptr++;
        return *this;
    }

    /**
      Extract an unsgined short from a buffer.

      \param v  Short to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (uint16_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = le16toh(v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a short from a buffer.

      \param v  Short to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (int16_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = (int16_t)le16toh((uint16_t)v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract an unsigned int from a buffer.

      \param v  Unsigned int to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (uint32_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = le32toh(v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract an int from a buffer.

      \param v  int to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (int32_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = (int32_t)le32toh((uint32_t)v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract an unsigned long int from a buffer.

      \param v  unsigned long int to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (uint64_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = le64toh(v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a long int from a buffer.

      \param v  long int to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (int64_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = (int64_t)le64toh((uint64_t)v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a float from a buffer.

      \param v  float to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (float& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        uint32_t tmp = le32toh(*(uint32_t *)(&v));
        memcpy(&v, &tmp, sizeof(tmp));
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a double from a buffer.

      \param v  double to extract to.
      \return  This extractor.
    */
    LeExtractor& operator >> (double& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        uint64_t tmp = le64toh(*(uint64_t *)(&v));
        memcpy(&v, &tmp, sizeof(tmp));
        m_gptr += sizeof(v);
        return *this;
    }
};


/**
  Wrapper extraction of big-endian data from a buffer to host ordering.
*/
class BeExtractor : public Extractor
{
public:
    /**
      Construct extractor for a buffer.

      \param buf  Buffer from which to extract.
      \param size  Size of buffer.
    */
    BeExtractor(const char *buf, std::size_t size) : Extractor(buf, size)
    {}

    /**
      Extract an unsigned byte from a buffer.

      \param v  unsigned byte to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (uint8_t& v)
    {
        v = *(const uint8_t *)m_gptr++;
        return *this;
    }

    /**
      Extract a byte from a buffer.

      \param v  byte to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (int8_t& v)
    {
        v = *(const int8_t *)m_gptr++;
        return *this;
    }

    /**
      Extract an unsigned short from a buffer.

      \param v  unsigned short to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (uint16_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = be16toh(v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a short from a buffer.

      \param v  short to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (int16_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = (int16_t)be16toh((uint16_t)v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract an unsigned int from a buffer.

      \param v  unsigned int to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (uint32_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = be32toh(v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract an int from a buffer.

      \param v  int to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (int32_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = (int32_t)be32toh((uint32_t)v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract an unsigned long int from a buffer.

      \param v  unsigned long int to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (uint64_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = be64toh(v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a long int from a buffer.

      \param v  long int to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (int64_t& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        v = (int64_t)be64toh((uint64_t)v);
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a float from a buffer.

      \param v  float to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (float& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        uint32_t tmp = be32toh(*(uint32_t *)(&v));
        memcpy(&v, &tmp, sizeof(tmp));
        m_gptr += sizeof(v);
        return *this;
    }

    /**
      Extract a double from a buffer.

      \param v  double to extract to.
      \return  This extractor.
    */
    BeExtractor& operator >> (double& v)
    {
        memcpy(&v, m_gptr, sizeof(v));
        uint64_t tmp = be64toh(*(uint64_t *)(&v));
        memcpy(&v, &tmp, sizeof(tmp));
        m_gptr += sizeof(v);
        return *this;
    }
};

} // namespace lazperf

