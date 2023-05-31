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

#include "alg.hpp"

#include "utils.hpp"
#include "vpc.hpp"

#include <thread>

#include <pdal/QuickInfo.hpp>
#include <pdal/util/Bounds.hpp>

using namespace pdal;


bool runAlg(std::vector<std::string> args, Alg &alg)
{

    try
    {
        if ( !alg.parseArgs(args) )
            return false;
    }
    catch (const pdal::arg_error& err)
    {
        std::cerr << "Failed to parse arguments: " << err.what() << std::endl;
        return false;
    }

    if (alg.hasSingleInput)
    {
        if (ends_with(alg.inputFile, ".vpc"))
        {
            VirtualPointCloud vpc;
            if (!vpc.read(alg.inputFile))
                return false;
            alg.totalPoints = vpc.totalPoints();
            alg.bounds = vpc.box3d();
            if (!alg.needsSingleCrs)
                alg.crs = SpatialReference(vpc.crsWkt);

            if (alg.needsSingleCrs && vpc.crsWkt == "_mix_")
            {
                std::cerr << "Algorithm requires that all inputs are in the same CRS. Please transform them to a single CRS first." << std::endl;
                return false;
            }
        }
        else
        {
            QuickInfo qi = getQuickInfo(alg.inputFile);
            alg.totalPoints = qi.m_pointCount;
            alg.bounds = qi.m_bounds;
            alg.crs = qi.m_srs;
        }
    }

    std::vector<std::unique_ptr<PipelineManager>> pipelines;

    alg.preparePipelines(pipelines);

    if (pipelines.empty())
        return false;

    runPipelineParallel(alg.totalPoints, alg.isStreaming, pipelines, alg.max_threads, alg.verbose);

    alg.finalize(pipelines);

    return true;
}


bool Alg::parseArgs(std::vector<std::string> args)
{
    pdal::Arg* argInput = nullptr;
    if (hasSingleInput)
    {
        argInput = &programArgs.add("input,i", "Input point cloud file", inputFile);
    }

    (void)programArgs.add("filter,f", "Filter expression for input data", filterExpression);
    (void)programArgs.add("bounds", "Filter by rectangle", filterBounds);

    addArgs();  // impl in derived

    // parallel run support (generic)
    pdal::Arg& argThreads = programArgs.add("threads", "Max number of concurrent threads for parallel runs", max_threads);

    programArgs.add("verbose", "Print extra debugging output", verbose);

    try
    {
        programArgs.parseSimple(args);
    }
    catch(pdal::arg_error err)
    {
        std::cerr << "failed to parse arguments: " << err.what() << std::endl;
        return false;
    }

    // TODO: ProgramArgs does not support required options
    if (argInput && !argInput->set())
    {
        std::cerr << "missing input" << std::endl;
        return false;
    }

    if (!filterBounds.empty())
    {
        try
        {
            parseBounds(filterBounds);
        }
        catch (pdal::Bounds::error& err)
        {
            std::cerr << "invalid bounds: " << err.what() << std::endl;
            return false;
        }
    }

    if (!checkArgs())  // impl in derived class
        return false;

    if (!args.empty())
    {
        std::cerr << "unexpected args!" << std::endl;
        for ( auto & a : args )
            std::cerr << " - " << a << std::endl;
        return false;
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

    return true;
}
