/*****************************************************************************
 *   Copyright (c) 2025, Lutra Consulting Ltd. and Hobu, Inc.                *
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
#include <pdal/PipelineWriter.hpp>

#include <gdal_utils.h>

#include "utils.hpp"
#include "alg.hpp"
#include "vpc.hpp"

using namespace pdal;

namespace fs = std::filesystem;


void ClassifyGround::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output point cloud file", outputFile);
    argOutputFormat = &programArgs.add("output-format", "Output format (las/laz/copc)", outputFormat);
    
    argCellSize = &programArgs.add("cell-size", "Sets the grid cell size in map units. Smaller values give finer detail but may increase noise.", cellSize, 1.0);
    argScalar = &programArgs.add("scalar", "Increases the threshold on steeper slopes. Raise this for rough terrain.", scalar, 1.25);
    argSlope = &programArgs.add("slope", "Controls how much terrain slope is tolerated as ground. Increase for steep terrain.", slope, 0.15);
    argThreshold = &programArgs.add("threshold", " Elevation threshold for separating ground from objects. Higher values allow larger deviations from ground.", threshold, 0.5);
    argWindowSize = &programArgs.add("window-size", "The maximum filter window size. Increase to better identify large buildings or objects, decrease to protect smaller features.", windowSize, 18.0);
}

bool ClassifyGround::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    if (argOutputFormat->set())
    {
        if (outputFormat != "las" && outputFormat != "laz" && outputFormat != "copc")
        {
            std::cerr << "unknown output format: " << outputFormat << std::endl;
            return false;
        }
    }
    else
        outputFormat = "las";  // uncompressed by default

    return true;
}

static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, pdal::Options &filterOptions)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Stage& r = makeReader(manager.get(), tile->inputFilenames[0]);

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

    last = &manager->makeFilter( "filters.smrf", *last, filterOptions);
 
    makeWriter(manager.get(), tile->outputFilename, last);

    return manager;
}


void ClassifyGround::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{   
    pdal::Options filterOptions;
    filterOptions.add(pdal::Option("cell", cellSize));
    filterOptions.add(pdal::Option("scalar", scalar));
    filterOptions.add(pdal::Option("slope", slope));
    filterOptions.add(pdal::Option("threshold", threshold));
    filterOptions.add(pdal::Option("window", windowSize));
    

    if (ends_with(inputFile, ".vpc"))
    {
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
            fs::path inputBasename = fileStem(f.filename);

            if (!ends_with(outputFile, ".vpc"))
                tile.outputFilename = (outputSubdir / inputBasename).string() + ".las";
            else
                tile.outputFilename = (outputSubdir / inputBasename).string() + "." + outputFormat;

            tileOutputFiles.push_back(tile.outputFilename);

            pipelines.push_back(pipeline(&tile, filterOptions));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;

        pipelines.push_back(pipeline(&tile, filterOptions));
    }
}

void ClassifyGround::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (tileOutputFiles.empty())
        return;

    buildOutput(outputFile, tileOutputFiles);
}