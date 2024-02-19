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

#include "Common.hpp"

#ifdef _WIN32
#include <io.h>
#endif

#include <fcntl.h>
#include <filesystem>

namespace untwine
{

// This is here so that things will work with older version of PDAL.

MapContext mapFile(const std::string& filename, bool readOnly, size_t pos, size_t size)
{
    MapContext ctx;

    if (!readOnly)
    {
        ctx.m_error = "readOnly must be true.";
        return ctx;
    }

#ifndef _WIN32
    ctx.m_fd = ::open(filename.data(), readOnly ? O_RDONLY : O_RDWR);
#else
    ctx.m_fd = ::_wopen(toNative(filename).data(), readOnly ? _O_RDONLY : _O_RDWR);
#endif

    if (ctx.m_fd == -1)
    {
        ctx.m_error = "Mapped file couldn't be opened.";
        return ctx;
    }
    ctx.m_size = size;

#ifndef _WIN32
    ctx.m_addr = ::mmap(0, size, PROT_READ, MAP_SHARED, ctx.m_fd, (off_t)pos);
    if (ctx.m_addr == MAP_FAILED)
    {
        ctx.m_addr = nullptr;
        ctx.m_error = "Couldn't map file";
    }
#else
    ctx.m_handle = CreateFileMapping((HANDLE)_get_osfhandle(ctx.m_fd),
        NULL, PAGE_READONLY, 0, 0, NULL);
    uint32_t low = pos & 0xFFFFFFFF;
    uint32_t high = (pos >> 8);
    ctx.m_addr = MapViewOfFile(ctx.m_handle, FILE_MAP_READ, high, low,
        ctx.m_size);
    if (ctx.m_addr == nullptr)
        ctx.m_error = "Couldn't map file";
#endif

    return ctx;
}

MapContext unmapFile(MapContext ctx)
{
#ifndef _WIN32
    if (::munmap(ctx.m_addr, ctx.m_size) == -1)
        ctx.m_error = "Couldn't unmap file.";
    else
    {
        ctx.m_addr = nullptr;
        ctx.m_size = 0;
        ctx.m_error = "";
    }
    ::close(ctx.m_fd);
#else
    if (UnmapViewOfFile(ctx.m_addr) == 0)
        ctx.m_error = "Couldn't unmap file.";
    else
    {
        ctx.m_addr = nullptr;
        ctx.m_size = 0;
        ctx.m_error = "";
    }
    CloseHandle(ctx.m_handle);
    ::_close(ctx.m_fd);
#endif
    return ctx;
}

// PDAL's directoryList had a bug, so we've imported a working
// version here so that we can still use older PDAL releases.

#ifndef __APPLE_CC__
std::vector<std::string> directoryList(const std::string& dir)
{
    namespace fs = std::filesystem;

    std::vector<std::string> files;

    try
    {
        fs::directory_iterator it(untwine::toNative(dir));
        fs::directory_iterator end;
        while (it != end)
        {
            files.push_back(untwine::fromNative(it->path()));
            it++;
        }
    }
    catch (fs::filesystem_error&)
    {
        files.clear();
    }
    return files;
}
#else

#include <dirent.h>

// Provide simple opendir/readdir solution for OSX because directory_iterator is
// not available until OSX 10.15
std::vector<std::string> directoryList(const std::string& dir)
{
    std::vector<std::string> files;

    DIR *dpdf = opendir(dir.c_str());
    if (dpdf)
    {
        while (true)
        {
            struct dirent *epdf = readdir(dpdf);
            if (!epdf)
                break;

            std::string name = untwine::fromNative(epdf->d_name);
            // Skip paths
            if (!pdal::Utils::iequals(name, ".") &&
                !pdal::Utils::iequals(name, ".."))
            {
                // we expect the path + name
                files.push_back(dir + "/" + untwine::fromNative(epdf->d_name));
            }
        }
        closedir(dpdf);
    }
    return files;
}
#endif

} // namespace untwine

