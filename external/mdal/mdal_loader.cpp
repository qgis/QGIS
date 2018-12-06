/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_config.hpp"
#include "mdal_loader.hpp"
#include "frmts/mdal_2dm.hpp"
#include "frmts/mdal_ascii_dat.hpp"
#include "frmts/mdal_binary_dat.hpp"
#include "mdal_utils.hpp"

#ifdef HAVE_HDF5
#include "frmts/mdal_xmdf.hpp"
#endif

#ifdef HAVE_GDAL
#include "frmts/mdal_gdal_grib.hpp"
#endif

#ifdef HAVE_NETCDF
#include "frmts/mdal_3di.hpp"
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
#include "frmts/mdal_gdal_netcdf.hpp"
#endif

std::unique_ptr<MDAL::Mesh> MDAL::Loader::load( const std::string &meshFile, MDAL_Status *status )
{
  if ( !MDAL::fileExists( meshFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return nullptr;
  }

  MDAL::Loader2dm loader2dm( meshFile );
  std::unique_ptr<MDAL::Mesh> mesh = loader2dm.load( status );

#ifdef HAVE_NETCDF
  if ( !mesh && status && *status == MDAL_Status::Err_UnknownFormat )
  {
    MDAL::Loader3Di loader3di( meshFile );
    mesh = loader3di.load( status );
  }
#endif

#ifdef HAVE_GDAL
  if ( !mesh && status && *status == MDAL_Status::Err_UnknownFormat )
  {
#ifdef HAVE_NETCDF
    if ( MDAL::endsWith( meshFile, ".nc" ) )
    {
      MDAL::LoaderGdalNetCDF loaderNetCDF( meshFile );
      mesh = loaderNetCDF.load( status );
    }
    else
    {
#endif // HAVE_GDAL && HAVE_NETCDF
      MDAL::LoaderGdalGrib loaderGrib( meshFile );
      mesh = loaderGrib.load( status );
    }
#ifdef HAVE_NETCDF
  }
#endif // HAVE_GDAL && HAVE_NETCDF
#endif // HAVE_GDAL
  return mesh;
}

void MDAL::Loader::loadDatasets( Mesh *mesh, const std::string &datasetFile, MDAL_Status *status )
{
  MDAL::LoaderAsciiDat loaderAsciiDat( datasetFile );
  loaderAsciiDat.load( mesh, status );

  if ( status && *status == MDAL_Status::Err_UnknownFormat )
  {
    MDAL::LoaderBinaryDat loaderBinaryDat( datasetFile );
    loaderBinaryDat.load( mesh, status );
  }

#ifdef HAVE_HDF5
  if ( status && *status == MDAL_Status::Err_UnknownFormat )
  {
    MDAL::LoaderXmdf loaderXmdf( datasetFile );
    loaderXmdf.load( mesh, status );
  }
#endif
}
