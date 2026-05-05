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

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
namespace fs = std::filesystem;

#include "vpc.hpp"
#include "utils.hpp"

#include <pdal/Polygon.hpp>
#include <pdal/Stage.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include "nlohmann/json.hpp"
#include <zip.h>


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

    fs::path filenameParent = fs::path(filename).parent_path();

    json data;

    if (ends_with(filename, ".vpz"))
    {
        int err = 0;
        zip_t *za = zip_open(filename.c_str(), ZIP_RDONLY, &err);
        if (!za)
        {
            std::cerr << "Failed to open VPZ file: " << filename << std::endl;
            return false;
        }

        // find the single .vpc entry
        const zip_int64_t numEntries = zip_get_num_entries(za, 0);
        zip_int64_t vpcIndex = -1;
        for (zip_int64_t i = 0; i < numEntries; ++i)
        {
            const char *name = zip_get_name(za, i, 0);
            if (name && ends_with(std::string(name), ".vpc"))
            {
                if (vpcIndex != -1)
                {
                    std::cerr << "VPZ file contains more than one .vpc entry: " << filename << std::endl;
                    zip_close(za);
                    return false;
                }
                vpcIndex = i;
            }
        }
        if (vpcIndex == -1)
        {
            std::cerr << "VPZ file contains no .vpc entry: " << filename << std::endl;
            zip_close(za);
            return false;
        }

        zip_stat_t st;
        zip_stat_index(za, vpcIndex, 0, &st);
        zip_file_t *zf = zip_fopen_index(za, vpcIndex, 0);
        if (!zf)
        {
            std::cerr << "Failed to open .vpc entry inside VPZ: " << filename << std::endl;
            zip_close(za);
            return false;
        }

        std::string content(st.size, '\0');
        zip_fread(zf, &content[0], st.size);
        zip_fclose(zf);
        zip_close(za);

        try
        {
            data = json::parse(content);
        }
        catch (std::exception &e)
        {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }
    else
    {
        std::ifstream inputJson(filename);
        if (!inputJson.good())
        {
            std::cerr << "Failed to read input VPC file: " << filename << std::endl;
            return false;
        }

        try
        {
            data = json::parse(inputJson);
        }
        catch (std::exception &e)
        {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
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

    try
    {
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

            // read boundary geometry
            nlohmann::json nativeGeometry = f["properties"]["proj:geometry"];
            std::stringstream sstream;
            sstream << std::setw(2) << nativeGeometry << std::endl;
            std::string wkt = sstream.str();
            pdal::Geometry nativeGeom(sstream.str());
            vpcf.boundaryWkt = nativeGeom.wkt();

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

            // read stats
            for (auto &statsItem : f["properties"]["pc:statistics"])
            {
                vpcf.stats.push_back(VirtualPointCloud::StatsItem(
                                        statsItem["name"],
                                        statsItem["position"],
                                        statsItem["average"],
                                        statsItem["count"],
                                        statsItem["maximum"],
                                        statsItem["minimum"],
                                        statsItem["stddev"],
                                        statsItem["variance"]));
            }

            // read overview files (assets with "overview" role)
            for (auto &asset : f["assets"])
            {
                if (!asset.contains("roles") || !asset["roles"].is_array())
                    continue;

                const auto roles = asset["roles"];
                if (std::find(roles.cbegin(), roles.cend(), "overview") == roles.cend())
                    continue;

                std::string ovFilename = asset["href"];
                if (ovFilename.substr(0, 2) == "./")
                    ovFilename = fs::weakly_canonical(filenameParent / ovFilename).string();
                vpcf.overviewFilenames.push_back(ovFilename);
            }

            files.push_back(vpcf);
        }
    }
    catch ( nlohmann::detail::invalid_iterator& e )
    {
        std::cerr << "Invalid 'features' value in a VPC file: " << e.what() << std::endl;
        return false;
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

void geometryToJson(const Geometry &geom, const BOX3D &bbox, nlohmann::json &jsonGeometry, nlohmann::json &jsonBbox)
{
    jsonBbox = { bbox.minx, bbox.miny, bbox.minz, bbox.maxx, bbox.maxy, bbox.maxz };

    std::string strGeom = geom.json();
    try
    {
        jsonGeometry = json::parse(strGeom);
    }
    catch (std::exception &e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

bool VirtualPointCloud::write(std::string filename)
{
    if (!isVpcFilename(filename))
        filename += ".vpz";

    std::string filenameAbsolute = filename;
    if (!fs::path(filename).is_absolute())
    {
        filenameAbsolute = fs::absolute(filename).string();
    }

    fs::path outputPath = fs::path(filenameAbsolute).parent_path();

    std::vector<nlohmann::ordered_json> jFiles;
    for ( const File &f : files )
    {
        std::string assetFilename;
        if (pdal::Utils::isRemote(f.filename))
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

        pdal::Geometry boundary = !f.boundaryWkt.empty() ? pdal::Geometry(f.boundaryWkt) : pdal::Polygon(f.bbox);

        // use bounding box as the geometry
        nlohmann::json jsonGeometry, jsonBbox;
        geometryToJson(boundary, f.bbox, jsonGeometry, jsonBbox);

        // bounding box in WGS 84: reproject if possible, or keep it as is
        nlohmann::json jsonGeometryWgs84 = jsonGeometry, jsonBboxWgs84 = jsonBbox;
        if (!f.crsWkt.empty())
        {
            pdal::Geometry boundaryWgs84 = boundary;
            boundaryWgs84.setSpatialReference(pdal::SpatialReference(f.crsWkt));
            if (boundaryWgs84.transform("EPSG:4326"))
            {
                geometryToJson(boundaryWgs84, boundaryWgs84.bounds(), jsonGeometryWgs84, jsonBboxWgs84);
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
          { "pc:encoding", "?" },   // TODO: https://github.com/stac-extensions/pointcloud/issues/6
          { "pc:schemas", schemas },

          // projection extension properties (none are required)
          { "proj:wkt2", f.crsWkt },
          { "proj:geometry", jsonGeometry },
          { "proj:bbox", jsonBbox },
        };

        if (!f.stats.empty())
        {
            nlohmann::json statsArray = json::array();
            for (const VirtualPointCloud::StatsItem &s : f.stats)
            {
                nlohmann::json stat = {
                    { "name", s.name },
                    { "position", s.position },
                    { "average", s.average },
                    { "count", s.count },
                    { "maximum", s.maximum },
                    { "minimum", s.minimum },
                    { "stddev", s.stddev },
                    { "variance", s.variance },
                };
                statsArray.push_back(stat);
            }
            props["pc:statistics"] = statsArray;
        }

        nlohmann::json links = json::array();

        nlohmann::json dataAsset = {
            { "href", assetFilename },
            { "roles", json::array({"data"}) },
        };
        nlohmann::json assets = { { "data", dataAsset } };

        for (size_t i = 0; i < f.overviewFilenames.size(); ++i)
        {
            std::string ovFilename(f.overviewFilenames[i]);
            if (!pdal::Utils::isRemote(ovFilename))
            {
                const fs::path fRelative = fs::relative(ovFilename, outputPath);
                ovFilename = "./" + fRelative.string();
            }
            const std::string key = f.overviewFilenames.size() > 1 ? ("overview-" + std::to_string(i + 1)) : "overview";
            nlohmann::json overviewAsset = {
                { "href", ovFilename },
                { "roles", json::array({"overview"}) },
            };
            assets[key] = overviewAsset;
        }

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
            { "geometry", jsonGeometryWgs84 },
            { "bbox", jsonBboxWgs84 },
            { "properties", props },
            { "links", links },
            { "assets", assets },

        });

    }

    nlohmann::ordered_json j = { { "type", "FeatureCollection" }, { "features", jFiles } };

    if (ends_with(filenameAbsolute, ".vpz"))
    {
        const std::string content = j.dump() + "\n";
        const std::string entryName = fs::path(filenameAbsolute).stem().string() + ".vpc";

        int err = 0;
        zip_t *za = zip_open(filenameAbsolute.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &err);
        if (!za)
        {
            std::cerr << "Failed to create VPZ file: " << filenameAbsolute << std::endl;
            return false;
        }

        zip_source_t *source = zip_source_buffer(za, content.c_str(), content.size(), 0);
        if (!source)
        {
            std::cerr << "Failed to create zip source buffer" << std::endl;
            zip_discard(za);
            return false;
        }

        if (zip_file_add(za, entryName.c_str(), source, ZIP_FL_OVERWRITE) < 0)
        {
            std::cerr << "Failed to add .vpc entry to VPZ: " << zip_strerror(za) << std::endl;
            zip_source_free(source);
            zip_discard(za);
            return false;
        }

        if (zip_close(za) != 0)
        {
            std::cerr << "Failed to write VPZ file: " << filenameAbsolute << std::endl;
            return false;
        }
    }
    else
    {
        std::ofstream outputJson(filenameAbsolute);
        if (!outputJson.good())
        {
            std::cerr << "Failed to create file: " << filenameAbsolute << std::endl;
            return false;
        }

        outputJson << j << std::endl;
        outputJson.close();
    }
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

    if (year < 0)  // Year is negative
    {
        std::cerr << "Warning: year(" << year <<
            ") is not valid. Defualting to 1970." << std::endl;
        year = 1970;
    }
    if ((dayOfYear < 1) || (dayOfYear > (leapYear ? 366 : 365)))
    {
        std::cerr << "Warning: DayOfYear(" << year <<
            ") is out of range. Defualting to 1." << std::endl;
        dayOfYear = 1;
    }

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
    std::string inputFileList;
    std::vector<std::string> inputFiles;
    bool boundaries = false;
    bool stats = false;
    bool overview = false;
    double overviewLength = 0.0;
    int max_threads = -1;
    bool verbose = false;
    bool help = false;

    ProgramArgs programArgs;
    programArgs.add("help,h", "Output command help.", help);
    programArgs.add("output,o", "Output virtual point cloud file", outputFile);
    programArgs.add("files,f", "input files", inputFiles).setPositional();
    programArgs.add("input-file-list", "Read input files from a txt file, one file per line.", inputFileList);
    programArgs.add("boundary", "Calculate boundary polygons from data", boundaries);
    programArgs.add("stats", "Calculate statistics from data", stats);
    programArgs.add("overview", "Create overview point cloud from source data", overview);
    programArgs.add("overview-length",
        "Split overview into multiple tiles of specified maximum edge length in CRS units (implies --overview)",
        overviewLength);

    pdal::Arg& argThreads = programArgs.add("threads", "Max number of concurrent threads for parallel runs", max_threads);
    programArgs.add("verbose", "Print extra debugging output", verbose);

    try
    {
        programArgs.parseSimple(args);
    }
    catch(pdal::arg_error err)
    {
        std::cerr << "failed to parse arguments: " << err.what() << std::endl;
        return;
    }

    if (help)
    {

        std::cout << "usage: pdal_wrench build_vpc [<args>]" << std::endl;
        programArgs.dump(std::cerr, 2, Utils::screenWidth());
        return;
    }

    if (!inputFileList.empty())
    {
        std::ifstream inputFile(inputFileList);
        std::string line;

        if(!inputFile)
        {
            std::cerr << "failed to open input file list: " << inputFileList << std::endl;
            return;
        }

        while (std::getline(inputFile, line))
        {
            inputFiles.push_back(line);
        }

    }

//    std::cout << "input " << inputFiles.size() << std::endl;
//    std::cout << "output " << outputFile << std::endl;

    if (inputFiles.empty())
    {
      std::cerr << "No input files!" << std::endl;
      return;
    }

    if (!argThreads.set())  // in such case our value is reset to zero
    {
        // use number of cores if not specified by the user
        max_threads = std::thread::hardware_concurrency();
        if (max_threads == 0)
        {
            // in case the value can't be detected, use something reasonable...
            max_threads = 4;
        }
    }

    // TODO: would be nice to support input directories too (recursive)

    VirtualPointCloud vpc;

    for (const std::string &inputFile : inputFiles)
    {
        std::string inputFileAbsolute = inputFile;
        if (!pdal::Utils::isRemote(inputFile) && !fs::path(inputFile).is_absolute())
        {
            // convert to absolute path using the current path
            inputFileAbsolute = fs::absolute(inputFile).string();
        }

        MetadataNode layout;
        MetadataNode n;
        try
        {
            n = getReaderMetadata(inputFileAbsolute, &layout);
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return;
        }

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
        f.filename = inputFileAbsolute;
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

    //

    if (overviewLength > 0.0)
        overview = true;

    std::string overviewFilenameBase;
    if (overview)
    {
        // for /tmp/hello.vpc we will use /tmp/hello-overview.copc.laz as overview file
        // if multiple overview tiles are generated, we will use /tmp/hello-overview-1.copc.laz etc.
        const fs::path outputParentDir = fs::path(outputFile).parent_path();
        const fs::path outputStem = outputParentDir / fs::path(outputFile).stem();
        overviewFilenameBase = outputStem.string();
    }

    // Compute overview tiling grid
    int numOverviewX = 1, numOverviewY = 1;
    const BOX2D totalBox = vpc.box3d().to2d();

    if (overview && overviewLength > 0.0)
    {
        const double totalW = totalBox.maxx - totalBox.minx;
        const double totalH = totalBox.maxy - totalBox.miny;
        numOverviewX = (std::max)(1, static_cast<int>(std::ceil(totalW / overviewLength)));
        numOverviewY = (std::max)(1, static_cast<int>(std::ceil(totalH / overviewLength)));
    }

    const int numOverviewTiles = numOverviewX * numOverviewY;
    const bool multiOverview = numOverviewTiles > 1;

    struct OverviewTile {
        BOX2D bbox;
        std::string copcFilename;
        std::vector<std::string> tempFiles;
    };
    std::vector<OverviewTile> overviewTiles;

    if (overview)
    {
        overviewTiles.reserve(numOverviewTiles);
        int overviewCounter = 0;
        for (int iy = 0; iy < numOverviewY; ++iy)
        {
            for (int ix = 0; ix < numOverviewX; ++ix)
            {
                if (multiOverview)
                {
                    OverviewTile ovTile;
                    ovTile.bbox = BOX2D(totalBox.minx + ix * overviewLength,
                                        totalBox.miny + iy * overviewLength,
                                        totalBox.minx + (ix + 1) * overviewLength,
                                        totalBox.miny + (iy + 1) * overviewLength);

                    // overview tiles are on a grid, some might not overlap with vpc files
                    // so we skip them to preserve continuous filename numbering
                    if (vpc.overlappingBox2D(ovTile.bbox).empty())
                        continue;

                    ovTile.copcFilename = overviewFilenameBase + "-overview-" + std::to_string(++overviewCounter) + ".copc.laz";
                    overviewTiles.push_back(ovTile);
                }
                else
                {
                    OverviewTile ovTile;
                    ovTile.bbox = totalBox;
                    ovTile.copcFilename = overviewFilenameBase + "-overview.copc.laz";
                    overviewTiles.push_back(ovTile);
                }
            }
        }
    }

    if (boundaries || stats || overview)
    {
        std::map<std::string, Stage*> hexbinFilters, statsFilters;
        std::vector<std::unique_ptr<PipelineManager>> pipelines;

        int overviewCounter = 0;
        for (VirtualPointCloud::File &f : vpc.files)
        {
            if (overview && multiOverview)
            {
                // For multi-tile overview: one pipeline per (source file, tile) combination
                for (OverviewTile &tile : overviewTiles)
                {
                    if (!tile.bbox.overlaps(f.bbox.to2d()))
                        continue;

                    std::unique_ptr<PipelineManager> manager( new PipelineManager );
                    Stage* last = &makeReader(manager.get(), f.filename);

                    pdal::Options crop_opts;
                    crop_opts.add(pdal::Option("bounds", box_to_pdal_bounds(tile.bbox)));
                    last = &manager->makeFilter("filters.crop", *last, crop_opts);

                    pdal::Options decim_opts;
                    decim_opts.add(pdal::Option("step", 1000));
                    last = &manager->makeFilter("filters.decimation", *last, decim_opts);

                    const std::string overviewOutput = overviewFilenameBase + "-overview-tmp-" + std::to_string(++overviewCounter) + ".las";
                    tile.tempFiles.push_back(overviewOutput);
                    makeWriter(manager.get(), overviewOutput, last);
                    pipelines.push_back(std::move(manager));
                }

                // Boundaries/stats for multi-overview still need a separate pipeline per file
                if (boundaries || stats)
                {
                    std::unique_ptr<PipelineManager> manager( new PipelineManager );
                    Stage* last = &makeReader(manager.get(), f.filename);
                    if (boundaries)
                    {
                        pdal::Options hexbin_opts;
                        last = &manager->makeFilter("filters.hexbin", *last, hexbin_opts);
                        hexbinFilters[f.filename] = last;
                    }
                    if (stats)
                    {
                        pdal::Options stats_opts;
                        last = &manager->makeFilter("filters.stats", *last, stats_opts);
                        statsFilters[f.filename] = last;
                    }
                    pipelines.push_back(std::move(manager));
                }
            }
            else
            {
                std::unique_ptr<PipelineManager> manager( new PipelineManager );

                Stage* last = &makeReader(manager.get(), f.filename);
                if (boundaries)
                {
                    pdal::Options hexbin_opts;
                    // TODO: any options?
                    last = &manager->makeFilter( "filters.hexbin", *last, hexbin_opts );
                    hexbinFilters[f.filename] = last;
                }

                if (stats)
                {
                    pdal::Options stats_opts;
                    // TODO: any options?
                    last = &manager->makeFilter( "filters.stats", *last, stats_opts );
                    statsFilters[f.filename] = last;
                }

                if (overview)
                {
                    // TODO: configurable method and step size?
                    pdal::Options decim_opts;
                    decim_opts.add(pdal::Option("step", 1000));
                    last = &manager->makeFilter( "filters.decimation", *last, decim_opts );

                    const std::string overviewOutput = overviewFilenameBase + "-overview-tmp-" + std::to_string(++overviewCounter) + ".las";
                    overviewTiles[0].tempFiles.push_back(overviewOutput);

                    makeWriter(manager.get(), overviewOutput, last);
                }

                pipelines.push_back(std::move(manager));
            }
        }

        runPipelineParallel(vpc.totalPoints(), true, pipelines, max_threads, verbose);

        if (overview)
        {
            // When doing overviews, this is the second stage where we index overview point cloud(s).
            // We do it separately because writers.copc is not streamable. We could also use
            // untwine instead of writers.copc...

            std::vector<std::unique_ptr<PipelineManager>> pipelinesCopcOverview;

            for (const OverviewTile &tile : overviewTiles)
            {
                if (tile.tempFiles.empty())
                    continue;

                std::unique_ptr<PipelineManager> manager( new PipelineManager );

                // TODO: I am not really sure why we need a merge filter, but without it
                // I am only getting points in output COPC from the last reader. Example
                // from the documentation suggests the merge filter should not be needed:
                // https://pdal.io/en/latest/stages/writers.copc.html

                Stage &merge = manager->makeFilter("filters.merge");

                pdal::Options writer_opts;
                Stage& writer = manager->makeWriter(tile.copcFilename, "writers.copc", merge, writer_opts);
                (void)writer;

                for (const std::string &tempFile : tile.tempFiles)
                {
                    Stage& reader = makeReader(manager.get(), tempFile);
                    merge.setInput(reader);
                }

                pipelinesCopcOverview.push_back(std::move(manager));
            }

            if (verbose)
            {
                std::cout << "Indexing overview point cloud(s)..." << std::endl;
            }
            runPipelineParallel(vpc.totalPoints()/1000, false, pipelinesCopcOverview, max_threads, verbose);

            // delete tmp overviews
            for (const OverviewTile &tile : overviewTiles)
            {
                for (const std::string &tempFile : tile.tempFiles)
                    std::filesystem::remove(tempFile);
            }
        }

        for (VirtualPointCloud::File &f : vpc.files)
        {
            if (boundaries)
            {
                pdal::Stage *hexbinFilter = hexbinFilters[f.filename];
                std::string b = hexbinFilter->getMetadata().findChild("boundary").value();
                f.boundaryWkt = b;
            }
            if (stats)
            {
                pdal::Stage *statsFilter = statsFilters[f.filename];
                MetadataNode m = statsFilter->getMetadata();
                std::vector<MetadataNode> children = m.children("statistic");
                for (const MetadataNode &n : children)
                {
                    VirtualPointCloud::StatsItem s(
                        n.findChild("name").value(),
                        n.findChild("position").value<uint32_t>(),
                        n.findChild("average").value<double>(),
                        n.findChild("count").value<point_count_t>(),
                        n.findChild("maximum").value<double>(),
                        n.findChild("minimum").value<double>(),
                        n.findChild("stddev").value<double>(),
                        n.findChild("variance").value<double>());
                    f.stats.push_back(s);
                }
            }
            if (overview)
            {
                for (const OverviewTile &tile : overviewTiles)
                {
                    if (tile.tempFiles.empty())
                        continue; // empty tile (no source files)
                    // we can't use overlaps() here as we don't want to include bboxes that only touch
                    if (tile.bbox.minx < f.bbox.maxx &&
                            tile.bbox.maxx > f.bbox.minx &&
                            tile.bbox.miny < f.bbox.maxy &&
                            tile.bbox.maxy > f.bbox.miny)
                        f.overviewFilenames.push_back(tile.copcFilename);
                }
            }
        }
    }

    vpc.write(outputFile);

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
