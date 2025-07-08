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

    pdal::Options writer_opts;
    writer_opts.add(pdal::Option("ogrdriver", "GPKG"));
    if (!attributes.empty())
        writer_opts.add(pdal::Option("attr_dims", join_strings(attributes, ',')));
    (void)manager->makeWriter( tile->outputFilename, "writers.ogr", *last, writer_opts);

    return manager;
}


void ToVector::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>& pipelines)
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
            ParallelJobInfo tile(ParallelJobInfo::FileBased, BOX2D(), filterExpression, filterBounds);
            tile.inputFilenames.push_back(f.filename);

            // for input file /x/y/z.las that goes to /tmp/hello.vpc,
            // individual output file will be called /tmp/hello/z.las
            fs::path inputBasename = fileStem(f.filename);
            tile.outputFilename = (outputSubdir / inputBasename).string() + ".gpkg";

            tileOutputFiles.push_back(tile.outputFilename);

            pipelines.push_back(pipeline(&tile, attributes));
        }
    }
    else
    {
        ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression, filterBounds);
        tile.inputFilenames.push_back(inputFile);
        tile.outputFilename = outputFile;
        pipelines.push_back(pipeline(&tile, attributes));
    }
}

void ToVector::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (tileOutputFiles.empty())
        return;

    if (ends_with(inputFile, ".vpc"))
    {
        // for /tmp/hello.vpc we will use /tmp/hello dir for all results
        fs::path outputParentDir = fs::path(outputFile).parent_path();
        fs::path outputSubdir = outputParentDir / fs::path(outputFile).stem();
        fs::path vrtFile = outputSubdir / "all.vrt";
        
        // remove vrt if it exist
        if (fs::exists(vrtFile))
        {
            fs::remove(vrtFile);
        }

        std::ofstream outputStreamFile(vrtFile);
        
        // check if file opened successfully and write VRT content to it
        if (outputStreamFile.is_open()) {
            
            outputStreamFile << "<OGRVRTDataSource>" << std::endl;
            outputStreamFile << "<OGRVRTUnionLayer name=\"points\">" << std::endl;

            for (const std::string& f : tileOutputFiles)
            {   
                outputStreamFile << "<OGRVRTLayer name=\"" << fs::path(f).stem().string() << "\">" << std::endl;
                outputStreamFile << "<SrcDataSource>" << f <<  "</SrcDataSource>" << std::endl;
                outputStreamFile << "<SrcLayer>points</SrcLayer>" << std::endl;
                outputStreamFile << "</OGRVRTLayer>" << std::endl;
            }

            outputStreamFile << "</OGRVRTUnionLayer>" << std::endl;
            outputStreamFile << "</OGRVRTDataSource>" << std::endl;

            // close file
            outputStreamFile.close();
        }
        else 
        {
            std::cerr << "Failed to open VRT file for writing: " << vrtFile.string() << std::endl;
            return;
        }

        // options for translate - empty
        GDALVectorTranslateOptions *options = GDALVectorTranslateOptionsNew(nullptr, NULL);

        // open VRT file and check it
        GDALDatasetH vrtDs = GDALOpenEx(vrtFile.string().c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL);
        if (!vrtDs)
        {
            std::cerr << "Failed to open composite VRT file" << std::endl;
            fs::remove(vrtFile);
            return;
        }

        // translate to resulting file and check it
        GDALDatasetH resultDS = GDALVectorTranslate(outputFile.c_str(), nullptr, 1, (GDALDatasetH *)&vrtDs, options, nullptr);
        if (!resultDS)
        {
            std::cerr << "Failed to create output file" << std::endl;
            GDALClose(vrtDs);
            fs::remove(vrtFile);
            return;
        }
        
        // close datasets
        GDALClose(vrtDs);
        GDALClose(resultDS);

        // delete temporary files 
        for (const std::string& f : tileOutputFiles)
        {
            fs::remove(f);
        }

        // delete vrt file
        fs::remove(vrtFile);

        // delete dir if empty
        if (fs::is_empty(outputSubdir))
        {
            fs::remove(outputSubdir);
        }
    }
}
