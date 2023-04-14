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

#include "utils.hpp"
#include "alg.hpp"
#include "vpc.hpp"

using namespace pdal;

namespace fs = std::filesystem;

/*
memory requirements to keep in mind:
- delaunator-cpp: 136 bytes per point -> 10M pts ~ 1.36 GB
- mesh in pdal: 48 bytes per point -> 10M pts ~ 0.5 GB
*/

void ToRasterTin::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output raster file", outputFile);
    argRes = &programArgs.add("resolution,r", "Resolution of the output grid", resolution);
    argTileSize = &programArgs.add("tile-size", "Size of a tile for parallel runs", tileAlignment.tileSize);
    argTileOriginX = &programArgs.add("tile-origin-x", "X origin of a tile for parallel runs", tileAlignment.originX);
    argTileOriginY = &programArgs.add("tile-origin-y", "Y origin of a tile for parallel runs", tileAlignment.originY);
}

bool ToRasterTin::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }
    if (!argRes->set())
    {
        std::cerr << "missing resolution" << std::endl;
        return false;
    }

    if (!argTileSize->set())
    {
        tileAlignment.tileSize = 1000;
    }

    if (!argTileOriginX->set())
        tileAlignment.originX = -1;
    if (!argTileOriginY->set())
        tileAlignment.originY = -1;

    collarSize = resolution*10;  // what's the right collar size?

    return true;
}


std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, double resolution)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    std::vector<Stage*> readers;
    for (const std::string &f : tile->inputFilenames)
    {
        readers.push_back(&manager->makeReader(f, ""));
    }

    if (tile->mode == ParallelJobInfo::Spatial)
    {
        for (Stage* reader : readers)
        {
            // with COPC files, we can also specify bounds at the reader
            // that will only read the required parts of the file
            if (reader->getName() == "readers.copc")
            {
                pdal::Options copc_opts;
                copc_opts.add(pdal::Option("threads", 1));
                copc_opts.add(pdal::Option("bounds", box_to_pdal_bounds(tile->boxWithCollar)));
                reader->addOptions(copc_opts);
            }
        }
    }

    Stage &delaunay = manager->makeFilter("filters.delaunay");
    for (Stage *stage : readers)
    {
        delaunay.setInput(*stage);  // connect all readers to the writer
    }

    if (!tile->filterExpression.empty())
    {
        Options filter_opts;
        filter_opts.add(pdal::Option("where", tile->filterExpression));
        delaunay.addOptions(filter_opts);
    }

    pdal::Options faceRaster_opts;
    faceRaster_opts.add(pdal::Option("resolution", resolution));

    if (tile->box.valid())  // if box is not provided, filters.faceraster will calculate it from data
    {
        faceRaster_opts.add(pdal::Option("origin_x", tile->box.minx));
        faceRaster_opts.add(pdal::Option("origin_y", tile->box.miny));
        faceRaster_opts.add(pdal::Option("width", (tile->box.maxx-tile->box.minx)/resolution));
        faceRaster_opts.add(pdal::Option("height", (tile->box.maxy-tile->box.miny)/resolution));
    }

    Stage &faceRaster = manager->makeFilter("filters.faceraster", delaunay, faceRaster_opts);

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("data_type", "float32"));  // default was float64 which seems like too much
    writer_opts.add(pdal::Option("gdalopts", "TILED=YES"));
    writer_opts.add(pdal::Option("gdalopts", "COMPRESS=DEFLATE"));
    (void)manager->makeWriter(tile->outputFilename, "writers.raster", faceRaster);

    return manager;
}


void ToRasterTin::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines, const BOX3D &bounds, point_count_t &totalPoints)
{
    if (ends_with(inputFile, ".vpc"))
    {
        // using spatial processing

        VirtualPointCloud vpc;
        if (!vpc.read(inputFile))
            return;

        // for /tmp/hello.tif we will use /tmp/hello dir for all results
        fs::path outputParentDir = fs::path(outputFile).parent_path();
        fs::path outputSubdir = outputParentDir / fs::path(outputFile).stem();
        fs::create_directories(outputSubdir);

        // TODO: optionally adjust origin to have nicer numbers for bounds?
        if (tileAlignment.originX == -1)
            tileAlignment.originX = bounds.minx;
        if (tileAlignment.originY == -1)
            tileAlignment.originY = bounds.miny;

        // align bounding box of data to the grid
        TileAlignment gridAlignment = tileAlignment;
        gridAlignment.tileSize = resolution;
        Tiling gridTiling = gridAlignment.coverBounds(bounds.to2d());
        BOX2D gridBounds = gridTiling.fullBox();
        Tiling t = tileAlignment.coverBounds(gridBounds);

        if (verbose)
        {
          std::cout << "grid " << gridTiling.tileCountX << "x" << gridTiling.tileCountY << std::endl;
          std::cout << "tiles " << t.tileCountX << " " << t.tileCountY << std::endl;
        }

        totalPoints = 0;  // we need to recalculate as we may use some points multiple times
        for (int iy = 0; iy < t.tileCountY; ++iy)
        {
            for (int ix = 0; ix < t.tileCountX; ++ix)
            {
                BOX2D tileBox = t.boxAt(ix, iy);

                // for tiles that are smaller than full box - only use intersection
                // to avoid empty areas in resulting rasters
                tileBox.clip(gridBounds);

                ParallelJobInfo tile(ParallelJobInfo::Spatial, tileBox, filterExpression);

                // add collar to avoid edge effects
                tile.boxWithCollar = tileBox;
                tile.boxWithCollar.grow(collarSize);

                for (const VirtualPointCloud::File & f: vpc.overlappingBox2D(tile.boxWithCollar))
                {
                    tile.inputFilenames.push_back(f.filename);
                    totalPoints += f.count;
                }
                if (tile.inputFilenames.empty())
                    continue;   // no input files for this tile

                // create temp output file names
                // for tile (x=2,y=3) that goes to /tmp/hello.tif,
                // individual output file will be called /tmp/hello/2_3.tif
                fs::path inputBasename = std::to_string(ix) + "_" + std::to_string(iy);
                tile.outputFilename = (outputSubdir / inputBasename).string() + ".tif";

                tileOutputFiles.push_back(tile.outputFilename);

                pipelines.push_back(pipeline(&tile, resolution));
            }
        }
    }
    else if (ends_with(inputFile, ".copc.laz"))
    {
        // using square tiles for single COPC

        // for /tmp/hello.tif we will use /tmp/hello dir for all results
        fs::path outputParentDir = fs::path(outputFile).parent_path();
        fs::path outputSubdir = outputParentDir / fs::path(outputFile).stem();
        fs::create_directories(outputSubdir);

        if (tileAlignment.originX == -1)
            tileAlignment.originX = bounds.minx;
        if (tileAlignment.originY == -1)
            tileAlignment.originY = bounds.miny;

        Tiling t = tileAlignment.coverBounds(bounds.to2d());

        for (int iy = 0; iy < t.tileCountY; ++iy)
        {
            for (int ix = 0; ix < t.tileCountX; ++ix)
            {
                BOX2D tileBox = t.boxAt(ix, iy);

                ParallelJobInfo tile(ParallelJobInfo::Spatial, tileBox, filterExpression);
                tile.inputFilenames.push_back(inputFile);

                // add collar to avoid edge effects
                tile.boxWithCollar = tileBox;
                tile.boxWithCollar.grow(collarSize);

                // create temp output file names
                // for tile (x=2,y=3) that goes to /tmp/hello.tif,
                // individual output file will be called /tmp/hello/2_3.tif
                fs::path inputBasename = std::to_string(ix) + "_" + std::to_string(iy);
                tile.outputFilename = (outputSubdir / inputBasename).string() + ".tif";

                tileOutputFiles.push_back(tile.outputFilename);

                pipelines.push_back(pipeline(&tile, resolution));
            }
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, resolution));
    }
}

void ToRasterTin::finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{
    if (pipelines.size() > 1)
    {
        rasterTilesToCog(tileOutputFiles, outputFile);
    }
}
