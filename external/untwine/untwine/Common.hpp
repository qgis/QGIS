#pragma once

#include <string>

namespace untwine
{

void fatal(const std::string& err);

struct Options
{
    std::string outputDir;
    pdal::StringList inputFiles;
    std::string tempDir;
    bool doCube;
    size_t fileLimit;
    int level;
    int progressFd;
};

const std::string MetadataFilename {"info2.txt"};

} // namespace untwine
