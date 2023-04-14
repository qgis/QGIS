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


void Merge::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output virtual point cloud file", outputFile);
    // we set hasSingleInput=false so the default "input,i" argument is not added
    programArgs.add("files", "input files", inputFiles).setPositional();
}

bool Merge::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    return true;
}

static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile)
{
    assert(tile);

    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    pdal::Options options;
    options.add(pdal::Option("forward", "all"));

    if (!tile->filterExpression.empty())
    {
        options.add(pdal::Option("where", tile->filterExpression));
    }

    Stage& w = manager->makeWriter(tile->outputFilename, "", options);

    for (const std::string& f : tile->inputFilenames)
    {
        w.setInput(manager->makeReader(f, ""));
    }

    return manager;
}

void Merge::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines, const BOX3D &, point_count_t &totalPoints)
{
    ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression);
    tile.inputFilenames = inputFiles;
    tile.outputFilename = outputFile;

    pipelines.push_back(pipeline(&tile));

    // only algs with single input have the number of points figured out already
    totalPoints = 0;
    for (const std::string& f : inputFiles)
    {
        QuickInfo qi = getQuickInfo(f);
        totalPoints += qi.m_pointCount;
    }

}
