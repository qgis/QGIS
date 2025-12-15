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


void HeightAboveGround::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output point cloud file", outputFile);
    argOutputFormat = &programArgs.add("output-format", "Output format (las/laz/copc)", outputFormat);
    argAlgorithm = &programArgs.add("algorithm", "Height Above Ground algorithm to use: nn (Nearest Neighbor) or delaunay (Delaunay).", algorithm, "nn");
    argReplaceZWithHeightAboveGround = &programArgs.add("replace-z", "Replace Z dimension with height above ground (true/false).", replaceZWithHeightAboveGround, true);

    // args - NN
    argNNCount = &programArgs.add("nn-count", "The number of ground neighbors to consider when determining the height above ground for a non-ground point", nnCount, 1);
    argNNMaxDistance = &programArgs.add("nn-max-distance", "Use only ground points within max_distance of non-ground point when performing neighbor interpolation.", nnMaxDistance, 0);

    // args - Delaunay
    argDelaunayCount = &programArgs.add("delaunay-count", "The number of ground neighbors to consider when determining the height above ground for a non-ground point.", delaunayCount, 10);
}

bool HeightAboveGround::checkArgs()
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

    if (!argAlgorithm->set())
    {
        std::cerr << "missing algorithm" << std::endl;
        return false;
    }
    else
    {
        if (!(algorithm == "nn" || algorithm == "delaunay"))
        {
            std::cerr << "unknown algorithm: " << algorithm << std::endl;
            return false;
        }
    }

    if (algorithm == "delaunay" && (argNNMaxDistance->set() || argNNCount->set()))
    {
        std::cout << "nn-* arguments are not supported with delaunay algorithm" << std::endl;
    }

    if (algorithm == "nn" && (argDelaunayCount->set()))
    {
        std::cout << "delaunay-count argument is not supported with nn algorithm" << std::endl;
    }

    return true;
}


static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, std::string algorithm, bool replaceZWithHeightAboveGround, int nnCount, double nnMaxDistance, int delaunayCount)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Options reader_opts;

    Stage& r = makeReader( manager.get(), tile->inputFilenames[0], reader_opts );

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

    // NN HAG filter
    if (algorithm == "nn")
    {
        Options hag_nn_opts;

        if (nnCount > 1)
        {
            hag_nn_opts.add(pdal::Option("count", nnCount));
        }

        if (nnMaxDistance > 0)
        {
            hag_nn_opts.add(pdal::Option("max_distance", nnMaxDistance));
        }

        last = &manager->makeFilter( "filters.hag_nn", *last, hag_nn_opts);
    }

    // Delaunay HAG filter
    if (algorithm == "delaunay")
    {
        Options hag_delaunay_opts;

        if (delaunayCount > 0)
        {
            hag_delaunay_opts.add(pdal::Option("count", delaunayCount));
        }

        last = &manager->makeFilter( "filters.hag_delaunay", *last, hag_delaunay_opts);
    }

    if (replaceZWithHeightAboveGround)
    {
        pdal::Options ferry_opts;
        ferry_opts.add(pdal::Option("dimensions", "HeightAboveGround=>Z"));

        last = &manager->makeFilter( "filters.ferry", *last, ferry_opts);
    }

    makeWriter( manager.get(), tile->outputFilename, last);

    return manager;
}


void HeightAboveGround::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{   
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

            pipelines.push_back(pipeline(&tile, algorithm, replaceZWithHeightAboveGround, nnCount, nnMaxDistance, delaunayCount));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, algorithm, replaceZWithHeightAboveGround, nnCount, nnMaxDistance, delaunayCount));
    }
}

void HeightAboveGround::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (tileOutputFiles.empty())
        return;

    buildOutput(outputFile, tileOutputFiles);
}
