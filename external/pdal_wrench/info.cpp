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


void Info::addArgs()
{
}

bool Info::checkArgs()
{
    return true;
}


// extract information from WKT - write to "crs" and "units" strings
static void formatCrsInfo(const std::string &crsWkt, std::string &crs, std::string &units)
{
  if (crsWkt.empty())
  {
      crs = "(unknown)";
      units = "(unknown)";
  }
  else if (crsWkt == "_mix_")
  {
      // this can only happen in case of VPC - it is our special marker
      // when there are two or more different CRS in the input files
      crs = "(multiple)";
      units = "(unknown)";
  }
  else
  {
      // this code is quite clumsy... most of what we need is handled by pdal::SpatialReference, but:
      // - there's no API to get human readable name of the CRS
      // - https://github.com/PDAL/PDAL/issues/3943 - SpatialReference::identifyVerticalEPSG() is broken
      // - https://github.com/PDAL/PDAL/issues/3946 - SpatialReference::getHorizontalUnits() may return incorrect units
      pdal::SpatialReference sr(crsWkt);
      std::string wktHoriz = sr.getHorizontal();
      std::string wktVert = sr.getVertical();
      CRS c(crsWkt);
      CRS ch(wktHoriz);
      CRS cv(wktVert);
      std::string crsEpsg = c.identifyEPSG();
      std::string crsHorizEpsg = sr.identifyHorizontalEPSG();
      std::string crsVertEpsg = cv.identifyEPSG();

      // CRS description and EPSG codes if available
      crs = c.name();
      if (!crsEpsg.empty())
        crs += " (EPSG:" + crsEpsg + ")";
      else if (!crsHorizEpsg.empty() && !crsVertEpsg.empty())
        crs += " (EPSG:" + crsHorizEpsg + "+" + crsVertEpsg + ")";   // this syntax of compound EPSG codes is understood by PROJ too
      else if (!crsHorizEpsg.empty())
        crs += " (EPSG:" + crsHorizEpsg +  " + ?)";

      if (wktVert.empty())
      {
        crs += "  (vertical CRS missing!)";
      }

      // horizontal and vertical units
      std::string unitsHoriz = ch.units();
      std::string unitsVert = cv.units();
      if (unitsHoriz == unitsVert)
        units = unitsHoriz;
      else
      {
        units = "horizontal=" + unitsHoriz + "  vertical=" + unitsVert;
      }
      // TODO: add a warning when horizontal and vertical units do not match (?)
  }

}


void Info::preparePipelines(std::vector<std::unique_ptr<PipelineManager>>&)
{
    if (ends_with(inputFile, ".vpc"))
    {
        VirtualPointCloud vpc;
        if (!vpc.read(inputFile))
            return;

        // TODO: other global metadata

        point_count_t total = 0;
        BOX3D box = !vpc.files.empty() ? vpc.files[0].bbox : BOX3D();
        for (const VirtualPointCloud::File& f : vpc.files)
        {
            total += f.count;
            box.grow(f.bbox);
        }

        std::string crsName;
        std::string units;
        formatCrsInfo(vpc.crsWkt, crsName, units);

        std::cout << "VPC           " << vpc.files.size() << " files" << std::endl;
        std::cout << "count         " << total << std::endl;
        std::cout << "extent        " << box.minx << " " << box.miny << " " << box.minz << std::endl;
        std::cout << "              " << box.maxx << " " << box.maxy << " " << box.maxz << std::endl;
        std::cout << "crs           " << crsName << std::endl;
        std::cout << "units         " << units << std::endl;

        // list individual files
        std::cout << std::endl << "Files:" << std::endl;
        for (const VirtualPointCloud::File& f : vpc.files)
        {
            // TODO: maybe add more info?
            std::cout << f.filename << std::endl;
        }

        // TODO: optionally run stats on the whole VPC
    }
    else
    {
        MetadataNode layout;
        MetadataNode meta = getReaderMetadata(inputFile, &layout);

        std::string crs;
        std::string units;
        std::string crsWkt = meta.findChild("srs").findChild("compoundwkt").value();
        formatCrsInfo(crsWkt, crs, units);

        std::cout << "LAS           " << meta.findChild("major_version").value() << "." << meta.findChild("minor_version").value() << std::endl;
        std::cout << "point format  " << meta.findChild("dataformat_id").value() << std::endl;
        std::cout << "count         " << meta.findChild("count").value() << std::endl;
        std::cout << "scale         " << meta.findChild("scale_x").value() << " " << meta.findChild("scale_y").value() << " " << meta.findChild("scale_z").value() << std::endl;
        std::cout << "offset        " << meta.findChild("offset_x").value() << " " << meta.findChild("offset_y").value() << " " << meta.findChild("offset_z").value() << std::endl;
        std::cout << "extent        " << meta.findChild("minx").value() << " " << meta.findChild("miny").value() << " " << meta.findChild("minz").value() << std::endl;
        std::cout << "              " << meta.findChild("maxx").value() << " " << meta.findChild("maxy").value() << " " << meta.findChild("maxz").value() << std::endl;
        std::cout << "crs           " << crs << std::endl;
        std::cout << "units         " << units << std::endl;
        // TODO: file size in MB ?

        // TODO: possibly show extra metadata: (probably --verbose mode)
        // - creation date + software ID + system ID
        // - filesource ID
        // - VLR info

        std::cout << std::endl << "Attributes:" << std::endl;
        MetadataNodeList dims = layout.children("dimensions");
        for (auto &dim : dims)
        {
            std::string name = dim.findChild("name").value();
            std::string type = dim.findChild("type").value();
            int size = dim.findChild("size").value<int>();
            std::cout << " - " << name << " " << type << " " << size << std::endl;
        }

        // TODO: optionally run filters.stats to get basic stats + counts of classes, returns, ...
    }
}

void Info::finalize(std::vector<std::unique_ptr<PipelineManager>>&)
{
}
