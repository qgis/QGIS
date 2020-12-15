#pragma once

#include <stdint.h>
#include <string>
#include <vector>

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
};

const std::string MetadataFilename {"info2.txt"};

} // namespace untwine
