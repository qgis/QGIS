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

#include "utils.hpp"

#include <filesystem>
#include <iostream>
#include <chrono>

#include <pdal/PipelineManager.hpp>
#include <pdal/Stage.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/util/ThreadPool.hpp>

#include <gdal_utils.h>

using namespace pdal;


static ProgressBar sProgressBar;


// Table subclass that also takes care of updating progress in streaming pipelines
class MyTable : public FixedPointTable
{
public:
    MyTable(point_count_t capacity) : FixedPointTable(capacity) {}

protected:
    virtual void reset()
    {
        sProgressBar.add();
        FixedPointTable::reset();
    }
};


std::string box_to_pdal_bounds(const BOX2D &box)
{
    std::ostringstream oss;
    oss << std::fixed << "([" << box.minx << "," << box.maxx << "],[" << box.miny << "," << box.maxy << "])";
    return oss.str();  // "([xmin, xmax], [ymin, ymax])"
}


QuickInfo getQuickInfo(std::string inputFile)
{
    // TODO: handle the case when driver is not inferred
    std::string driver = StageFactory::inferReaderDriver(inputFile);
    if (driver.empty())
    {
        std::cerr << "Could not infer driver for input file: " << inputFile << std::endl;
        return QuickInfo();
    }

    StageFactory factory;
    Stage *reader = factory.createStage(driver);  // reader is owned by the factory
    pdal::Options opts;
    opts.add("filename", inputFile);
    reader->setOptions(opts);
    return reader->preview();

    // PipelineManager m;
    // Stage &r = m.makeReader(inputFile, "");
    // return r.preview().m_pointCount;
}

MetadataNode getReaderMetadata(std::string inputFile, MetadataNode *pointLayoutMeta)
{
    // compared to quickinfo / preview, this provides more info...

    PipelineManager m;
    Stage &r = m.makeReader(inputFile, "");
    FixedPointTable table(10000);
    r.prepare(table);
    if (pointLayoutMeta)
    {
        *pointLayoutMeta = table.layout()->toMetadata();
    }
    return r.getMetadata();
}

#define CHUNK_SIZE 100000

void runPipelineParallel(point_count_t totalPoints, bool isStreaming, std::vector<std::unique_ptr<PipelineManager>>& pipelines, int max_threads, bool verbose)
{
    int num_chunks = totalPoints / CHUNK_SIZE;

    if (verbose)
    {
        std::cout << "total points: " << (float)totalPoints / 1'000'000 << "M" << std::endl;

        std::cout << "jobs " << pipelines.size() << std::endl;
        std::cout << "max threads " << max_threads << std::endl;
        if (!isStreaming)
            std::cout << "running in non-streaming mode!" << std::endl;
    }

    auto start = std::chrono::high_resolution_clock::now();

    sProgressBar.init(isStreaming ? num_chunks : pipelines.size());

    int nThreads = (std::min)( (int)pipelines.size(), max_threads );
    ThreadPool p(nThreads);
    for (size_t i = 0; i < pipelines.size(); ++i)
    {
        PipelineManager* pipeline = pipelines[i].get();
        if (isStreaming)
        {
            p.add([pipeline]() {

                MyTable table(CHUNK_SIZE);
                pipeline->executeStream(table);

            });
        }
        else
        {
            p.add([pipeline, &pipelines, i]() {
                pipeline->execute();
                pipelines[i].reset();  // to free the point table and views (meshes, rasters)
                sProgressBar.add();
            });
        }
    }

    //std::cout << "starting to wait" << std::endl;

    // while (p.tasksInQueue() + p.tasksInProgress())
    // {
    //     //std::cout << "progress: " << p.tasksInQueue() << " " << p.tasksInProgress() << " cnt " << cntPnt/1'000 << std::endl;
    //     std::this_thread::sleep_for(500ms);
    // }

    p.join();

    sProgressBar.done();

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    if (verbose)
    {
        std::cout << "time " << duration.count()/1000. << " s" << std::endl;
    }
}


static GDALDatasetH rasterTilesToVrt(const std::vector<std::string> &inputFiles, const std::string &outputVrtFile)
{
    // build a VRT so that all tiles can be handled as a single data source

    std::vector<const char*> dsNames;
    for ( const std::string &t : inputFiles )
    {
        dsNames.push_back(t.c_str());
    }

    // https://gdal.org/api/gdal_utils.html
    GDALDatasetH ds = GDALBuildVRT(outputVrtFile.c_str(), (int)dsNames.size(), nullptr, dsNames.data(), nullptr, nullptr);
    return ds;
}

static bool rasterVrtToCog(GDALDatasetH ds, const std::string &outputFile)
{
    const char* args[] = { "-of", "COG", "-co", "COMPRESS=DEFLATE", NULL };
    GDALTranslateOptions* psOptions = GDALTranslateOptionsNew((char**)args, NULL);

    GDALDatasetH dsFinal = GDALTranslate(outputFile.c_str(), ds, psOptions, nullptr);
    GDALTranslateOptionsFree(psOptions);
    if (!dsFinal)
        return false;
    GDALClose(dsFinal);
    return true;
}

bool rasterTilesToCog(const std::vector<std::string> &inputFiles, const std::string &outputFile)
{
    std::string outputVrt = outputFile;
    assert(ends_with(outputVrt, ".tif"));
    outputVrt.erase(outputVrt.rfind(".tif"), 4);
    outputVrt += ".vrt";

    GDALDatasetH ds = rasterTilesToVrt(inputFiles, outputVrt);

    if (!ds)
        return false;

    rasterVrtToCog(ds, outputFile);
    GDALClose(ds);

    std::filesystem::remove(outputVrt);

    return true;
}

bool readerSupportsBounds(Stage &reader)
{
    // these readers support "bounds" option with a 2D/3D bounding box, and based
    // on it, they will do very efficient reading of data and only return what's
    // in the given bounding box
    return reader.getName() == "readers.copc" || reader.getName() == "readers.ept";
}

bool allReadersSupportBounds(const std::vector<Stage *> &readers)
{
    for (Stage *r : readers)
    {
        if (!readerSupportsBounds(*r))
            return false;
    }
    return true;
}

pdal::Bounds parseBounds(const std::string &boundsStr)
{
    // if the input string is not a correct 2D/3D PDAL bounds then parse()
    // will throw an exception
    pdal::Bounds b;
    std::string::size_type pos(0);
    b.parse(boundsStr, pos);
    return b;
}

BOX2D intersectionBox2D(const BOX2D &b1, const BOX2D &b2)
{
    BOX2D b;
    b.minx = b1.minx > b2.minx ? b1.minx : b2.minx;
    b.miny = b1.miny > b2.miny ? b1.miny : b2.miny;
    b.maxx = b1.maxx < b2.maxx ? b1.maxx : b2.maxx;
    b.maxy = b1.maxy < b2.maxy ? b1.maxy : b2.maxy;
    if (b.minx > b.maxx || b.miny > b.maxy)
        return BOX2D();
    return b;
}


BOX2D intersectTileBoxWithFilterBox(const BOX2D &tileBox, const BOX2D &filterBox)
{
    if (tileBox.valid() && filterBox.valid())
    {
        return intersectionBox2D(tileBox, filterBox);
    }
    else if (tileBox.valid())
    {
        return tileBox;
    }
    else if (filterBox.valid())
    {
        return filterBox;
    }
    else
    {
        return BOX2D();  // invalid box
    }
}
