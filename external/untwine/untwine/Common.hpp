#pragma once

#include <stdint.h>
#include <array>
#include <string>
#include <vector>

#include <pdal/SpatialReference.hpp>
#include <pdal/util/Bounds.hpp>

#include "FileDimInfo.hpp"

namespace untwine
{

using PointCount = uint64_t;
using StringList = std::vector<std::string>;

void fatal(const std::string& err);

struct Options
{
    std::string outputDir;
    StringList inputFiles;
    std::string tempDir;
    bool doCube;
    size_t fileLimit;
    int level;
    int progressFd;
    StringList dimNames;
    bool stats;
};

struct BaseInfo
{
    pdal::BOX3D bounds;
    pdal::BOX3D trueBounds;
    size_t pointSize;
    std::string inputDir;
    std::string outputDir;
    int maxLevel;
    DimInfoList dimInfo;
    pdal::SpatialReference srs;
    bool stats;

    using d3 = std::array<double, 3>;
    d3 scale { -1.0, -1.0, -1.0 };
    d3 offset {};
};

const std::string MetadataFilename {"info2.txt"};

} // namespace untwine
