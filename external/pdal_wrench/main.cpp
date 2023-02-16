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
#if 1
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
#elif 0
    std::string cmd = "density";
    std::vector<std::string> args;
    // args.push_back("-i");
    // args.push_back("/home/martin/qgis/point-cloud-sandbox/data/24-fixed.las");
    // args.push_back("-o");
    // args.push_back("/tmp/dens.tif");
    // args.push_back("-r");
    // args.push_back("1");
    //args.push_back("xxx");

    args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/merged.copc.laz");
    args.push_back("--output=/tmp/densX.tif");
    args.push_back("--resolution=1");
    args.push_back("--tile-size=250");
    args.push_back("--tile-origin-x=0");
    args.push_back("--tile-origin-y=0");
    args.push_back("--threads=4");
#elif 0
    std::string cmd = "boundary";
    std::vector<std::string> args;

    args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/24-fixed.las");
    //args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/merged.copc.laz"); // TODO: empty boundary???
    args.push_back("--output=/tmp/boundary.gpkg");
    //args.push_back("--resolution=1");
    //args.push_back("--tile-size=250");
    //args.push_back("--threads=4");
    //args.push_back("--filter=Classification==5");
#elif 0
    std::string cmd = "clip";
    std::vector<std::string> args;

    args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/24-fixed.las");
    args.push_back("--polygon=/home/martin/qgis/point-cloud-sandbox/data/24-polygon.gpkg");
    args.push_back("--output=/tmp/clipped.las");
    //args.push_back("--filter=Classification==5");
#elif 0
    std::string cmd = "build_vpc";
    std::vector<std::string> args;
    args.push_back("--output=/tmp/tatry-9.vpc");
    args.push_back("/home/martin/tatry-tiles/tatry_0_1.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_0_2.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_0_3.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_1_1.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_1_2.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_1_3.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_2_1.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_2_2.laz");
    args.push_back("/home/martin/tatry-tiles/tatry_2_3.laz");
#elif 0
    std::string cmd = "clip";
    std::vector<std::string> args;

    args.push_back("--input=/tmp/tatry-9.vpc");
    args.push_back("--polygon=/home/martin/qgis/point-cloud-sandbox/data/tatry.gpkg");
    args.push_back("--output=/tmp/tatry-clipped.vpc");
    //args.push_back("--output-format=laz");

#elif 0
    std::string cmd = "density";
    std::vector<std::string> args;
    args.push_back("--input=/tmp/first.vpc");
    args.push_back("--output=/tmp/first.tif");
    args.push_back("--resolution=1");
    args.push_back("--threads=4");
    //args.push_back("--filter=Classification==2");
    // for good alignment of input and output
    args.push_back("--tile-origin-x=377250");
    args.push_back("--tile-origin-y=5441420");

#elif 0
    std::string cmd = "boundary";
    std::vector<std::string> args;
    args.push_back("--input=/tmp/tatry-9.vpc");
    args.push_back("--output=/tmp/tatry-9-boundary.gpkg");
#elif 0
    std::string cmd = "merge";
    std::vector<std::string> args;
    args.push_back("--output=/tmp/merged.las");
    args.push_back("/home/martin/qgis/point-cloud-sandbox/data/trencin-2-ground.laz");
    args.push_back("/home/martin/qgis/point-cloud-sandbox/data/trencin-6-buildings.laz");
#elif 0
    std::string cmd = "to_raster_tin";
    std::vector<std::string> args;
    args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/trencin-2-ground.laz");
    args.push_back("--output=/tmp/raster_tin.tif");
    args.push_back("--resolution=1");
#elif 0
    std::string cmd = "to_raster";
    std::vector<std::string> args;

    args.push_back("--input=/tmp/first.vpc");
    args.push_back("--output=/tmp/first-dem.tif");
    args.push_back("--resolution=1");
    args.push_back("--filter=Classification==2");
#elif 0
    std::string cmd = "to_raster_tin";
    std::vector<std::string> args;

    // args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/merged.copc.laz");
    // args.push_back("--output=/tmp/merged-tin.tif");
    // args.push_back("--resolution=1");
    // args.push_back("--tile-size=250");
    // args.push_back("--tile-origin-x=0");
    // args.push_back("--tile-origin-y=0");
    // args.push_back("--threads=1");

    args.push_back("--input=/tmp/first.vpc");
    args.push_back("--output=/tmp/first-tin.tif");
    args.push_back("--resolution=1");
    args.push_back("--filter=Classification==2");
    args.push_back("--threads=1");
    //args.push_back("--tile-size=500");
    // for good alignment of input and output
    //args.push_back("--tile-origin-x=377250");
    //args.push_back("--tile-origin-y=5441420");
#elif 0
    std::string cmd = "thin";
    std::vector<std::string> args;

    args.push_back("--input=/tmp/first.vpc");
//    args.push_back("--mode=every-nth");
//    args.push_back("--step-every-nth=20");
//    args.push_back("--output=/tmp/tatry-thinned.vpc");

    args.push_back("--mode=sample");
    args.push_back("--step-sample=1");
    args.push_back("--output=/tmp/tatry-thinned-sample.vpc");
#elif 0
    std::string cmd = "to_vector";
    std::vector<std::string> args;

    args.push_back("--input=/tmp/first.vpc");
    //args.push_back("--step=20");
    args.push_back("--output=/tmp/first.gpkg");
#elif 0
    std::string cmd = "info";
    std::vector<std::string> args;
    args.push_back("--input=/tmp/clipped.las");
#elif 0
    std::string cmd = "info";
    std::vector<std::string> args;
    args.push_back("--input=/tmp/tatry-9.vpc");
#elif 0
    std::string cmd = "translate";
    std::vector<std::string> args;

    args.push_back("--input=/home/martin/qgis/point-cloud-sandbox/data/trencin.laz");
    args.push_back("--assign-crs=EPSG:5514");
    args.push_back("--output=/tmp/trencin-fixed-crs.las");
#elif 0
    std::string cmd = "translate";
    std::vector<std::string> args;

    args.push_back("--input=/tmp/trencin-fixed-crs.las");
    args.push_back("--transform-crs=EPSG:3857");
    args.push_back("--filter=Classification==2");
    args.push_back("--output=/tmp/trencin-3857.las");
#elif 0
  std::string cmd = "tile";
  std::vector<std::string> args;
  //args.push_back("--threads=1");
  args.push_back("--length=200");
  args.push_back("--output=/tmp/tatry-3-tiled.vpc");
  args.push_back("/home/martin/tatry-tiles/tatry_0_1.laz");
  //args.push_back("/tmp/first.vpc");
#elif 0
  std::string cmd = "build_vpc";
  std::vector<std::string> args;
  args.push_back("--output=/tmp/nrcan-test.vpc");
  args.push_back("https://download-telecharger.services.geo.ca/pub/elevation/pointclouds_nuagespoints/NRCAN/Hamilton_Niagara_2021_2/ON_Niagara_20210525_NAD83CSRS_UTM17N_1km_E567_N4805_CLASS.copc.laz");
  args.push_back("https://download-telecharger.services.geo.ca/pub/elevation/pointclouds_nuagespoints/NRCAN/Hamilton_Niagara_2021_2/ON_Niagara_20210525_NAD83CSRS_UTM17N_1km_E567_N4806_CLASS.copc.laz");
  args.push_back("https://download-telecharger.services.geo.ca/pub/elevation/pointclouds_nuagespoints/NRCAN/Hamilton_Niagara_2021_2/ON_Niagara_20210525_NAD83CSRS_UTM17N_1km_E567_N4807_CLASS.copc.laz");
#else
  std::string cmd = "info";
  std::vector<std::string> args;
  // compound
  //args.push_back("--input=/home/martin/tmp/las/nrcan/ON_LAKE_NIPISSING_20190619_NAD83CSRS_UTMZ17_1KM_E5430_N51090_CLASS.laz");
  //args.push_back("--input=/home/martin/tmp/las/autzen/autzen-classified.laz");  // using "US survey foot" as unit
  //args.push_back("--input=/home/martin/tmp/las/bergen/data/32-1-468-145-45.laz");
  //args.push_back("--input=/home/martin/tmp/las/estonia-nir/467494_2021_tava.laz");
  // mixed lat/lon and meters
  //args.push_back("--input=/home/martin/tmp/las/noaa/20140208_LA_37_20164901.laz");
  // vertical only specifies it is in meters
  //args.push_back("--input=/home/martin/tmp/las/fi/L4142G1_7.laz");
  //args.push_back("--input=/home/martin/tmp/las/lene/ground.laz");
  //args.push_back("--input=https://s3.amazonaws.com/hobu-lidar/montreal-2015.copc.laz");
  // missing vertical
  //args.push_back("--input=/home/martin/tmp/las/ba-hrad/03_Bratislava_18_214150_5339178_a_c_jtsk03_bpv_clip.las");
  //args.push_back("--input=/home/martin/tmp/las/ign/PTS_LAMB93_IGN69_0965_6570.copc.laz");
  //args.push_back("--input=/home/martin/tmp/las/Données_MNT_lidar/subset_samp.laz");
  //args.push_back("--input=/home/martin/tmp/las/solothurn/2607000_1228000.laz");
  // no crs
  //args.push_back("--input=/home/martin/tmp/las/fi-helsinki/rgb_673496c.laz");
  //args.push_back("--input=/home/martin/tmp/las/sdfe/1km_6210_527.laz");


  //args.push_back("--input=https://s3-us-west-2.amazonaws.com/usgs-lidar-public/USGS_LPC_MD_VA_Sandy_NCR_2014_LAS_2015/ept.json");
#endif

    std::cout << "command: " << cmd << std::endl;

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
