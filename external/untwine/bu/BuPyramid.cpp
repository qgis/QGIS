#include <iomanip>
#include <regex>
#include <set>
#include <string>
#include <vector>
#include <filesystem>

#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include "BuPyramid.hpp"
#include "FileInfo.hpp"
#include "OctantInfo.hpp"
#include "../untwine/Common.hpp"
#include "../untwine/ProgressWriter.hpp"

#include <dirlist.hpp>  //untwine/os

namespace untwine
{
namespace bu
{

/// BuPyramid

BuPyramid::BuPyramid(BaseInfo& common) : m_b(common), m_manager(m_b)
{}


void BuPyramid::run(ProgressWriter& progress)
{
    getInputFiles();
    size_t count = queueWork();
    if (!count)
        throw FatalError("No temporary files to process. I/O or directory list error?");

    progress.setPercent(.6);
    progress.setIncrement(.4 / count);
    m_manager.setProgress(&progress);
    m_manager.run();
}

void BuPyramid::getInputFiles()
{
    auto matches = [](const std::string& f)
    {
        std::regex check("([0-9]+)-([0-9]+)-([0-9]+)-([0-9]+)\\.bin");
        std::smatch m;
        if (!std::regex_match(f, m, check))
            return std::make_pair(false, VoxelKey(0, 0, 0, 0));
        int level = std::stoi(m[1].str());
        int x = std::stoi(m[2].str());
        int y = std::stoi(m[3].str());
        int z = std::stoi(m[4].str());
        return std::make_pair(true, VoxelKey(x, y, z, level));
    };

    std::vector<std::string> files = os::directoryList(m_b.opts.tempDir);

    VoxelKey root;
    for (std::string file : files)
    {
        uintmax_t size = pdal::FileUtils::fileSize(file);
        file = pdal::FileUtils::getFilename(file);
        auto ret = matches(file);
        bool success = ret.first;
        VoxelKey& key = ret.second;
        if (success)
            m_allFiles.emplace(key, FileInfo(file, size / m_b.pointSize));
    }

    // Remove any files that are hangers-on from a previous run - those that are parents
    // of other input.
    for (auto it = m_allFiles.begin(); it != m_allFiles.end(); ++it)
    {
        VoxelKey k = it->first;
        while (k != root)
        {
            k = k.parent();
            m_allFiles.erase(k);
        };
    }
}


size_t BuPyramid::queueWork()
{
    std::set<VoxelKey> needed;
    std::set<VoxelKey> parentsToProcess;
    std::vector<OctantInfo> have;
    const VoxelKey root;

    for (auto& afi : m_allFiles)
    {
        VoxelKey k = afi.first;
        FileInfo& f = afi.second;

        // Stick an OctantInfo for this file in the 'have' list.
        OctantInfo o(k);
        o.appendFileInfo(f);
        have.push_back(o);

        // Walk up the tree and make sure that we're populated for all children necessary
        // to process to the top level.
        while (k != root)
        {
            k = k.parent();
            parentsToProcess.insert(k);
            for (int i = 0; i < 8; ++i)
                needed.insert(k.child(i));
        }
    }

    // Now remove entries for the files we have and their parents.
    for (const OctantInfo& o : have)
    {
        VoxelKey k = o.key();
        while (k != root)
        {
            needed.erase(k);
            k = k.parent();
        }
    }

    // Queue what we have and what's left that's needed.
    for (const OctantInfo& o : have)
        m_manager.queue(o);
    for (const VoxelKey& k : needed)
        m_manager.queue(OctantInfo(k));
    return parentsToProcess.size();
}

} // namespace bu
} // namespace untwine
