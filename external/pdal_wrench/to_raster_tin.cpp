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


std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, double resolution, double collarSize)
{
    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    std::vector<Stage*> readers;
    for (const std::string &f : tile->inputFilenames)
    {
        readers.push_back(&manager->makeReader(f, ""));
    }

    std::vector<Stage*> last = readers;

    // find out what will be the bounding box for this job
    // (there could be also no bbox if there's no "bounds" filter and no tiling)
    BOX2D filterBox = !tile->filterBounds.empty() ? parseBounds(tile->filterBounds).to2d() : BOX2D();
    BOX2D box = intersectTileBoxWithFilterBox(tile->box, filterBox);
    BOX2D boxWithCollar;

    // box with collar is used for filtering of data on the input
    // bow without collar is used for output bounds

    if (box.valid())
    {
        BOX2D filterBoxWithCollar = filterBox;
        if (filterBoxWithCollar.valid())
            filterBoxWithCollar.grow(collarSize);
        boxWithCollar = tile->box;
        boxWithCollar.grow(collarSize);
        boxWithCollar = intersectTileBoxWithFilterBox(boxWithCollar, filterBoxWithCollar);

        // We are going to do filtering of points based on 2D box. Ideally we want to do
        // the filtering in the reader (if the reader can do it efficiently like copc/ept),
        // otherwise we have to add filters.crop stage to filter points after they were read

        for (Stage* reader : readers)
        {
            if (readerSupportsBounds(*reader))
            {
                // add "bounds" option to reader
                pdal::Options copc_opts;
                copc_opts.add(pdal::Option("threads", 1));
                copc_opts.add(pdal::Option("bounds", box_to_pdal_bounds(boxWithCollar)));
                reader->addOptions(copc_opts);
            }
        }

        if (!allReadersSupportBounds(readers) && !tile->filterBounds.empty())
        {
            // At least some readers can't do the filtering - do it with a filter
            Options filter_opts;
            filter_opts.add(pdal::Option("bounds", box_to_pdal_bounds(filterBoxWithCollar)));
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

    if (readers.size() > 1)
    {
        // explicitly merge if there are multiple inputs, otherwise things don't work
        // (no pixels are written - not sure which downstream stage is to blame)
        Stage *merge = &manager->makeFilter("filters.merge");
        for (Stage *stage : last)
            merge->setInput(*stage);
        last.clear();
        last.push_back(merge);
    }

    Stage &delaunay = manager->makeFilter("filters.delaunay");
    for (Stage *stage : last)
        delaunay.setInput(*stage);

    pdal::Options faceRaster_opts;
    faceRaster_opts.add(pdal::Option("resolution", resolution));

    if (box.valid())  // if box is not provided, filters.faceraster will calculate it from data
    {
        faceRaster_opts.add(pdal::Option("origin_x", box.minx));
        faceRaster_opts.add(pdal::Option("origin_y", box.miny));
        faceRaster_opts.add(pdal::Option("width", ceil((box.maxx-box.minx)/resolution)));
        faceRaster_opts.add(pdal::Option("height", ceil((box.maxy-box.miny)/resolution)));
    }

    Stage &faceRaster = manager->makeFilter("filters.faceraster", delaunay, faceRaster_opts);

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("data_type", "float32"));  // default was float64 which seems like too much
    writer_opts.add(pdal::Option("gdalopts", "TILED=YES"));
    writer_opts.add(pdal::Option("gdalopts", "COMPRESS=DEFLATE"));
    (void)manager->makeWriter(tile->outputFilename, "writers.raster", faceRaster);

    return manager;
}


void ToRasterTin::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
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

                if (!filterBounds.empty() && !intersectionBox2D(tileBox, parseBounds(filterBounds).to2d()).valid())
                {
                    if (verbose)
                        std::cout << "skipping tile " << iy << " " << ix << " -- " << tileBox.toBox() << std::endl;
                    continue;
                }

                ParallelJobInfo tile(ParallelJobInfo::Spatial, tileBox, filterExpression, filterBounds);

                // add collar to avoid edge effects
                BOX2D boxWithCollar = tileBox;
                boxWithCollar.grow(collarSize);

                for (const VirtualPointCloud::File & f: vpc.overlappingBox2D(boxWithCollar))
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

                pipelines.push_back(pipeline(&tile, resolution, collarSize));
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

                if (!filterBounds.empty() && !intersectionBox2D(tileBox, parseBounds(filterBounds).to2d()).valid())
                {
                    if (verbose)
                        std::cout << "skipping tile " << iy << " " << ix << " -- " << tileBox.toBox() << std::endl;
                    continue;
                }

                ParallelJobInfo tile(ParallelJobInfo::Spatial, tileBox, filterExpression, filterBounds);
                tile.inputFilenames.push_back(inputFile);

                // add collar to avoid edge effects
                BOX2D boxWithCollar = tileBox;
                boxWithCollar.grow(collarSize);

                // create temp output file names
                // for tile (x=2,y=3) that goes to /tmp/hello.tif,
                // individual output file will be called /tmp/hello/2_3.tif
                fs::path inputBasename = std::to_string(ix) + "_" + std::to_string(iy);
                tile.outputFilename = (outputSubdir / inputBasename).string() + ".tif";

                tileOutputFiles.push_back(tile.outputFilename);

                pipelines.push_back(pipeline(&tile, resolution, collarSize));
            }
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, resolution, 0));
    }
}

void ToRasterTin::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (!tileOutputFiles.empty())
    {
        rasterTilesToCog(tileOutputFiles, outputFile);

        // clean up the temporary directory
        fs::path outputParentDir = fs::path(outputFile).parent_path();
        fs::path outputSubdir = outputParentDir / fs::path(outputFile).stem();
        fs::remove_all(outputSubdir);
    }
}
