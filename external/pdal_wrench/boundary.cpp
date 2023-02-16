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

#include <gdal/gdal.h>
#include <gdal/ogr_api.h>
#include <gdal/ogr_srs_api.h>

#include "utils.hpp"
#include "alg.hpp"
#include "vpc.hpp"

using namespace pdal;


void Boundary::addArgs()
{
    argOutput = &programArgs.add("output,o", "Output vector file", outputFile);
}

bool Boundary::checkArgs()
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
    assert(tile->inputFilenames.size() == 1);

    std::unique_ptr<PipelineManager> manager( new PipelineManager );

    Stage& r = manager->makeReader(tile->inputFilenames[0], "");

    // TODO: what edge size? (by default samples 5000 points if not specified
    // TODO: set threshold ? (default at least 16 points to keep the cell)
    // btw. if threshold=0, there are still missing points because of simplification (smooth=True)

    pdal::Options hexbin_opts;
    hexbin_opts.add(pdal::Option("edge_size", 5));
    hexbin_opts.add(pdal::Option("threshold", 0));

    if (!tile->filterExpression.empty())
    {
        hexbin_opts.add(pdal::Option("where", tile->filterExpression));
    }

    Stage& w = manager->makeFilter( "filters.hexbin", r, hexbin_opts );
    return manager;
}

void Boundary::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines, const BOX3D &bounds, point_count_t &totalPoints)
{
    if (ends_with(inputFile, ".vpc"))
    {
        // VPC handling
        VirtualPointCloud vpc;
        if (!vpc.read(inputFile))
            return;

        for (const VirtualPointCloud::File& f : vpc.files)
        {
            ParallelJobInfo tile(ParallelJobInfo::FileBased, BOX2D(), filterExpression);
            tile.inputFilenames.push_back(f.filename);
            pipelines.push_back(pipeline(&tile));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression);
        tile.inputFilenames.push_back(inputFile);
        pipelines.push_back(pipeline(&tile));
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

    // TODO: may be copc reader too... we could get input's CRS from QuickInfo
    PipelineManager *pm = pipelines[0].get();
    std::string crs_wkt = pm->getMetadata().findChild("readers.las").findChild("spatialreference").value();

    GDALAllRegister();

    OGRSpatialReferenceH hSrs;
    hSrs = OSRNewSpatialReference(crs_wkt.c_str());
    assert(hSrs);

    OGRwkbGeometryType wkbType = /*wkt.find("MULTI") == std::string::npos ? wkbPolygon :*/ wkbMultiPolygon;

    OGRSFDriverH hDriver = OGRGetDriverByName("GPKG");
    if (hDriver == nullptr)
    {
        std::cout << "failed to create GPKG driver" << std::endl;
        return;
    }

    GDALDatasetH hDS = GDALCreate( hDriver, outputFile.c_str(), 0, 0, 0, GDT_Unknown, nullptr );
    if (hDS == nullptr)
    {
        std::cout << "failed to create output file: " << outputFile << std::endl;
        return;
    }

    OGRLayerH hLayer = GDALDatasetCreateLayer( hDS, "boundary", hSrs, wkbType, nullptr );
    if (hLayer == nullptr)
    {
        std::cout << "failed to create layer in the output file: " << outputFile << std::endl;
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
            std::cout << "Failed to parse geometry: " << wkt << std::endl;
        }
        OGRFeatureH hFeature = OGR_F_Create(OGR_L_GetLayerDefn(hLayer));
        if (OGR_F_SetGeometry(hFeature, geom) != OGRERR_NONE)
        {
            std::cout << "couldn't set geometry " << wkt << std::endl;
        }
        if (OGR_L_CreateFeature(hLayer, hFeature) != OGRERR_NONE)
        {
            std::cout << "failed to create a new feature in the output file!" << std::endl;
        }

        OGR_F_Destroy(hFeature);
    }

    OSRDestroySpatialReference(hSrs);
    GDALClose(hDS);
}
