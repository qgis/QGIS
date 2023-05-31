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


// TODO: add support for filters.sample and/or filters.voxeldownsize
// (both in streaming mode but more memory intense - keeping occupation grid)

void Thin::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output point cloud file", outputFile);
    argOutputFormat = &programArgs.add("output-format", "Output format (las/laz/copc)", outputFormat);
    argMode = &programArgs.add("mode", " 'every-nth' or 'sample' - either to keep every N-th point or to keep points based on their distance", mode);
    argStepEveryN = &programArgs.add("step-every-nth", "Keep every N-th point", stepEveryN);
    argStepSample = &programArgs.add("step-sample", "Minimum spacing between points", stepSample);
}

bool Thin::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    if (!argMode->set())
    {
        std::cerr << "missing mode" << std::endl;
        return false;
    }
    else if (mode == "every-nth")
    {
        if (!argStepEveryN->set())
        {
            std::cerr << "missing step for every N-th point mode" << std::endl;
            return false;
        }
    }
    else if (mode == "sample")
    {
        if (!argStepSample->set())
        {
            std::cerr << "missing step for sampling mode" << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "unknown mode: " << mode << std::endl;
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


static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, std::string mode, int stepEveryN, double stepSample)
{
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

    if (mode == "every-nth")
    {
        pdal::Options decim_opts;
        decim_opts.add(pdal::Option("step", stepEveryN));
        last = &manager->makeFilter( "filters.decimation", *last, decim_opts );
    }
    else if (mode == "sample")
    {
        pdal::Options sample_opts;
        sample_opts.add(pdal::Option("cell", stepSample));
        last = &manager->makeFilter( "filters.sample", *last, sample_opts );
    }

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("forward", "all"));  // TODO: maybe we could use lower scale than the original
    manager->makeWriter( tile->outputFilename, "", *last, writer_opts);

    return manager;
}


void Thin::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
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

            pipelines.push_back(pipeline(&tile, mode, stepEveryN, stepSample));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, mode, stepEveryN, stepSample));
    }
}

void Thin::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
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
