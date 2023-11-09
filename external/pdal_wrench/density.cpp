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

void Density::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output raster file", outputFile);
    argRes = &programArgs.add("resolution,r", "Resolution of the density grid", resolution);
    argTileSize = &programArgs.add("tile-size", "Size of a tile for parallel runs", tileAlignment.tileSize);
    argTileOriginX = &programArgs.add("tile-origin-x", "X origin of a tile for parallel runs", tileAlignment.originX);
    argTileOriginY = &programArgs.add("tile-origin-y", "Y origin of a tile for parallel runs", tileAlignment.originY);
}

bool Density::checkArgs()
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

    // TODO: not sure why ProgramArgs cleans the default value
    if (!argTileSize->set())
    {
        tileAlignment.tileSize = 1000;
    }

    if (!argTileOriginX->set())
        tileAlignment.originX = -1;
    if (!argTileOriginY->set())
        tileAlignment.originY = -1;

    return true;
}


std::unique_ptr<PipelineManager> Density::pipeline(ParallelJobInfo *tile) const
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

    if (box.valid())
    {
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
                copc_opts.add(pdal::Option("bounds", box_to_pdal_bounds(box)));
                reader->addOptions(copc_opts);
            }
        }

        if (!allReadersSupportBounds(readers) && !tile->filterBounds.empty())
        {
            // At least some readers can't do the filtering - do it with a filter
            Options filter_opts;
            filter_opts.add(pdal::Option("bounds", tile->filterBounds));
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

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("binmode", true));
    writer_opts.add(pdal::Option("output_type", "count"));
    writer_opts.add(pdal::Option("resolution", resolution));

    writer_opts.add(pdal::Option("data_type", "int16"));  // 16k points in a cell should be enough? :)
    writer_opts.add(pdal::Option("gdalopts", "TILED=YES"));
    writer_opts.add(pdal::Option("gdalopts", "COMPRESS=DEFLATE"));

    if (box.valid())
    {
        BOX2D box2 = box;
        // fix tile size - PDAL's writers.gdal adds one pixel (see GDALWriter::createGrid()),
        // because it probably expects that that the bounds and resolution do not perfectly match
        box2.maxx -= resolution;
        box2.maxy -= resolution;

        writer_opts.add(pdal::Option("bounds", box_to_pdal_bounds(box2)));
    }

    // TODO: "writers.gdal: Requested driver 'COG' does not support file creation.""
    //   writer_opts.add(pdal::Option("gdaldriver", "COG"));

    pdal::StageCreationOptions opts{ tile->outputFilename, "", nullptr, writer_opts, "" };
    Stage& w = manager->makeWriter( opts );
    for (Stage *stage : last)
        w.setInput(*stage);

    return manager;
}


void Density::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
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

        bool unalignedFiles = false;

        // TODO: optionally adjust origin to have nicer numbers for bounds?
        if (tileAlignment.originX == -1)
            tileAlignment.originX = bounds.minx;
        if (tileAlignment.originY == -1)
            tileAlignment.originY = bounds.miny;

        // align bounding box of data to the grid
        TileAlignment gridAlignment = tileAlignment;
        gridAlignment.tileSize = resolution;
        Tiling gridTiling = gridAlignment.coverBounds(bounds.to2d());
        std::cout << "grid " << gridTiling.tileCountX << "x" << gridTiling.tileCountY << std::endl;
        BOX2D gridBounds = gridTiling.fullBox();

        Tiling t = tileAlignment.coverBounds(gridBounds);
        std::cout << "tiles " << t.tileCountX << " " << t.tileCountY << std::endl;

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
                for (const VirtualPointCloud::File & f: vpc.overlappingBox2D(tileBox))
                {
                    tile.inputFilenames.push_back(f.filename);
                    totalPoints += f.count;
                }
                if (tile.inputFilenames.empty())
                    continue;   // no input files for this tile

                if (tile.inputFilenames.size() > 1)
                    unalignedFiles = true;

                // create temp output file names
                // for tile (x=2,y=3) that goes to /tmp/hello.tif,
                // individual output file will be called /tmp/hello/2_3.tif
                fs::path inputBasename = std::to_string(ix) + "_" + std::to_string(iy);
                tile.outputFilename = (outputSubdir / inputBasename).string() + ".tif";

                tileOutputFiles.push_back(tile.outputFilename);

                pipelines.push_back(pipeline(&tile));
            }
        }

        if (unalignedFiles)
        {
            std::cerr << std::endl;
            std::cerr << "Warning: input files not perfectly aligned with tile grid - processing may take longer." << std::endl;
            std::cerr << "Consider using --tile-size, --tile-origin-x, --tile-origin-y arguments" << std::endl;
            std::cerr << std::endl;
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

                // create temp output file names
                // for tile (x=2,y=3) that goes to /tmp/hello.tif,
                // individual output file will be called /tmp/hello/2_3.tif
                fs::path inputBasename = std::to_string(ix) + "_" + std::to_string(iy);
                tile.outputFilename = (outputSubdir / inputBasename).string() + ".tif";

                tileOutputFiles.push_back(tile.outputFilename);

                pipelines.push_back(pipeline(&tile));
            }
        }
    }
    else
    {
        // single input LAS/LAZ - no parallelism

        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile));
    }

}


void Density::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
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
