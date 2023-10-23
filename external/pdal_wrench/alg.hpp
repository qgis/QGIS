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

#pragma once

#include <pdal/PipelineManager.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include "utils.hpp"

using namespace pdal;

struct ParallelJobInfo;


/**
 * Base class for algorithms. The general pattern is that:
 * 1. algorithm defines arguments (addArgs()) and checks if the values from user are valid (checkArgs())
 * 2. prepare PDAL pipelines (preparePipelines()) and get them executed in multiple threads
 * 3. run finalization code (finalize())
 */
struct Alg
{
    // parallel runs (generic)
    int max_threads = -1;

    // a hint whether the pipelines will be executed in streaming mode
    bool isStreaming = true;

    // all algs should have some input...
    bool hasSingleInput = true;   // some algs need multiple inputs - they should set this flag to false
    std::string inputFile;

    std::string filterExpression;  // optional argument to limit input points

    std::string filterBounds;  // optional clipping rectangle for input (pdal::Bounds)

    bool needsSingleCrs = true;  // most algs assume that all input files in VPC are in the same CRS,
                                 // and only few exceptions (e.g. info) tolerate mixture of multiple CRS

    bool verbose = false;        // write extra debugging output from the algorithm

    point_count_t totalPoints = 0;   // calculated number of points from the input data
    BOX3D bounds;                    // calculated 3D bounding box from the input data
    SpatialReference crs;            // CRS of the input data (only valid when needsSingleCrs==true)


    pdal::ProgramArgs programArgs;

    Alg() = default;
    virtual ~Alg() = default;

    // no copying
    Alg(const Alg &other) = delete;
    Alg& operator=(const Alg &other) = delete;

    bool parseArgs(std::vector<std::string> args);

    // interface

    /**
     * Adds required and optional arguments to "programArgs" member variable.
     */
    virtual void addArgs() = 0;
    /**
     * Called after argument parsing - evaluates whether the input is correct, returns false if not.
     */
    virtual bool checkArgs() = 0;
    /**
     * Prepares pipelines that the algorithm needs to run and populates the given vector.
     * Pipelines are then run in a thread pool.
     */
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) = 0;
    /**
     * Runs and post-processing code when pipelines are done executing.
     */
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) { ( void )pipelines; };
};

bool runAlg(std::vector<std::string> args, Alg &alg);


//////////////


struct Info : public Alg
{
    Info() { needsSingleCrs = false; }

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
};

struct Translate : public Alg
{
    // parameters from the user
    std::string outputFile;
    std::string assignCrs;
    std::string transformCrs;
    std::string transformCoordOp;
    std::string outputFormat;  // las / laz / copc

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argOutputFormat = nullptr;

    std::vector<std::string> tileOutputFiles;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
};

struct Density : public Alg
{
    // parameters from the user
    std::string outputFile;
    double resolution = 0;

    // tiling setup for parallel runs
    TileAlignment tileAlignment;

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argRes = nullptr;
    pdal::Arg* argTileSize = nullptr;
    pdal::Arg* argTileOriginX = nullptr;
    pdal::Arg* argTileOriginY = nullptr;

    std::vector<std::string> tileOutputFiles;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;

    // new
    std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile = nullptr) const;
};


struct Boundary : public Alg
{
    // parameters from the user
    std::string outputFile;
    double resolution = 0;    // cell size of the hexbin filter (if zero, it will be estimated by PDAL)
    int pointsThreshold = 0;  // min. number of points in order to have a cell considered occupied

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argResolution = nullptr;
    pdal::Arg* argPointsThreshold = nullptr;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;

};


struct Clip : public Alg
{
    // parameters from the user
    std::string outputFile;
    std::string polygonFile;
    std::string outputFormat;  // las / laz / copc

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argOutputFormat = nullptr;
    pdal::Arg* argPolygon = nullptr;

    std::vector<std::string> tileOutputFiles;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;

    // new
    //std::unique_ptr<PipelineManager> pipeline(ParallelTileInfo *tile, const pdal::Options &crop_opts) const;
};


struct Merge : public Alg
{

    // parameters from the user
    std::string outputFile;
    std::vector<std::string> inputFiles;
    std::string inputFileList;

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;

    Merge() { hasSingleInput = false; }

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;

};


struct Thin : public Alg
{
    // parameters from the user
    std::string outputFile;
    std::string mode;  // "every-nth" or "sample"
    int stepEveryN;  // keep every N-th point
    double stepSample;  // cell size for Poisson sampling
    std::string outputFormat;  // las / laz / copc

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argMode = nullptr;
    pdal::Arg* argStepEveryN = nullptr;
    pdal::Arg* argStepSample = nullptr;
    pdal::Arg* argOutputFormat = nullptr;

    std::vector<std::string> tileOutputFiles;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
};



struct ToRaster : public Alg
{
    // parameters from the user
    std::string outputFile;
    double resolution = 0;
    std::string attribute;
    double collarSize = 0;

    // tiling setup for parallel runs
    TileAlignment tileAlignment;

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argRes = nullptr;
    pdal::Arg* argAttribute = nullptr;

    pdal::Arg* argTileSize = nullptr;
    pdal::Arg* argTileOriginX = nullptr;
    pdal::Arg* argTileOriginY = nullptr;

    std::vector<std::string> tileOutputFiles;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
};


struct ToRasterTin : public Alg
{
    // parameters from the user
    std::string outputFile;
    double resolution = 0;
    double collarSize = 0;

    // tiling setup for parallel runs
    TileAlignment tileAlignment;

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    pdal::Arg* argRes = nullptr;
    pdal::Arg* argTileSize = nullptr;
    pdal::Arg* argTileOriginX = nullptr;
    pdal::Arg* argTileOriginY = nullptr;

    std::vector<std::string> tileOutputFiles;

    ToRasterTin() { isStreaming = false; }

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
};


struct ToVector : public Alg
{
    // parameters from the user
    std::string outputFile;
    std::vector<std::string> attributes;

    // args - initialized in addArgs()
    pdal::Arg* argOutput = nullptr;
    //pdal::Arg* argAttribute = nullptr;

    std::vector<std::string> tileOutputFiles;

    // impl
    virtual void addArgs() override;
    virtual bool checkArgs() override;
    virtual void preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
    virtual void finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines) override;
};
