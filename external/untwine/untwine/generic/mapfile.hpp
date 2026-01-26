
#pragma once

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

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
};

inline MapContext mapFile(const std::string& filename, bool readOnly, size_t pos, size_t size)
{
    MapContext ctx;

    if (!readOnly)
    {
        ctx.m_error = "readOnly must be true.";
        return ctx;
    }

    ctx.m_fd = ::open(filename.data(), readOnly ? O_RDONLY : O_RDWR);

    if (ctx.m_fd == -1)
    {
        ctx.m_error = "Mapped file couldn't be opened.";
        return ctx;
    }
    ctx.m_size = size;

    ctx.m_addr = ::mmap(0, size, PROT_READ, MAP_SHARED, ctx.m_fd, (off_t)pos);
    if (ctx.m_addr == MAP_FAILED)
    {
        ctx.m_addr = nullptr;
        ctx.m_error = "Couldn't map file";
    }

    return ctx;
}

inline MapContext unmapFile(MapContext ctx)
{
    if (::munmap(ctx.m_addr, ctx.m_size) == -1)
        ctx.m_error = "Couldn't unmap file.";
    else
    {
        ctx.m_addr = nullptr;
        ctx.m_size = 0;
        ctx.m_error = "";
    }
    ::close(ctx.m_fd);
    return ctx;
}

} // namespace os
} // namespace untwine
