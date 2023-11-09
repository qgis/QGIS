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
#include <thread>

#include <pdal/PipelineManager.hpp>
#include <pdal/Stage.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include <gdal.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>

#include "utils.hpp"
#include "alg.hpp"
#include "vpc.hpp"

using namespace pdal;


void Boundary::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output vector file", outputFile);

    argResolution = &programArgs.add("resolution", "Resolution of cells used to calculate boundary. "
                                     "If not specified, it will be estimated from first 5000 points.", resolution);
    argPointsThreshold = &programArgs.add("threshold", "Minimal number of points in a cell to consider cell occupied.", pointsThreshold);
}

bool Boundary::checkArgs()
{
    if (!argOutput->set())
    {
        std::cerr << "missing output" << std::endl;
        return false;
    }

    if (!argResolution->set() && argPointsThreshold->set())
    {
        std::cerr << "Resolution argument must be set when points threshold is set." << std::endl;
        return false;
    }

    if (!argPointsThreshold->set())
    {
        pointsThreshold = 15;   // the same default as in PDAL for HexBin filter
    }

    return true;
}

static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, double resolution, int pointsThreshold)
{
    assert(tile);
    assert(tile->inputFilenames.size() == 1);

    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Stage& r = manager->makeReader(tile->inputFilenames[0], "");

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

    // TODO: what edge size? (by default samples 5000 points if not specified
    // TODO: set threshold ? (default at least 16 points to keep the cell)
    // btw. if threshold=0, there are still missing points because of simplification (smooth=True)

    pdal::Options hexbin_opts;
    if (resolution != 0)
    {
       hexbin_opts.add(pdal::Option("edge_size", resolution));
    }
    hexbin_opts.add(pdal::Option("threshold", pointsThreshold));
    (void)manager->makeFilter( "filters.hexbin", *last, hexbin_opts );

    return manager;
}

void Boundary::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{
    if (ends_with(inputFile, ".vpc"))
    {
        // VPC handling
        VirtualPointCloud vpc;
        if (!vpc.read(inputFile))
            return;

        for (const VirtualPointCloud::File& f : vpc.files)
        {
            ParallelJobInfo tile(ParallelJobInfo::FileBased, BOX2D(), filterExpression, filterBounds);
            tile.inputFilenames.push_back(f.filename);
            pipelines.push_back(pipeline(&tile, resolution, pointsThreshold));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        pipelines.push_back(pipeline(&tile, resolution, pointsThreshold));
    }
}

static std::string extractPolygon(PipelineManager &pipeline)
{
    pdal::MetadataNode mn = pipeline.getMetadata();
    //Utils::toJSON(mn, std::cout);

    pdal::MetadataNode hb = mn.findChild("filters.hexbin").findChild("boundary");
    //Utils::toJSON(hb, std::cout);
    return hb.value();
}

void Boundary::finalize(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
{
    if (pipelines.empty())
        return;

    GDALAllRegister();

    OGRSpatialReferenceH hSrs;
    hSrs = OSRNewSpatialReference(crs.getWKT().c_str());
    assert(hSrs);

    OGRwkbGeometryType wkbType = /*wkt.find("MULTI") == std::string::npos ? wkbPolygon :*/ wkbMultiPolygon;

    OGRSFDriverH hDriver = OGRGetDriverByName("GPKG");
    if (hDriver == nullptr)
    {
        std::cerr << "Failed to create GPKG driver" << std::endl;
        return;
    }

    GDALDatasetH hDS = GDALCreate( hDriver, outputFile.c_str(), 0, 0, 0, GDT_Unknown, nullptr );
    if (hDS == nullptr)
    {
        std::cerr << "Failed to create output file: " << outputFile << std::endl;
        return;
    }

    OGRLayerH hLayer = GDALDatasetCreateLayer( hDS, "boundary", hSrs, wkbType, nullptr );
    if (hLayer == nullptr)
    {
        std::cerr << "Failed to create layer in the output file: " << outputFile << std::endl;
        return;
    }

    // TODO: to a union of boundary polygons

    for (auto& pipePtr : pipelines)
    {
        // extract boundary polygon
        std::string wkt = extractPolygon(*pipePtr.get());

        OGRGeometryH geom;
        char *wkt_ptr = wkt.data();
        if (OGR_G_CreateFromWkt(&wkt_ptr, hSrs, &geom) != OGRERR_NONE)
        {
            std::cerr << "Failed to parse geometry: " << wkt << std::endl;
        }
        if ( wkbFlatten(OGR_G_GetGeometryType(geom)) == wkbPolygon)
        {
            // this function takes ownership of "geom" and then creates new instance
            geom = OGR_G_ForceToMultiPolygon(geom);
        }
        OGRFeatureH hFeature = OGR_F_Create(OGR_L_GetLayerDefn(hLayer));
        if (OGR_F_SetGeometryDirectly(hFeature, geom) != OGRERR_NONE)
        {
            std::cerr << "Could not set geometry " << wkt << std::endl;
        }
        if (OGR_L_CreateFeature(hLayer, hFeature) != OGRERR_NONE)
        {
            std::cerr << "Failed to create a new feature in the output file!" << std::endl;
        }

        OGR_F_Destroy(hFeature);
    }

    OSRDestroySpatialReference(hSrs);
    GDALClose(hDS);
}
