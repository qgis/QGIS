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

#include "charbuf.hpp"

namespace lazperf
{

void charbuf::initialize(char *buf, size_t count, std::ios::pos_type bufOffset)
{
    m_bufOffset = bufOffset;
    m_buf = buf;
    setg(buf, buf, buf + count);
    setp(buf, buf + count);
}


std::ios::pos_type charbuf::seekpos(std::ios::pos_type pos,
    std::ios_base::openmode which)
{
    pos -= m_bufOffset;
    if (which & std::ios_base::in)
    {
        if (pos >= egptr() - eback())
            return -1;
        char *cpos = eback() + pos;
        setg(eback(), cpos, egptr());
    }
    if (which & std::ios_base::out)
    {
        if (pos > epptr() - m_buf)
            return -1;
        char *cpos = m_buf + pos;
        setp(cpos, epptr());
    }
    return pos;
}

std::ios::pos_type
charbuf::seekoff(std::ios::off_type off, std::ios_base::seekdir dir,
    std::ios_base::openmode which)
{
    std::ios::pos_type pos;
    char *cpos = nullptr;
    if (which & std::ios_base::in)
    {
        switch (dir)
        {
        case std::ios::beg:
            cpos = eback() + off - m_bufOffset;
            break;
        case std::ios::cur:
            cpos = gptr() + off;
            break;
        case std::ios::end:
            cpos = egptr() - off;
            break;
        default:
            break;  // Should never happen.
        }
        if (cpos < eback() || cpos > egptr())
            return -1;
        setg(eback(), cpos, egptr());
        pos = cpos - eback();
    }
    if (which & std::ios_base::out)
    {
        switch (dir)
        {
        case std::ios::beg:
            cpos = m_buf + off - m_bufOffset;
            break;
        case std::ios::cur:
            cpos = pptr() + off;
            break;
        case std::ios::end:
            cpos = egptr() - off;
            break;
        default:
            break;  // Should never happen.
        }
        if (cpos < m_buf || cpos > epptr())
            return -1;
        setp(cpos, epptr());
        pos = cpos - m_buf;
    }
    return pos;
}

} //namespace lazperf
