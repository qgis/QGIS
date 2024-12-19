#pragma once

#include <filesystem>

#include <stringconv.hpp>

namespace untwine
{
namespace os
{

// PDAL's directoryList had a bug, so we've imported a working
// version here so that we can still use older PDAL releases.

inline std::vector<std::string> directoryList(const std::string& dir)
{
    namespace fs = std::filesystem;

    std::vector<std::string> files;

    try
    {
        fs::directory_iterator it(toNative(dir));
        fs::directory_iterator end;
        while (it != end)
        {
            files.push_back(fromNative(it->path()));
            it++;
        }
    }
    catch (fs::filesystem_error&)
    {
        files.clear();
    }
    return files;
}

} // namespace os
} // namespace untwine
