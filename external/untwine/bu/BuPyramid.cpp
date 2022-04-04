#include <iomanip>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include "BuPyramid.hpp"
#include "FileInfo.hpp"
#include "OctantInfo.hpp"
#include "../untwine/Common.hpp"
#include "../untwine/ProgressWriter.hpp"

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

    progress.setPercent(.6);
    progress.setIncrement(.4 / count);
    m_manager.setProgress(&progress);
    //ABELL - Not sure why this was being run in a separate thread. The current thread
    // would block in join() anyway.
    /**
    std::thread runner(&PyramidManager::run, &m_manager);
    runner.join();
    **/
    m_manager.run();
    if (!m_b.opts.singleFile)
        writeInfo();
}


void BuPyramid::writeInfo()
{
    auto typeString = [](pdal::Dimension::BaseType b)
    {
        using namespace pdal::Dimension;

        switch (b)
        {
        case BaseType::Signed:
            return "signed";
        case BaseType::Unsigned:
            return "unsigned";
        case BaseType::Floating:
            return "float";
        default:
            return "";
        }
    };

    std::ofstream out(m_b.opts.outputName + "/ept.json");
    int maxdigits = std::numeric_limits<double>::max_digits10;

    out << "{\n";

    pdal::BOX3D& b = m_b.bounds;

    // Set fixed output for bounds output to get sufficient precision.
    out << std::fixed << std::setprecision(maxdigits);
    out << "\"bounds\": [" <<
        b.minx << ", " << b.miny << ", " << b.minz << ", " <<
        b.maxx << ", " << b.maxy << ", " << b.maxz << "],\n";

    pdal::BOX3D& tb = m_b.trueBounds;
    out << "\"boundsConforming\": [" <<
        tb.minx << ", " << tb.miny << ", " << tb.minz << ", " <<
        tb.maxx << ", " << tb.maxy << ", " << tb.maxz << "],\n";
    // Reset to default float output to match PDAL option handling for now.
    out << std::defaultfloat;

    out << "\"dataType\": \"laszip\",\n";
    out << "\"hierarchyType\": \"json\",\n";
    out << "\"points\": " << m_manager.totalPoints() << ",\n";
    out << "\"span\": 128,\n";
    out << "\"version\": \"1.0.0\",\n";
    out << "\"schema\": [\n";
    for (auto di = m_b.dimInfo.begin(); di != m_b.dimInfo.end(); ++di)
    {
        const FileDimInfo& fdi = *di;

        out << "\t{";
            out << "\"name\": \"" << fdi.name << "\", ";
            out << "\"type\": \"" << typeString(pdal::Dimension::base(fdi.type)) << "\", ";
            out << std::fixed << std::setprecision(maxdigits);
            if (fdi.name == "X")
                out << "\"scale\": " << m_b.scale[0] << ", \"offset\": " << m_b.offset[0] << ", ";
            if (fdi.name == "Y")
                out << "\"scale\": " << m_b.scale[1] << ", \"offset\": " << m_b.offset[1] << ", ";
            if (fdi.name == "Z")
                out << "\"scale\": " << m_b.scale[2] << ", \"offset\": " << m_b.offset[2] << ", ";
            out << std::defaultfloat;
            out << "\"size\": " << pdal::Dimension::size(fdi.type);
            const Stats *stats = m_manager.stats(fdi.dim);
            if (stats)
            {
                const Stats::EnumMap& v = stats->values();
                out << ", ";
                if (v.size())
                {
                    out << "\"counts\": [ ";
                    for (auto ci = v.begin(); ci != v.end(); ++ci)
                    {
                        auto c = *ci;
                        if (ci != v.begin())
                            out << ", ";
                        out << "{\"value\": " << c.first << ", \"count\": " << c.second << "}";
                    }
                    out << "], ";
                }
                out << "\"count\": " << m_manager.totalPoints() << ", ";
                out << std::fixed << std::setprecision(maxdigits);
                out << "\"maximum\": " << stats->maximum() << ", ";
                out << "\"minimum\": " << stats->minimum() << ", ";
                out << "\"mean\": " << stats->average() << ", ";
                out << "\"stddev\": " << stats->stddev() << ", ";
                out << "\"variance\": " << stats->variance();
                out << std::defaultfloat;
            }
        out << "}";
        if (di + 1 != m_b.dimInfo.end())
            out << ",";
        out << "\n";
    }
    out << "],\n";

    out << "\"srs\": {\n";
    if (m_b.srs.valid())
    {
        out << "\"wkt\": " <<  "\"" << pdal::Utils::escapeJSON(m_b.srs.getWKT()) << "\"\n";
    }
    out << "}\n";

    out << "}\n";
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

    std::vector<std::string> files = pdal::FileUtils::directoryList(m_b.opts.tempDir);

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
