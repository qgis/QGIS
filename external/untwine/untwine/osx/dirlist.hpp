#pragma once

#include <dirent.h>

#include <stringconv.hpp>

namespace untwine
{
namespace os
{

// Provide simple opendir/readdir solution for OSX because directory_iterator is
// not available until OSX 10.15
inline std::vector<std::string> directoryList(const std::string& dir)
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

            std::string name = fromNative(epdf->d_name);
            // Skip paths
            if (!pdal::Utils::iequals(name, ".") &&
                !pdal::Utils::iequals(name, ".."))
            {
                // we expect the path + name
                files.push_back(dir + "/" + fromNative(epdf->d_name));
            }
        }
        closedir(dpdf);
    }
    return files;
}

} // namespace os
} // namespace untwine
