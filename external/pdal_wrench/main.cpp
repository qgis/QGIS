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

#include "alg.hpp"
#include "vpc.hpp"

extern int runTile(std::vector<std::string> arglist);  // tile/tile.cpp


// TODO: make it windows/unicode friendly
//#ifdef _WIN32
//int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
//#else
int main(int argc, char* argv[])
//#endif
{
    if (argc < 2)
    {
        std::cerr << "need to specify command:" << std::endl;
        std::cerr << " - boundary" << std::endl;
        std::cerr << " - clip" << std::endl;
        std::cerr << " - density" << std::endl;
        std::cerr << " - build_vpc" << std::endl;
        std::cerr << " - info" << std::endl;
        std::cerr << " - merge" << std::endl;
        std::cerr << " - thin" << std::endl;
        std::cerr << " - tile" << std::endl;
        std::cerr << " - to_raster" << std::endl;
        std::cerr << " - to_raster_tin" << std::endl;
        std::cerr << " - to_vector" << std::endl;
        std::cerr << " - translate" << std::endl;
        return 1;
    }
    std::string cmd = argv[1];

    // TODO: use untwine::fromNative(argv[i])
    std::vector<std::string> args;
    for ( int i = 2; i < argc; ++i )
        args.push_back(argv[i]);

    if (cmd == "density")
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
