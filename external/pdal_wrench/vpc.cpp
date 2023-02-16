/*****************************************************************************
 *   Copyright (c) 2023, Lutra Consulting Ltd. and Hobu, Inc.                *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include "vpc.hpp"
#include "utils.hpp"

#include <pdal/util/ProgramArgs.hpp>

#include "nlohmann/json.hpp"


using json = nlohmann::json;

using namespace pdal;

// TODO:
// - optionally support absolute paths (with a flag to build_vpc?)


void VirtualPointCloud::clear()
{
    files.clear();
}

void VirtualPointCloud::dump()
{
    std::cout << "----- VPC" << std::endl;
    for (auto& f : files)
    {
        std::cout << " - " << f.filename << "  " << f.count << "  "
                    << f.bbox.minx << " " << f.bbox.miny << " " << f.bbox.maxx << " " << f.bbox.maxy << std::endl;
    }
}

bool VirtualPointCloud::read(std::string filename)
{
    clear();

    std::ifstream inputJson(filename);
    if (!inputJson.good())
    {
        std::cout << "failed to read file " << filename << std::endl;
        return false;
    }

    fs::path filenameParent = fs::path(filename).parent_path();

    json data;
    try
    {
        data = json::parse(inputJson);
    }
    catch (std::exception &e)
    {
        std::cout << "json parsing error: " << e.what() << std::endl;
        return false;
    }
    if (!data.contains("vpc"))
    {
        std::cout << "not a VPC file " << filename << std::endl;
        return false;
    }
    if (data["vpc"] != "1.0.0")
    {
        std::cout << "unsupported VPC file version " << data["vpc"] << std::endl;
        return false;
    }

    crsWkt = data["metadata"]["crs"];

    for (auto& f : data["files"])
    {
        File vpcf;
        vpcf.filename = f["filename"];
        vpcf.count = f["count"];
        json jb = f["bbox"];
        vpcf.bbox = BOX3D(
            jb[0].get<double>(), jb[1].get<double>(), jb[2].get<double>(),
            jb[3].get<double>(),jb[4].get<double>(), jb[5].get<double>() );

        if (vpcf.filename.substr(0, 2) == "./")
        {
            // resolve relative path
            vpcf.filename = fs::weakly_canonical(filenameParent / vpcf.filename).string();
        }

        files.push_back(vpcf);
    }

    return true;
}

bool VirtualPointCloud::write(std::string filename)
{
    std::ofstream outputJson(filename);
    if (!outputJson.good())
    {
        std::cout << "failed to create file" << std::endl;
        return false;
    }

    fs::path outputPath = fs::path(filename).parent_path();

    std::vector<nlohmann::ordered_json> jFiles;
    for ( const File &f : files )
    {
        fs::path fRelative = fs::relative(f.filename, outputPath);

        jFiles.push_back({
            { "filename", "./" + fRelative.string() },
            { "count", f.count },
            { "bbox", { f.bbox.minx, f.bbox.miny, f.bbox.minz, f.bbox.maxx, f.bbox.maxy, f.bbox.maxz } },
        });
    }

    nlohmann::ordered_json jMeta = {
      { "crs", crsWkt },
    };

    nlohmann::ordered_json j = { { "vpc", "1.0.0" }, { "metadata", jMeta }, { "files", jFiles } };

    outputJson << std::setw(2) << j << std::endl;
    outputJson.close();
    return true;
}


void buildVpc(std::vector<std::string> args)
{
    std::string outputFile;
    std::vector<std::string> inputFiles;

    ProgramArgs programArgs;
    programArgs.add("output,o", "Output virtual point cloud file", outputFile);
    programArgs.add("files,f", "input files", inputFiles).setPositional();

    try
    {
        programArgs.parseSimple(args);
    }
    catch(pdal::arg_error err)
    {
        std::cerr << "failed to parse arguments: " << err.what() << std::endl;
        return;
    }

    std::cout << "input " << inputFiles.size() << std::endl;
    std::cout << "output " << outputFile << std::endl;

    if (inputFiles.empty())
    {
      std::cerr << "No input files!" << std::endl;
      return;
    }

    // TODO: would be nice to support input directories too (recursive)

    VirtualPointCloud vpc;
    std::set<std::string> vpcCrsWkt;

    for (const std::string &inputFile : inputFiles)
    {
        MetadataNode n = getReaderMetadata(inputFile);
        point_count_t cnt = n.findChild("count").value<point_count_t>();
        BOX3D bbox(
                n.findChild("minx").value<double>(),
                n.findChild("miny").value<double>(),
                n.findChild("minz").value<double>(),
                n.findChild("maxx").value<double>(),
                n.findChild("maxy").value<double>(),
                n.findChild("maxz").value<double>()
        );

        std::string crsWkt = n.findChild("srs").findChild("compoundwkt").value();
        vpcCrsWkt.insert(crsWkt);

        VirtualPointCloud::File f;
        f.filename = inputFile;
        f.count = cnt;
        f.bbox = bbox;
        vpc.files.push_back(f);
    }

    if (vpcCrsWkt.size() == 1)
    {
        vpc.crsWkt = *vpcCrsWkt.begin();
    }
    else
    {
        std::cout << "found a mixture of multiple CRS in input files (" << vpcCrsWkt.size() << ")" << std::endl;
        vpc.crsWkt = "_mix_";
    }

    vpc.dump();

    vpc.write(outputFile);

    vpc.read(outputFile);

    // TODO: for now hoping that all files have the same file type + CRS + point format + scaling
    // "dataformat_id"
    // "spatialreference"
    // "scale_x" ...


    //Utils::toJSON(n, std::cout);

}

point_count_t VirtualPointCloud::totalPoints() const
{
    point_count_t cnt = 0;
    for (const File &f : files)
        cnt += f.count;
    return cnt;
}

BOX3D VirtualPointCloud::box3d() const
{
    if (files.empty())
        return BOX3D();
    BOX3D b = files[0].bbox;
    for (const File &f : files)
        b.grow(f.bbox);
    return b;
}

// compared to BOX2D::overlaps(), this one excludes the
// maxx/maxy coords from the box "a" - so it returns false
// when "b" touches "a" at the top or right side.
// this avoids including files from neighboring tiles.
inline bool overlaps2(const BOX2D &a, const BOX2D &b)
{
    return a.minx <= b.maxx && a.maxx > b.minx &&
            a.miny <= b.maxy && a.maxy > b.miny;
}

std::vector<VirtualPointCloud::File> VirtualPointCloud::overlappingBox2D(const BOX2D &box)
{
    std::vector<VirtualPointCloud::File> overlaps;
    for (const File &f : files)
    {
        if (overlaps2(box, f.bbox.to2d()))
            overlaps.push_back(f);
    }
    return overlaps;
}
