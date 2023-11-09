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

/*
TODO:
- algs that output point cloud: support multi-copc or single-copc output - as a post-processing step?
- VPC: overlapping files? do not allow - require tiling
*/

#include <iostream>
#include <vector>

#include <pdal/util/FileUtils.hpp>

#include "alg.hpp"
#include "vpc.hpp"

extern int runTile(std::vector<std::string> arglist);  // tile/tile.cpp


void printUsage()
{
    std::cout << "usage: pdal_wrench <command> [<args>]" << std::endl;
    std::cout << "       pdal_wrench [--help]" << std::endl;
    std::cout << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "   boundary        Exports a polygon file containing boundary" << std::endl;
    std::cout << "   build_vpc       Creates a virtual point cloud" << std::endl;
    std::cout << "   clip            Outputs only points that are inside of the clipping polygons" << std::endl;
    std::cout << "   density         Exports a raster where each cell contains number of points" << std::endl;
    std::cout << "   info            Prints basic metadata from the point cloud file" << std::endl;
    std::cout << "   merge           Merges multiple point cloud files to a single one" << std::endl;
    std::cout << "   thin            Creates a thinned version of the point cloud (with fewer points)" << std::endl;
    std::cout << "   tile            Creates square tiles from input data" << std::endl;
    std::cout << "   to_raster       Exports point cloud data to a 2D raster grid" << std::endl;
    std::cout << "   to_raster_tin   Exports point cloud data to a 2D raster grid using triangulation" << std::endl;
    std::cout << "   to_vector       Exports point cloud data to a vector layer with 3D points" << std::endl;
    std::cout << "   translate       Converts to a different file format, reproject, and more" << std::endl;
}


#if defined(_WIN32) && defined(_MSC_VER)
int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
#else
int main(int argc, char* argv[])
#endif
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }
    std::string cmd = pdal::FileUtils::fromNative(argv[1]);

    std::vector<std::string> args;
    for ( int i = 2; i < argc; ++i )
        args.push_back(pdal::FileUtils::fromNative(argv[i]));

    if (cmd == "--help" || cmd == "help")
    {
        printUsage();
    }
    else if (cmd == "density")
    {
        Density density;
        runAlg(args, density);
    }
    else if (cmd == "boundary")
    {
        Boundary boundary;
        runAlg(args, boundary);
    }
    else if (cmd == "clip")
    {
        Clip clip;
        runAlg(args, clip);
    }
    else if (cmd == "build_vpc")
    {
        buildVpc(args);
    }
    else if (cmd == "merge")
    {
        Merge merge;
        runAlg(args, merge);
    }
    else if (cmd == "thin")
    {
        Thin thin;
        runAlg(args, thin);
    }
    else if (cmd == "to_raster")
    {
        ToRaster toRaster;
        runAlg(args, toRaster);
    }
    else if (cmd == "to_raster_tin")
    {
        ToRasterTin toRasterTin;
        runAlg(args, toRasterTin);
    }
    else if (cmd == "to_vector")
    {
        ToVector toVector;
        runAlg(args, toVector);
    }
    else if (cmd == "info")
    {
        Info info;
        runAlg(args, info);
    }
    else if (cmd == "translate")
    {
        Translate translate;
        runAlg(args, translate);
    }
    else if (cmd == "tile")
    {
      runTile(args);
    }
    else
    {
        std::cerr << "unknown command: " << cmd << std::endl;
        return 1;
    }

    return 0;
}
