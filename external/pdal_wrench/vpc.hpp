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

#include <vector>
#include <string>

#include <pdal/Geometry.hpp>
#include <pdal/pdal_types.hpp>
#include <pdal/util/Bounds.hpp>

using namespace pdal;


void buildVpc(std::vector<std::string> args);


struct VirtualPointCloud
{
    //! Schema for a single attribute
    struct SchemaItem
    {
      SchemaItem(const std::string &n, const std::string &t, int s): name(n), type(t), size(s) {}

      std::string name;
      std::string type;
      int size;
    };

    //! Stats for a single attribute
    struct StatsItem
    {
        StatsItem(const std::string &n, uint32_t p, double a, point_count_t c, double max, double min, double st, double vr)
          : name(n), position(p), average(a), count(c), maximum(max), minimum(min), stddev(st), variance(vr) {}

        std::string name;
        uint32_t position;
        double average;
        point_count_t count;
        double maximum;
        double minimum;
        double stddev;
        double variance;
    };

    struct File
    {
        std::string filename;
        point_count_t count;
        std::string boundaryWkt;   // not pdal::Geometry because of https://github.com/PDAL/PDAL/issues/4016
        BOX3D bbox;
        std::string crsWkt;
        std::string datetime;  // RFC 3339 encoded date/time - e.g. 2023-01-01T12:00:00Z
        std::vector<SchemaItem> schema;  // we're not using it, just for STAC export
        std::vector<StatsItem> stats;

        // support for overview point clouds - currently we assume a file refers to at most a single overview file
        // (when building VPC with overviews, we create one overview file for all source data)
        std::string overviewFilename;
    };

    std::vector<File> files;
    std::string crsWkt;  // valid WKT for CRS of all files (or empty string if undefined, or "_mix_" if a mixture of CRS was seen)

    void clear();
    void dump();
    bool read(std::string filename);
    bool write(std::string filename);

    point_count_t totalPoints() const;
    BOX3D box3d() const;

    //! returns files that have bounding box overlapping the given bounding box
    std::vector<File> overlappingBox2D(const BOX2D &box);
};
