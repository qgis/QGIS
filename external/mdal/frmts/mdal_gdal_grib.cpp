/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/


#include "mdal_gdal_grib.hpp"
#include "mdal_utils.hpp"
#include <string>
#include <limits>

MDAL::LoaderGdalGrib::LoaderGdalGrib( const std::string &gribFile )
  : MDAL::LoaderGdal( gribFile, "GRIB" ),
    mRefTime( std::numeric_limits<double>::min() )
{}

bool MDAL::LoaderGdalGrib::parseBandInfo( const MDAL::GdalDataset *cfGDALDataset,
    const metadata_hash &metadata, std::string &band_name,
    double *time, bool *is_vector, bool *is_x
                                        )
{
  MDAL_UNUSED( cfGDALDataset );

  metadata_hash::const_iterator iter;

  // NAME
  iter = metadata.find( "grib_comment" );
  if ( iter == metadata.end() ) return true; //FAILURE
  band_name = iter->second;

  if ( MDAL::equals( mRefTime, std::numeric_limits<double>::min() ) )
  {
    iter = metadata.find( "grib_ref_time" );
    if ( iter == metadata.end() ) return true; //FAILURE
    mRefTime = parseMetadataTime( iter->second );
  }

  // TIME
  iter = metadata.find( "grib_valid_time" );
  if ( iter == metadata.end() ) return true; //FAILURE
  double valid_time = parseMetadataTime( iter->second );
  *time = ( valid_time - mRefTime ) / 3600.0; // input times are always in seconds UTC, we need them back in hours

  // Parse X, Y components if present
  parseBandIsVector( band_name, is_vector, is_x );

  return false; // success
}
