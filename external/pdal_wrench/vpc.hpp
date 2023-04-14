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

#include <pdal/pdal_types.hpp>
#include <pdal/util/Bounds.hpp>

using namespace pdal;


void buildVpc(std::vector<std::string> args);


struct VirtualPointCloud
{
    struct SchemaItem
    {
      SchemaItem(const std::string &n, const std::string &t, int s): name(n), type(t), size(s) {}

      std::string name;
      std::string type;
      int size;
    };

    struct File
    {
        std::string filename;
        point_count_t count;
        BOX3D bbox;
        std::string crsWkt;
        std::string datetime;  // RFC 3339 encoded date/time - e.g. 2023-01-01T12:00:00Z
        std::vector<SchemaItem> schema;  // we're not using it, just for STAC export
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
