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
#include <thread>

#include <pdal/PipelineManager.hpp>
#include <pdal/Stage.hpp>
#include <pdal/util/ProgramArgs.hpp>
#include <pdal/pdal_types.hpp>
#include <pdal/Polygon.hpp>

#include <gdal_utils.h>

#include "utils.hpp"
#include "alg.hpp"
#include "vpc.hpp"

using namespace pdal;

namespace fs = std::filesystem;


void Clip::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output point cloud file", outputFile);
    argOutputFormat = &programArgs.add("output-format", "Output format (las/laz/copc)", outputFormat);
    argPolygon = &programArgs.add("polygon,p", "Input polygon vector file", polygonFile);
}

bool Clip::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }
    if (!argPolygon->set())
    {
        std::cerr << "missing polygon" << std::endl;
        return false;
    }

    if (argOutputFormat->set())
    {
        if (outputFormat != "las" && outputFormat != "laz")
        {
            std::cerr << "unknown output format: " << outputFormat << std::endl;
            return false;
        }
    }
    else
        outputFormat = "las";  // uncompressed by default

    return true;
}


// populate polygons into filters.crop options
bool loadPolygons(const std::string &polygonFile, pdal::Options& crop_opts, BOX2D& bbox)
{
    GDALAllRegister();

    GDALDatasetH hDS = GDALOpenEx( polygonFile.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
    if( hDS == NULL )
    {
        std::cerr << "Could not open input polygon file: " << polygonFile << std::endl;
        return false;
    }

    // TODO: reproject polygons to the CRS of the point cloud if they are not the same

    OGRLayerH hLayer = GDALDatasetGetLayer(hDS, 0);
    //hLayer = GDALDatasetGetLayerByName( hDS, "point" );

    OGREnvelope fullEnvelope;

    OGR_L_ResetReading(hLayer);
    OGRFeatureH hFeature;
    while( (hFeature = OGR_L_GetNextFeature(hLayer)) != NULL )
    {
        OGRGeometryH hGeometry = OGR_F_GetGeometryRef(hFeature);
        if ( hGeometry != NULL )
        {
            OGREnvelope envelope;
            OGR_G_GetEnvelope(hGeometry, &envelope);
            if (!fullEnvelope.IsInit())
                fullEnvelope = envelope;
            else
                fullEnvelope.Merge(envelope);
            crop_opts.add(pdal::Option("polygon", pdal::Polygon(hGeometry)));
        }
        OGR_F_Destroy( hFeature );
    }
    GDALClose( hDS );

    bbox = BOX2D(fullEnvelope.MinX, fullEnvelope.MinY, fullEnvelope.MaxX, fullEnvelope.MaxY);

    return true;
}


static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, const pdal::Options &crop_opts)
{
    assert(tile);

    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Stage& r = manager->makeReader( tile->inputFilenames[0], "");

    Stage *last = &r;

    // filtering
    if (!tile->filterBounds.empty())
    {
        Options filter_opts;
        filter_opts.add(pdal::Option("bounds", tile->filterBounds));

        if (readerSupportsBounds(r))
        {
            // Reader of the format can do the filtering - use that whenever possible!
            r.addOptions(filter_opts);
        }
        else
        {
            // Reader can't do the filtering - do it with a filter
            last = &manager->makeFilter( "filters.crop", *last, filter_opts);
        }
    }
    if (!tile->filterExpression.empty())
    {
        Options filter_opts;
        filter_opts.add(pdal::Option("expression", tile->filterExpression));
        last = &manager->makeFilter( "filters.expression", *last, filter_opts);
    }

    last = &manager->makeFilter( "filters.crop", *last, crop_opts );

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("forward", "all"));
    manager->makeWriter( tile->outputFilename, "", *last, writer_opts);

    return manager;
}


void Clip::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{
    pdal::Options crop_opts;
    BOX2D bbox;
    if (!loadPolygons(polygonFile, crop_opts, bbox))
        return;

    if (ends_with(inputFile, ".vpc"))
    {
        if (!ends_with(outputFile, ".vpc"))
        {
            std::cerr << "If input file is a VPC, output should be VPC too." << std::endl;
            return;
        }

        // for /tmp/hello.vpc we will use /tmp/hello dir for all results
        fs::path outputParentDir = fs::path(outputFile).parent_path();
        fs::path outputSubdir = outputParentDir / fs::path(outputFile).stem();
        fs::create_directories(outputSubdir);

        // VPC handling
        VirtualPointCloud vpc;
        if (!vpc.read(inputFile))
            return;

        for (const VirtualPointCloud::File& f : vpc.files)
        {
            if (!bbox.overlaps(f.bbox.to2d()))
            {
                totalPoints -= f.count;
                continue;  // we can safely skip
            }

            if (verbose)
            {
              std::cout << "using " << f.filename << std::endl;
            }

            ParallelJobInfo tile(ParallelJobInfo::FileBased, BOX2D(), filterExpression, filterBounds);
            tile.inputFilenames.push_back(f.filename);

            // for input file /x/y/z.las that goes to /tmp/hello.vpc,
            // individual output file will be called /tmp/hello/z.las
            fs::path inputBasename = fs::path(f.filename).stem();
            tile.outputFilename = (outputSubdir / inputBasename).string() + "." + outputFormat;

            tileOutputFiles.push_back(tile.outputFilename);

            pipelines.push_back(pipeline(&tile, crop_opts));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, crop_opts));
    }
}

void Clip::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (tileOutputFiles.empty())
        return;

    // now build a new output VPC
    std::vector<std::string> args;
    args.push_back("--output=" + outputFile);
    for (std::string f : tileOutputFiles)
        args.push_back(f);
    buildVpc(args);
}
