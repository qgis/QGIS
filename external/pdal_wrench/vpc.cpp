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

#include <pdal/Polygon.hpp>


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
        std::cerr << "Failed to read input VPC file: " << filename << std::endl;
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
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }

    if (data["type"] != "FeatureCollection")
    {
        std::cerr << "The input file is not a VPC file: " << filename << std::endl;
        return false;
    }
    if (!data.contains("features"))
    {
        std::cerr << "Missing 'features' key in a VPC file" << std::endl;
        return false;
    }

    std::set<std::string> vpcCrsWkt;

    for (auto& f : data["features"])
    {
        if (!f.contains("type") || f["type"] != "Feature" ||
            !f.contains("stac_version") ||
            !f.contains("assets") || !f["assets"].is_object() ||
            !f.contains("properties") || !f["properties"].is_object())
        {
            std::cerr << "Malformed STAC item: " << f << std::endl;
            continue;
        }

        if (f["stac_version"] != "1.0.0")
        {
            std::cerr << "Unsupported STAC version: " << f["stac_version"] << std::endl;
            continue;
        }

        nlohmann::json firstAsset = *f["assets"].begin();

        File vpcf;
        vpcf.filename = firstAsset["href"];
        vpcf.count = f["properties"]["pc:count"];
        vpcf.crsWkt = f["properties"]["proj:wkt2"];
        vpcCrsWkt.insert(vpcf.crsWkt);

        nlohmann::json nativeBbox = f["properties"]["proj:bbox"];
        vpcf.bbox = BOX3D(
            nativeBbox[0].get<double>(), nativeBbox[1].get<double>(), nativeBbox[2].get<double>(),
            nativeBbox[3].get<double>(), nativeBbox[4].get<double>(), nativeBbox[5].get<double>() );

        if (vpcf.filename.substr(0, 2) == "./")
        {
            // resolve relative path
            vpcf.filename = fs::weakly_canonical(filenameParent / vpcf.filename).string();
        }

        for (auto &schemaItem : f["properties"]["pc:schemas"])
        {
            vpcf.schema.push_back(VirtualPointCloud::SchemaItem(schemaItem["name"], schemaItem["type"], schemaItem["size"].get<int>()));
        }

        files.push_back(vpcf);
    }

    if (vpcCrsWkt.size() == 1)
    {
        crsWkt = *vpcCrsWkt.begin();
    }
    else
    {
        std::cerr << "found a mixture of multiple CRS in input files (" << vpcCrsWkt.size() << ")" << std::endl;
        crsWkt = "_mix_";
    }

    return true;
}

bool VirtualPointCloud::write(std::string filename)
{
    std::ofstream outputJson(filename);
    if (!outputJson.good())
    {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return false;
    }

    fs::path outputPath = fs::path(filename).parent_path();

    std::vector<nlohmann::ordered_json> jFiles;
    for ( const File &f : files )
    {
        std::string assetFilename;
        if (Utils::startsWith(f.filename, "http://") || Utils::startsWith(f.filename, "https://"))
        {
            // keep remote URLs as they are
            assetFilename = f.filename;
        }
        else
        {
            // turn local paths to relative
            fs::path fRelative = fs::relative(f.filename, outputPath);
            assetFilename = "./" + fRelative.string();
        }
        std::string fileId = fs::path(f.filename).stem().string();  // TODO: we should make sure the ID is unique

        // bounding box in WGS 84: reproject if possible, or keep it as is
        BOX3D bboxWgs84 = f.bbox;
        if (!f.crsWkt.empty())
        {
            pdal::Polygon p(f.bbox);
            p.setSpatialReference(pdal::SpatialReference(f.crsWkt));
            if (p.transform("EPSG:4326"))
            {
                bboxWgs84 = p.bounds();
            }
        }

        std::vector<nlohmann::json> schemas;
        for ( auto &si : f.schema )
        {
            schemas.push_back(nlohmann::json{
              { "name", si.name },
              { "type", si.type },
              { "size", si.size },
            });
        }

        nlohmann::json props = {
          // Acquisition time: readers.las and readers.copc provide "creation_year" and "creation_doy"
          // metadata - they are not always valid, but that's not really our problem...
          // Alternatively if there is no single datetime, STAC defines that "start_datetime" and "end_datetime"
          // may be used when the acquisition was done in a longer time period...
          { "datetime", f.datetime },

          // required pointcloud extension properties
          { "pc:count", f.count },
          { "pc:type", "lidar" },   // TODO: how could we know?
          { "pc:encoding", "?" },   // TODO: what value to use?
          { "pc:schemas", schemas },

          // TODO: write pc:statistics if we have it (optional)

          // projection extension properties (none are required)
          { "proj:wkt2", f.crsWkt },
          { "proj:bbox", { f.bbox.minx, f.bbox.miny, f.bbox.minz, f.bbox.maxx, f.bbox.maxy, f.bbox.maxz } },
          // TODO: add "proj:geometry" if we know exact boundary
        };
        nlohmann::json links = json::array();

        nlohmann::json asset = {
            { "href", assetFilename },
        };

        jFiles.push_back(
        {
            { "type", "Feature" },
            { "stac_version", "1.0.0" },
            { "stac_extensions",
                {
                  "https://stac-extensions.github.io/pointcloud/v1.0.0/schema.json",
                  "https://stac-extensions.github.io/projection/v1.1.0/schema.json"
                }
            },
            { "id", fileId },
            { "geometry", nullptr },  // TODO: use WGS 84 (multi-)polygon if we have exact boundary
            { "bbox", { bboxWgs84.minx, bboxWgs84.miny, bboxWgs84.minz, bboxWgs84.maxx, bboxWgs84.maxy, bboxWgs84.maxz } },
            { "properties", props },
            { "links", links },
            { "assets", { { "data", asset } } },

        });

    }

    nlohmann::ordered_json j = { { "type", "FeatureCollection" }, { "features", jFiles } };

    outputJson << std::setw(2) << j << std::endl;
    outputJson.close();
    return true;
}


bool isLeapYear(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    if (year % 4 == 0) return true;
    return false;
}


// https://www.rfc-editor.org/rfc/rfc3339#section-5.6
//  e.g. 2023-01-01T12:00:00Z
std::string dateTimeStringFromYearAndDay(int year, int dayOfYear)
{
    bool leapYear = isLeapYear(year);

    if (year < 0) return "";  // Year is negative
    if ((dayOfYear < 1) || (dayOfYear > (leapYear ? 366 : 365))) return ""; // Day of year is out of range

    // Figure out month and day of month, from day of year.
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (leapYear) daysInMonth[1]++;

    int month = 0;
    int daysLeft = dayOfYear;
    while (daysLeft > daysInMonth[month])
        daysLeft -= daysInMonth[month++];
    ++month;

    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = daysLeft;

    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", &tm);
    return std::string(timeString);
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

//    std::cout << "input " << inputFiles.size() << std::endl;
//    std::cout << "output " << outputFile << std::endl;

    if (inputFiles.empty())
    {
      std::cerr << "No input files!" << std::endl;
      return;
    }

    // TODO: would be nice to support input directories too (recursive)

    VirtualPointCloud vpc;

    for (const std::string &inputFile : inputFiles)
    {
        MetadataNode layout;
        MetadataNode n = getReaderMetadata(inputFile, &layout);
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

        int dayOfYear = n.findChild("creation_doy").value<int>();
        int year = n.findChild("creation_year").value<int>();

        VirtualPointCloud::File f;
        f.filename = inputFile;
        f.count = cnt;
        f.bbox = bbox;
        f.crsWkt = crsWkt;
        f.datetime = dateTimeStringFromYearAndDay(year, dayOfYear);

        for (auto &dim : layout.children("dimensions"))
        {
            f.schema.push_back(VirtualPointCloud::SchemaItem(
                  dim.findChild("name").value(),
                  dim.findChild("type").value(),
                  dim.findChild("size").value<int>()));
        }

        vpc.files.push_back(f);
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
