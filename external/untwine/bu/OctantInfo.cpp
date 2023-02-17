/*****************************************************************************
 *   Copyright (c) 2021, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <fstream>
#include <vector>

#include "OctantInfo.hpp"
#include "../untwine/Common.hpp"

namespace untwine
{
namespace bu
{

void OctantInfo::mergeSmallFiles(const std::string tempDir, size_t pointSize)
{
    std::string baseFilename = key().toString() + "_merge.bin";
    std::string filename = tempDir + "/" + baseFilename;

    std::ofstream out(filename, std::ios::binary | std::ios::trunc);
    if (!out)
        throw FatalError("Couldn't open temporary merge file '" + filename + "'.");

    int totalPoints = 0;
    auto it = m_fileInfos.begin();
    while (it != m_fileInfos.end())
    {
        FileInfo& fi = *it;
        int numPoints = fi.numPoints();
        std::vector<char> buf(1500 * pointSize);
        if (numPoints < 1500)
        {
            size_t bytes = numPoints * pointSize;
            filename = tempDir + "/" + fi.filename();
            std::ifstream in(filename, std::ios::binary);
            if (!in)
                throw FatalError("Couldn't open file '" + filename + "' to merge.");
            in.read(buf.data(), bytes);
            out.write(buf.data(), bytes);
            totalPoints += numPoints;
            it = m_fileInfos.erase(it);
        }
        else
            it++;
    }
    // Stick a new file info for the merge file on the list.
    // If there were no file infos to merge, then don't add the file because we'll end up
    // with a 0-sized file that we try to map and that will blow up.
    if (totalPoints > 0)
        m_fileInfos.emplace_back(baseFilename, totalPoints);
}

} // namespace bu
} // namespace untwine
