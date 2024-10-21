#pragma once

#include <stdint.h>
#include <array>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include "FatalError.hpp"
#include "FileDimInfo.hpp"

namespace untwine
{

// Number of cells into which points are put for each octree voxel.
const int CellCount = 128;

using PointCount = uint64_t;
using StringList = std::vector<std::string>;

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
    bool no_srs;
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

    static const std::array<Dimension::Id, 15> lasDims
    {
        D::X,
        D::Y,
        D::Z,
        D::Intensity,
        D::ReturnNumber,
        D::NumberOfReturns,
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

} // namespace untwine
