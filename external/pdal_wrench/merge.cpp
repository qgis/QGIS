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
#include <fstream>
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
    programArgs.add("input-file-list", "Read input files from a text file", inputFileList);

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

    std::vector<Stage*> readers;
    for (const std::string& f : tile->inputFilenames)
    {
        readers.push_back(&manager->makeReader(f, ""));
    }

    std::vector<Stage*> last = readers;

    // filtering
    if (!tile->filterBounds.empty())
    {
        Options filter_opts;
        filter_opts.add(pdal::Option("bounds", tile->filterBounds));

        if (allReadersSupportBounds(readers))
        {
            // Reader of the format can do the filtering - use that whenever possible!
            for (Stage *r : readers)
              r->addOptions(filter_opts);
        }
        else
        {
            // Reader can't do the filtering - do it with a filter
            Stage *filterCrop = &manager->makeFilter( "filters.crop", filter_opts);
            for (Stage *s : last)
                filterCrop->setInput(*s);
            last.clear();
            last.push_back(filterCrop);
        }
    }
    if (!tile->filterExpression.empty())
    {
        Options filter_opts;
        filter_opts.add(pdal::Option("expression", tile->filterExpression));
        Stage *filterExpr = &manager->makeFilter( "filters.expression", filter_opts);
        for (Stage *s : last)
            filterExpr->setInput(*s);
        last.clear();
        last.push_back(filterExpr);
    }

    pdal::Options options;
    options.add(pdal::Option("forward", "all"));
    Stage* writer = &manager->makeWriter(tile->outputFilename, "", options);
    for (Stage *s : last)
        writer->setInput(*s);

    return manager;
}

void Merge::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{
    ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
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
