/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/


#include "mdal_gdal_grib.hpp"
#include "mdal_utils.hpp"
#include <string>
#include <limits>

MDAL::DriverGdalGrib::DriverGdalGrib( )
  : MDAL::DriverGdal(
      "GRIB",
      "GDAL Grib",
      "*.grb;;*.grb2;;*.bin;;*.grib;;*.grib1;;*.grib2"
  , "GRIB" ),
    mRefTime( DateTime() )
{}

MDAL::DriverGdalGrib *MDAL::DriverGdalGrib::create()
{
  return new DriverGdalGrib();
}

MDAL::DriverGdalGrib::~DriverGdalGrib() = default;

bool MDAL::DriverGdalGrib::parseBandInfo( const MDAL::GdalDataset *cfGDALDataset,
    const metadata_hash &metadata, std::string &band_name,
    MDAL::RelativeTimestamp *time, bool *is_vector, bool *is_x
                                        )
{
  MDAL_UNUSED( cfGDALDataset )

  metadata_hash::const_iterator iter;

  // NAME
  iter = metadata.find( "grib_comment" );
  if ( iter == metadata.end() ) return true; //FAILURE
  band_name = iter->second;

  if ( !mRefTime.isValid() )
  {
    iter = metadata.find( "grib_ref_time" );
    if ( iter == metadata.end() ) return true; //FAILURE
    mRefTime = DateTime( parseMetadataTime( iter->second ), DateTime::Unix );
  }

  // TIME
  iter = metadata.find( "grib_valid_time" );
  if ( iter == metadata.end() ) return true; //FAILURE
  DateTime valid_time = DateTime( parseMetadataTime( iter->second ), DateTime::Unix );
  *time = ( valid_time - mRefTime );

  // Parse X, Y components if present
  parseBandIsVector( band_name, is_vector, is_x );

  return false; // success
}

MDAL::DateTime MDAL::DriverGdalGrib::referenceTime() const
{
  return mRefTime;
}
