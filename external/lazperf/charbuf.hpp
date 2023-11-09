/******************************************************************************
* Copyright (c) 2014, Hobu Inc.
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

#include <streambuf>
#include <iostream>
#include <vector>

namespace lazperf
{

/**
  Allow a data buffer to be used at a std::streambuf.
*/
class charbuf : public std::streambuf
{
public:
    /**
      Construct an empty charbuf.
    */
    charbuf() : m_bufOffset(0)
        {}

    /**
      Construct a charbuf that wraps a byte vector.

      \param v  Byte vector to back streambuf.
      \param bufOffset  Offset in vector (ignore bytes before offset).
    */
    charbuf (std::vector<char>& v, pos_type bufOffset = 0)
        { initialize(v.data(), v.size(), bufOffset); }

    /**
      Construct a charbuf that wraps a byte buffer.

      \param buf  Buffer to back streambuf.
      \param count  Size of buffer.
      \param bufOffset  Offset in vector (ignore bytes before offset).
    */
    charbuf(char *buf, size_t count, pos_type bufOffset = 0)
        { initialize(buf, count, bufOffset); }

    /**
      Set a buffer to back a charbuf.

      \param buf  Buffer to back streambuf.
      \param count  Size of buffer.
      \param bufOffset  Offset in vector (ignore bytes before offset).
    */
    void initialize(char *buf, size_t count, pos_type bufOffset = 0);

protected:
    /**
      Seek to a position in the buffer.

      \param pos  Position to seek to.
      \param which  I/O mode [default: rw]
      \return  Current position adjusted for buffer offset.
    */
    std::ios::pos_type seekpos(std::ios::pos_type pos,
        std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

    /**
      Seek to a position based on an offset from a position.

      \param off  Offset from current position.
      \param dir  Offset basis (beg, cur or end)
      \param which  I/O mode [default: rw]
      \return  Current position adjusted for buffer offset.
    */
    std::ios::pos_type seekoff(std::ios::off_type off,
        std::ios_base::seekdir dir,
        std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

private:
    /**
      Offset that allows one to seek to positions not based on the beginning
      of the backing vector, but to some other reference point.
    */
    std::ios::pos_type m_bufOffset;

    /**
      For the put pointer, it seems we need the beginning of the buffer
      in order to deal with offsets.
    */
    char *m_buf;
};

} //namespace lazperf
