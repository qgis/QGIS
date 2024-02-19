#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <stdint.h>
#include <array>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include "FileDimInfo.hpp"

namespace untwine
{

// Number of cells into which points are put for each octree voxel.
const int CellCount = 128;

using PointCount = uint64_t;
using StringList = std::vector<std::string>;

class  FatalError : public std::runtime_error
{
public:
    inline FatalError(std::string const& msg) : std::runtime_error(msg)
        {}
};

struct Options
{
    std::string outputName;
    StringList inputFiles;
    std::string tempDir;
    bool doCube;
    size_t fileLimit;
    int level;
    int progressFd;
    bool progressDebug;
    StringList dimNames;
    bool stats;
    std::string a_srs;
    bool metadata;
    bool dummy;
};

struct BaseInfo
{
public:
    BaseInfo()
    {};

    bool preserveHeaderFields() const
        {return opts.inputFiles.size() == 1; }

    Options opts;
    pdal::BOX3D bounds;
    pdal::BOX3D trueBounds;
    size_t pointSize;
    std::string outputFile;
    DimInfoList dimInfo;
    pdal::SpatialReference srs;
    int pointFormatId;
    uint16_t globalEncoding {0};
    uint16_t creationYear {1};
    uint16_t creationDoy {1};
    uint16_t fileSourceId {0};
    std::string systemId;
    std::string generatingSoftware { "Untwine" };

    using d3 = std::array<double, 3>;
    d3 scale { -1.0, -1.0, -1.0 };
    d3 offset { std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN(),
                std::numeric_limits<double>::quiet_NaN()};
};

// We make a special dimension to store the bits (class flags, scanner channel, scan dir, eofl).
const std::string UntwineBitsDimName { "UntwineBitsUntwine" };
// That special dimension is a byte in size.
const pdal::Dimension::Type UntwineBitsType { pdal::Dimension::Type::Unsigned8 };

// PDAL explodes the class flags, leaving us with the following dimensions that map to the
// special "untwine bits" dimension.
inline bool isUntwineBitsDim(const std::string& s)
{
    static const std::vector<std::string> bitsDims
        { "Synthetic", "KeyPoint", "Overlap", "Withheld", "ScanChannel",
          "ScanDirectionFlag", "EdgeOfFlightLine", "ClassFlags" };

    return std::find(bitsDims.begin(), bitsDims.end(), s) != bitsDims.end();
}

// The position is the bit position of a bit, or the number of bits to shift an integer
// value to get it to the right location in the UntwineBits dimension.
inline int getUntwineBitPos(const std::string& s)
{
    static std::unordered_map<std::string, int> positions {
        {"Synthetic", 0},
        {"KeyPoint", 1},
        {"Withheld", 2},
        {"Overlap", 3},
        {"ScanChannel", 5},
        {"ScanDirectionFlag", 6},
        {"EdgeOfFlightLine", 7},
        {"ClassFlags", 0}
    };
    auto it = positions.find(s);
    if (it == positions.end())
        return -1;
    return it->second;
}

inline bool isExtraDim(const std::string& name)
{
    using namespace pdal;
    using D = Dimension::Id;

    static const std::array<Dimension::Id, 16> lasDims
    {
        D::X,
        D::Y,
        D::Z,
        D::Intensity,
        D::ReturnNumber,
        D::NumberOfReturns,
        D::ReturnNumber,
        D::Classification,
        D::UserData,
        D::ScanAngleRank,
        D::PointSourceId,
        D::GpsTime,
        D::Red,
        D::Green,
        D::Blue,
        D::Infrared
    };

    D id = Dimension::id(name);
    for (Dimension::Id lasId : lasDims)
        if (lasId == id)
            return false;
    return (name != UntwineBitsDimName);
}

// We check both _WIN32 and _MSC_VER to deal with MinGW, which doesn't support the special
// Windows wide character interfaces for streams.
#if defined(_WIN32) && defined(_MSC_VER)
inline std::wstring toNative(const std::string& in)
{
    if (in.empty())
        return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, 0, in.data(), in.length(), nullptr, 0);
    std::wstring out(len, 0);
    if (MultiByteToWideChar(CP_UTF8, 0, in.data(), in.length(), out.data(), len) == 0)
    {
        char buf[200] {};
        len = FormatMessageA(0, 0, GetLastError(), 0, buf, 199, 0);
        throw FatalError("Can't convert UTF8 to UTF16: " + std::string(buf, len));
    }
    return out;
}

inline std::string fromNative(const std::wstring& in)
{
    if (in.empty())
        return std::string();

    int len = WideCharToMultiByte(CP_UTF8, 0, in.data(), in.length(), nullptr,
        0, nullptr, nullptr);
    std::string out(len, 0);
    if (WideCharToMultiByte(CP_UTF8, 0, in.data(), in.length(), out.data(),
        len, nullptr, nullptr) == 0)
    {
        char buf[200] {};
        len = FormatMessageA(0, 0, GetLastError(), 0, buf, 199, 0);
        throw FatalError("Can't convert UTF16 to UTF8: " + std::string(buf, len));
    }
    return out;
}
#else
inline std::string toNative(const std::string& in)
{
    return in;
}

inline std::string fromNative(const std::string& in)
{
    return in;
}
#endif

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
MapContext mapFile(const std::string& filename, bool readOnly, size_t pos, size_t size);

/**
  Unmap a previously mapped file.
  \param ctx  Previously returned MapContext
  \return  MapContext indicating current state of the file mapping.
*/
MapContext unmapFile(MapContext ctx);

std::vector<std::string> directoryList(const std::string& dir);

} // namespace untwine
