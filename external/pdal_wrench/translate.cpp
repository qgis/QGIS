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
    argOutputFormatVpc = &programArgs.add("vpc-output-format", "Output format (las/laz/copc)", outputFormatVpc, "copc");
    programArgs.add("assign-crs", "Assigns CRS to data (no reprojection)", assignCrs);
    programArgs.add("transform-crs", "Transforms (reprojects) data to another CRS", transformCrs);
    programArgs.add("transform-coord-op", "Details on how to do the transform of coordinates when --transform-crs is used. "
                    "It can be a PROJ pipeline or a WKT2 CoordinateOperation. "
                    "When not specified, PROJ will pick the default transform.", transformCoordOp);
    programArgs.add("transform-matrix", "A whitespace-delimited transformation matrix. "
                    "The matrix is assumed to be presented in row-major order. "
                    "Only matrices with sixteen elements are allowed.", transformMatrix);
}

bool Translate::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    // TODO: or use the same format as the reader?
    if (argOutputFormatVpc->set())
    {
        if (outputFormatVpc != "las" && outputFormatVpc != "laz" && outputFormatVpc != "copc")
        {
            std::cerr << "unknown output format: " << outputFormatVpc << std::endl;
            return false;
        }
    }

    if ( ends_with(outputFile, ".vpc") && outputFormatVpc == "copc" )
    {
        isStreaming = false;
    }

    if (!transformCoordOp.empty() && transformCrs.empty())
    {
        std::cerr << "Need to specify also --transform-crs when --transform-coord-op is used." << std::endl;
        return false;
    }

    return true;
}


static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, std::string assignCrs, std::string transformCrs, std::string transformCoordOp, std::string transformMatrix)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Options reader_opts;
    if (!assignCrs.empty())
        reader_opts.add(pdal::Option("override_srs", assignCrs));

    Stage& r = makeReader(manager.get(), tile->inputFilenames[0], reader_opts);

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

    if (!transformMatrix.empty())
    {
        Options matrix_opts;
        matrix_opts.add(pdal::Option("matrix", transformMatrix));
        Stage* matrixTransform = &manager->makeFilter( "filters.transformation", *last, matrix_opts);
        last = matrixTransform;
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

    makeWriter(manager.get(), tile->outputFilename, last, writer_opts);

    return manager;
}


void Translate::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
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

            tile.outputFilename = tileOutputFileName(outputFile, outputFormatVpc, outputSubdir, f.filename);

            tileOutputFiles.push_back(tile.outputFilename);

            pipelines.push_back(pipeline(&tile, assignCrs, transformCrs, transformCoordOp, transformMatrix));
        }
    }
    else
    {
        if (ends_with(outputFile, ".copc.laz"))
        {
            isStreaming = false;
        }
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, assignCrs, transformCrs, transformCoordOp, transformMatrix));
    }
}

void Translate::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (tileOutputFiles.empty())
        return;

    buildOutput(outputFile, tileOutputFiles);
}
