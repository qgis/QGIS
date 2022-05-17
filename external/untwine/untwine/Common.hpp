#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdint.h>
#include <array>
#include <string>
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
    bool singleFile;
    StringList inputFiles;
    std::string tempDir;
    bool preserveTempDir;
    bool doCube;
    size_t fileLimit;
    int level;
    int progressFd;
    bool progressDebug;
    StringList dimNames;
    bool stats;
    std::string a_srs;
    bool metadata;
};

struct BaseInfo
{
public:
    BaseInfo()
    {};

    Options opts;
    pdal::BOX3D bounds;
    pdal::BOX3D trueBounds;
    size_t pointSize;
    std::string outputFile;
    DimInfoList dimInfo;
    pdal::SpatialReference srs;
    int pointFormatId;

    using d3 = std::array<double, 3>;
    d3 scale { -1.0, -1.0, -1.0 };
    d3 offset {};
};

const std::string MetadataFilename {"info2.txt"};

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

    int len = WideCharToMultiByte(CP_UTF8, 0, in.data(), in.length(), nullptr, 0, nullptr, nullptr);
    std::string out(len, 0);
    if (WideCharToMultiByte(CP_UTF8, 0, in.data(), in.length(), out.data(), len, nullptr, nullptr) == 0)
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

} // namespace untwine
