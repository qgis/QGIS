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
#include <mutex>

using namespace pdal;

// tiling scheme containing tileCountX x tileCountY square tiles of tileSize x tileSize,
// with lower-left corner of the tiling being at [tileStartX,tileStartY]
struct Tiling
{
    int tileCountX;
    int tileCountY;
    double tileStartX;
    double tileStartY;
    double tileSize;

    BOX2D fullBox() const
    {
        return BOX2D(tileStartX,
                     tileStartY,
                     tileStartX + tileSize * tileCountX,
                     tileStartY + tileSize * tileCountY);
    }

    BOX2D boxAt(int ix, int iy) const
    {
        return BOX2D(tileStartX + tileSize*ix,
                     tileStartY + tileSize*iy,
                     tileStartX + tileSize*(ix+1),
                     tileStartY + tileSize*(iy+1));
    }
};

// specification that square tiles of tileSize x tileSize should be aligned:
// so that all corners have coordinates [originX + N*tileSize, originY + M*tileSize]
// where N,M are some integer values
struct TileAlignment
{
    double originX;
    double originY;
    double tileSize;

    // returns tiling that fully covers given bounding box, using this tile alignment
    Tiling coverBounds(const BOX2D &box) const
    {
        Tiling t;
        t.tileSize = tileSize;
        double offsetX = fmod(originX, tileSize);
        double offsetY = fmod(originY, tileSize);
        t.tileStartX = floor((box.minx - offsetX)/tileSize)*tileSize + offsetX;
        t.tileStartY = floor((box.miny - offsetY)/tileSize)*tileSize + offsetY;
        t.tileCountX = ceil((box.maxx - t.tileStartX)/tileSize);
        t.tileCountY = ceil((box.maxy - t.tileStartY)/tileSize);
        return t;
    }
};

struct ParallelJobInfo
{
    enum ParallelMode {
        Single,      //!< no parallelism
        FileBased,   //!< each input file processed separately
        Spatial,     //!< using tiles - "box" should be used
    } mode;

    ParallelJobInfo(ParallelMode m = Single): mode(m) {}
    ParallelJobInfo(ParallelMode m, const BOX2D &b, const std::string fe, const std::string fb)
      : mode(m), box(b), filterExpression(fe), filterBounds(fb) {}

    // what input point cloud files to read for a job
    std::vector<std::string> inputFilenames;

    // what is the output file name of this job
    std::string outputFilename;

    // bounding box for this job (for input/output)
    BOX2D box;

    // PDAL filter expression to apply on all pipelines
    std::string filterExpression;

    // PDAL filter on 2D or 3D bounds to apply on all pipelines
    // Format is "([xmin, xmax], [ymin, ymax])" or "([xmin, xmax], [ymin, ymax], [zmin, zmax])"
    std::string filterBounds;

    // modes of operation:
    // A. multi input without box  (LAS/LAZ)    -- per file strategy
    //    - all input files are processed, no filtering on bounding box
    // B. multi input with box     (anything)   -- tile strategy
    //    - all input files are processed, but with filtering applied
    //    - COPC: filtering inside readers.copc with "bounds" argument
    //    - LAS/LAZ: filter either using CropFilter after reader -or- "where"

    // streaming algs:
    // - multi-las: if not overlapping:  mode A
    //              if overlapping:      mode A - with a warning it is inefficient?
    // - multi-copc:  mode B
    // - single-copc: mode B or just single pipeline
};



#include <ogr_spatialref.h>

// few CRS-related functions that cover in addition to what pdal::SpatialReference doess not provide
struct CRS
{
public:
  // construct CRS using a well-known text definition (WKT)
  CRS(std::string s = "")
  {
    ptr.reset( static_cast<OGRSpatialReference*>(OSRNewSpatialReference(s.size() ? s.c_str() : nullptr)) );
  }

  std::string name() { return ptr ? ptr->GetName() : ""; }

  // workaround for https://github.com/PDAL/PDAL/issues/3943
  std::string identifyEPSG()
  {
    if (!ptr)
        return "";

    if (const char* c = ptr->GetAuthorityCode(nullptr))
        return std::string(c);

    if (ptr->AutoIdentifyEPSG() == OGRERR_NONE)
    {
        if (const char* c = ptr->GetAuthorityCode(nullptr))
            return std::string(c);
    }

    return "";
  }

  // workaround for https://github.com/PDAL/PDAL/issues/3946
  std::string units()
  {
    if (!ptr)
        return std::string();

    const char* units(nullptr);

    // The returned value remains internal to the OGRSpatialReference
    // and should not be freed, or modified. It may be invalidated on
    // the next OGRSpatialReference call.
    (void)ptr->GetLinearUnits(&units);
    std::string tmp(units);
    Utils::trim(tmp);
    return tmp;
  }

private:
  struct OGRDeleter
  {
      void operator()(OGRSpatialReference* o)
      {
          OSRDestroySpatialReference(o);
      };
  };

  std::unique_ptr<OGRSpatialReference, OGRDeleter> ptr;
};


// GDAL-style progress bar:
// 0...10...20...30...40...50...60...70...80...90...100 - done.
struct ProgressBar
{
private:
  uint64_t total = 0;
  uint64_t current = 0;
  int last_percent = -1;  // -1 = not started, 0-50 means 0-100 percent
  std::mutex mutex;

public:
  void init(uint64_t tot)
  {
    total = tot;
    current = 0;
    last_percent = -1;
    add(0);
  }

  void add(uint64_t count = 1)
  {
    mutex.lock();
    current += count;

    int new_percent = (int)std::round((std::min)(1.0,((double)current/(double)total))*100)/2;
    while (new_percent > last_percent)
    {
        ++last_percent;
        if (last_percent % 5 == 0)
            std::cout << last_percent*2 << std::flush;
        else
            std::cout << "." << std::flush;
    }
    mutex.unlock();
  }

  void done()
  {
    std::cout << " - done." << std::endl;
  }
};


QuickInfo getQuickInfo(std::string inputFile);

MetadataNode getReaderMetadata(std::string inputFile, MetadataNode *pointLayoutMeta = nullptr);

void runPipelineParallel(point_count_t totalPoints, bool isStreaming, std::vector<std::unique_ptr<PipelineManager>>& pipelines, int max_threads, bool verbose);

std::string box_to_pdal_bounds(const BOX2D &box);

pdal::Bounds parseBounds(const std::string &boundsStr);

bool readerSupportsBounds(Stage &reader);

bool allReadersSupportBounds(const std::vector<Stage *> &readers);

BOX2D intersectionBox2D(const BOX2D &b1, const BOX2D &b2);

BOX2D intersectTileBoxWithFilterBox(const BOX2D &tileBox, const BOX2D &filterBox);


inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


inline std::string join_strings(const std::vector<std::string>& list, char delimiter)
{
    std::string output;
    for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        output += *it;
        if (it != list.end() - 1)
            output += delimiter;
    }
    return output;
}


bool rasterTilesToCog(const std::vector<std::string> &inputFiles, const std::string &outputFile);
