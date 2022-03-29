/******************************************************************************
 * Copyright (c) 2020, Hobu Inc.
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

#include <fcntl.h>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <string>

namespace untwine
{
    struct MapContext
    {
    public:
        MapContext() : m_fd(-1), m_addr(nullptr)
        {}

        void *addr() const
        { return m_addr; }
        std::string what() const
        { return m_error; }

        int m_fd;
        size_t m_size;
        void *m_addr;
        std::string m_error;
#ifdef _WIN32
        HANDLE m_handle;
#endif
    };
    /**
      Map a file to memory.
      \param filename  Filename to map.
      \param readOnly  Must be true at this time.
      \param pos       Starting position of file to map.
      \param size      Number of bytes in file to map.
      \return  MapContext.  addr() gets the mapped address.  what() gets
         any error message.  addr() returns nullptr on error.
    */
    MapContext mapFile(const std::string& filename, bool readOnly,
        size_t pos, size_t size);

    /**
      Unmap a previously mapped file.
      \param ctx  Previously returned MapContext
      \return  MapContext indicating current state of the file mapping.
    */
    MapContext unmapFile(MapContext ctx);

} // namespace untwine

