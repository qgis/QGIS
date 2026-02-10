/*****************************************************************************
 *   Copyright (c) 2026, Lutra Consulting Ltd. and Hobu, Inc.                *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <filesystem>
#include <iostream>
#include <thread>

#include <pdal/PipelineManager.hpp>
#include <pdal/Polygon.hpp>
#include <pdal/Stage.hpp>
#include <pdal/pdal_types.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include <gdal_utils.h>

#include "alg.hpp"
#include "utils.hpp"
#include "vpc.hpp"

using namespace pdal;

void ComparePointClouds::addArgs() 
{
  argOutput = &programArgs.add("output,o", "Output point cloud file", outputFile);
  argComparedInputFile = &programArgs.add("input-compare", "Point cloud file to compare against input", comparedInputFile);
  
  programArgs.add("subsampling-cell-size", "Minimum spacing between points", subsamplingCellSize, 0.0);

  programArgs.add( "normal-radius", "Radius of the sphere around each core point that defines the neighbors from which normals are calculated.", normalRadius, 2.0);
  programArgs.add("cyl-radius", "Radius of the cylinder inside of which points are searched for when calculating change", cylRadius, 2.0);
  programArgs.add("cyl-halflen", "The half-length of the cylinder of neighbors used for calculating change", cylHalflen, 5.0);
  programArgs.add("reg-error", "Registration error", regError, 0.0);
  argOrientation = &programArgs.add( "cyl-orientation", "Which direction to orient the cylinder/normal vector used for comparison between the two point clouds. (up, origin, none)", cylOrientation, "up");
}

bool ComparePointClouds::checkArgs() 
{

  if (ends_with(inputFile, ".vpc")) {
    std::cerr << "input cannot be a VPC file" << std::endl;
    return false;
  }

  if (ends_with(comparedInputFile, ".vpc")) {
    std::cerr << "compared input cannot be a VPC file" << std::endl;
    return false;
  }

  if (!argOutput->set()) {
    std::cerr << "missing output" << std::endl;
    return false;
  }

  if (!argComparedInputFile->set()) {
    std::cerr << "missing compared input file" << std::endl;
    return false;
  }

  if (argOrientation->set()) {
    if (cylOrientation != "up" && cylOrientation != "origin" &&
        cylOrientation != "none") {
      std::cerr << "unknown orientation: " << cylOrientation << std::endl;
      return false;
    }
  }

  return true;
}

static std::unique_ptr<PipelineManager> pipeline(ParallelJobInfo *tile, std::string compareFile, double subsamplingCellSize, double normalRadius, double cylRadius, double cylHalflen, double regError, std::string cylOrientation)
{
  std::unique_ptr<PipelineManager> manager(new PipelineManager);

  Stage &reader1 = makeReader(manager.get(), tile->inputFilenames[0]);
  Stage *last = &reader1;

  // filtering
  if (!tile->filterBounds.empty())
  {
      Options filter_opts;
      filter_opts.add(pdal::Option("bounds", tile->filterBounds));

      if (readerSupportsBounds(reader1))
      {
          // Reader of the format can do the filtering - use that whenever possible!
          reader1.addOptions(filter_opts);
      }
      else
      {
          // Reader can't do the filtering - do it with a filter
          last = &manager->makeFilter("filters.crop", *last, filter_opts);
      }
  }

  if (!tile->filterExpression.empty())
  {
      Options filter_opts;
      filter_opts.add(pdal::Option("expression", tile->filterExpression));
      last = &manager->makeFilter("filters.expression", *last, filter_opts);
  }     
  
  Stage *reader1FilteredCropped = last;

  Stage &reader2 = makeReader(manager.get(), compareFile);
  last = &reader2;

  // filtering
  if (!tile->filterBounds.empty())
  {
      Options filter_opts;
      filter_opts.add(pdal::Option("bounds", tile->filterBounds));

      if (readerSupportsBounds(reader2))
      {
          // Reader of the format can do the filtering - use that whenever possible!
          reader2.addOptions(filter_opts);
      }
      else
      {
          // Reader can't do the filtering - do it with a filter
          last = &manager->makeFilter("filters.crop", *last, filter_opts);
      }
  }

  if (!tile->filterExpression.empty())
  {
      Options filter_opts;
      filter_opts.add(pdal::Option("expression", tile->filterExpression));
      last = &manager->makeFilter("filters.expression", *last, filter_opts);
  }
  Stage *reader2FilteredCropped = last;

  Stage *corePoints = nullptr;
  if (subsamplingCellSize != 0.0)
  {
    pdal::Options sample_opts;
    sample_opts.add(pdal::Option("cell", subsamplingCellSize));
    corePoints = &manager->makeFilter("filters.sample", *reader1FilteredCropped, sample_opts);
  }
  else
  {
    corePoints = reader1FilteredCropped;
  }

  Options compare_opts;
  compare_opts.add(pdal::Option("normal_radius", normalRadius));
  compare_opts.add(pdal::Option("cyl_radius", cylRadius));
  compare_opts.add(pdal::Option("cyl_halflen", cylHalflen));
  compare_opts.add(pdal::Option("reg_error", regError));
  compare_opts.add(pdal::Option("orientation", cylOrientation));

  Stage *filterM3c2 = &manager->makeFilter("filters.m3c2", compare_opts);

  // inputs to M3C2 filter are origin file, compared file and core points (in order)
  filterM3c2->setInput(*reader1FilteredCropped);
  filterM3c2->setInput(*reader2FilteredCropped);
  filterM3c2->setInput(*corePoints);
  
  makeWriter(manager.get(), tile->outputFilename, filterM3c2);

  return manager;
}

void ComparePointClouds::preparePipelines(std::vector<std::unique_ptr<PipelineManager>> &pipelines)
{
  ParallelJobInfo tile(ParallelJobInfo::Single, BOX2D(), filterExpression,
                       filterBounds);
  tile.inputFilenames.push_back(inputFile);
  tile.outputFilename = outputFile;
  pipelines.push_back(pipeline(&tile, comparedInputFile, subsamplingCellSize, normalRadius, cylRadius, cylHalflen, regError, cylOrientation));
}

void ComparePointClouds::finalize( std::vector<std::unique_ptr<PipelineManager>> &)
{
}
