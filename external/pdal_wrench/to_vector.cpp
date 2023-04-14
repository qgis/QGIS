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


void ToVector::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output vector file", outputFile);
    programArgs.add("attribute", "Attributes to include", attributes);
}

bool ToVector::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    return true;
}


static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, const std::vector<std::string> &attributes)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Stage& r = manager->makeReader( tile->inputFilenames[0], "");

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("ogrdriver", "GPKG"));
    if (!attributes.empty())
        writer_opts.add(pdal::Option("attr_dims", join_strings(attributes, ',')));

    if (!tile->filterExpression.empty())
    {
        writer_opts.add(pdal::Option("where", tile->filterExpression));
    }

    (void)manager->makeWriter( tile->outputFilename, "writers.ogr", r, writer_opts);

    return manager;
}


void ToVector::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines, const BOX3D &, point_count_t &)
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
            ParallelJobInfo tile(ParallelJobInfo::FileBased, BOX2D(), filterExpression);
            tile.inputFilenames.push_back(f.filename);

            // for input file /x/y/z.las that goes to /tmp/hello.vpc,
            // individual output file will be called /tmp/hello/z.las
            fs::path inputBasename = fs::path(f.filename).stem();
            tile.outputFilename = (outputSubdir / inputBasename).string() + ".gpkg";

            tileOutputFiles.push_back(tile.outputFilename);

            pipelines.push_back(pipeline(&tile, attributes));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, attributes));
    }
}

void ToVector::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (tileOutputFiles.empty())
        return;

    // TODO: create Vector VRT and GDALVectorTranslate()
}
