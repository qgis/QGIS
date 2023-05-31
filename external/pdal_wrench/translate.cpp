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


void Translate::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output point cloud file", outputFile);
    argOutputFormat = &programArgs.add("output-format", "Output format (las/laz/copc)", outputFormat);
    programArgs.add("assign-crs", "Assigns CRS to data (no reprojection)", assignCrs);
    programArgs.add("transform-crs", "Transforms (reprojects) data to another CRS", transformCrs);
    programArgs.add("transform-coord-op", "Details on how to do the transform of coordinates when --transform-crs is used. "
                    "It can be a PROJ pipeline or a WKT2 CoordinateOperation. "
                    "When not specified, PROJ will pick the default transform.", transformCoordOp);
}

bool Translate::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    // TODO: or use the same format as the reader?
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

    if (!transformCoordOp.empty() && transformCrs.empty())
    {
        std::cerr << "Need to specify also --transform-crs when --transform-coord-op is used." << std::endl;
        return false;
    }

    return true;
}


static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, std::string assignCrs, std::string transformCrs, std::string transformCoordOp)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Options reader_opts;
    if (!assignCrs.empty())
        reader_opts.add(pdal::Option("override_srs", assignCrs));

    Stage& r = manager->makeReader( tile->inputFilenames[0], "", reader_opts);

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

    // optional reprojection
    Stage* reproject = nullptr;
    if (!transformCrs.empty())
    {
        Options transform_opts;
        transform_opts.add(pdal::Option("out_srs", transformCrs));
        if (!transformCoordOp.empty())
        {
            transform_opts.add(pdal::Option("coord_op", transformCoordOp));
            reproject = &manager->makeFilter( "filters.projpipeline", *last, transform_opts);
        }
        else
        {
            reproject = &manager->makeFilter( "filters.reprojection", *last, transform_opts);
        }
        last = reproject;
    }

    pdal::Options writer_opts;
    if (!reproject)
    {
        // let's use the same offset & scale & header & vlrs as the input
        writer_opts.add(pdal::Option("forward", "all"));
    }
    else
    {
        // avoid adding offset as it probably wouldn't work
        // TODO: maybe adjust scale as well - depending on the CRS
        writer_opts.add(pdal::Option("forward", "header,scale,vlr"));
        writer_opts.add(pdal::Option("offset_x", "auto"));
        writer_opts.add(pdal::Option("offset_y", "auto"));
        writer_opts.add(pdal::Option("offset_z", "auto"));
    }

    (void)manager->makeWriter( tile->outputFilename, "", *last, writer_opts);

    return manager;
}


void Translate::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{
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
            ParallelJobInfo tile(ParallelJobInfo::FileBased, BOX2D(), filterExpression, filterBounds);
            tile.inputFilenames.push_back(f.filename);

            // for input file /x/y/z.las that goes to /tmp/hello.vpc,
            // individual output file will be called /tmp/hello/z.las
            fs::path inputBasename = fs::path(f.filename).stem();
            tile.outputFilename = (outputSubdir / inputBasename).string() + "." + outputFormat;

            tileOutputFiles.push_back(tile.outputFilename);

            pipelines.push_back(pipeline(&tile, assignCrs, transformCrs, transformCoordOp));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, assignCrs, transformCrs, transformCoordOp));
    }
}

void Translate::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
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
