/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_gdal_netcdf.hpp"
#include "mdal_utils.hpp"

MDAL::DriverGdalNetCDF::DriverGdalNetCDF()
  : MDAL::DriverGdal(
      "NETCDF",
      "GDAL NetCDF",
      "*.nc"
      , "GRIB" )
  , mTimeDiv( 1.0 )
{
}

MDAL::DriverGdalNetCDF *MDAL::DriverGdalNetCDF::create()
{
  return new DriverGdalNetCDF();
}

std::string MDAL::DriverGdalNetCDF::GDALFileName( const std::string &fileName )
{
#ifdef WIN32
  // Force usage of the predefined GDAL driver
  // http://gis.stackexchange.com/a/179167
  // on Windows, HDF5 driver is checked before NETCDF driver in GDAL
  return  "NETCDF:\"" + fileName + "\"";
#else
  return fileName;
#endif
}

bool MDAL::DriverGdalNetCDF::parseBandInfo( const MDAL::GdalDataset *cfGDALDataset, const MDAL::DriverGdal::metadata_hash &metadata, std::string &band_name, double *time, bool *is_vector, bool *is_x )
{
  MDAL_UNUSED( cfGDALDataset );

  metadata_hash::const_iterator iter;

  iter = metadata.find( "netcdf_dim_time" );
  if ( iter == metadata.end() ) return true; //FAILURE, skip no-time bands
  *time = parseMetadataTime( iter->second ) / mTimeDiv;

  // NAME
  iter = metadata.find( "long_name" );
  if ( iter == metadata.end() )
  {
    iter = metadata.find( "netcdf_varname" );
    if ( iter == metadata.end() ) return true; //FAILURE, should be always present
    band_name = iter->second;
  }
  else
  {
    band_name = iter->second;
  }

  // Loop throught all additional dimensions but time
  for ( iter = metadata.begin(); iter != metadata.end(); ++iter )
  {
    std::string key = iter->first;
    if ( MDAL::contains( key, "netcdf_dim_" ) )
    {
      key = MDAL::replace( key, "netcdf_dim_", "" );
      if ( key != "time" )
      {
        band_name += "_" + key + ":" + iter->second;
      }
    }
  }

  // Parse X, Y components if present
  parseBandIsVector( band_name, is_vector, is_x );

  return false; // SUCCESS
}

void MDAL::DriverGdalNetCDF::parseGlobals( const MDAL::DriverGdal::metadata_hash &metadata )
{
  metadata_hash::const_iterator iter = metadata.find( "time#units" );
  if ( iter != metadata.end() )
  {
    std::string units = iter->second;
    mTimeDiv = MDAL::parseTimeUnits( units );
    // TODO store reference time from iter->second too, see crayfish_netcdf.cpp
  }
}
