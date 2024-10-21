#pragma once

#include <Windows.h>
#include <io.h>
#include <fcntl.h>

#include <stringconv.hpp>

namespace untwine
{
namespace os
{

//ABELL - This exists here because older version of PDAL don't have it and the QGIS
//  crew wanted things to work with older versions of PDAL.
/**
  Context info for mapping a file.
*/
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
    HANDLE m_handle;
};

inline MapContext mapFile(const std::string& filename, bool readOnly, size_t pos, size_t size)
{
    MapContext ctx;

    if (!readOnly)
    {
        ctx.m_error = "readOnly must be true.";
        return ctx;
    }

    ctx.m_fd = ::_wopen(toNative(filename).data(), readOnly ? _O_RDONLY : _O_RDWR);

    if (ctx.m_fd == -1)
    {
        ctx.m_error = "Mapped file couldn't be opened.";
        return ctx;
    }
    ctx.m_size = size;

    ctx.m_handle = CreateFileMapping((HANDLE)_get_osfhandle(ctx.m_fd),
        NULL, PAGE_READONLY, 0, 0, NULL);
    uint32_t low = pos & 0xFFFFFFFF;
    uint32_t high = (pos >> 8);
    ctx.m_addr = MapViewOfFile(ctx.m_handle, FILE_MAP_READ, high, low,
        ctx.m_size);
    if (ctx.m_addr == nullptr)
        ctx.m_error = "Couldn't map file";

    return ctx;
}

inline MapContext unmapFile(MapContext ctx)
{
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
    return ctx;
}

} // namespace os
} // namespace untwine
