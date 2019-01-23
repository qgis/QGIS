/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <vector>
#include <assert.h>
#include <netcdf.h>
#include "mdal_netcdf.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"

NetCDFFile::NetCDFFile(): mNcid( 0 ) {}

NetCDFFile::~NetCDFFile()
{
  if ( mNcid != 0 ) nc_close( mNcid );
}

int NetCDFFile::handle() const
{
  return mNcid;
}

void NetCDFFile::openFile( const std::string &fileName )
{
  int res = nc_open( fileName.c_str(), NC_NOWRITE, &mNcid );
  if ( res != NC_NOERR )
  {
    MDAL::debug( nc_strerror( res ) );
    throw MDAL_Status::Err_UnknownFormat;
  }
}

bool NetCDFFile::hasVariable( const std::string &name ) const
{
  assert( mNcid != 0 );
  int varid;
  return ( nc_inq_varid( mNcid, name.c_str(), &varid ) == NC_NOERR );
}

std::vector<int> NetCDFFile::readIntArr( const std::string &name, size_t dim ) const
{
  assert( mNcid != 0 );
  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  std::vector<int> arr_val( dim );
  if ( nc_get_var_int( mNcid, arr_id, arr_val.data() ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return arr_val;
}

std::vector<double> NetCDFFile::readDoubleArr( const std::string &name, size_t dim ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  std::vector<double> arr_val( dim );
  if ( nc_get_var_double( mNcid, arr_id, arr_val.data() ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return arr_val;
}

bool NetCDFFile::hasArr( const std::string &name ) const
{
  assert( mNcid != 0 );

  int arr_id;
  return nc_inq_varid( mNcid, name.c_str(), &arr_id ) == NC_NOERR;
}

std::vector<std::string> NetCDFFile::readArrNames() const
{
  assert( mNcid != 0 );

  std::vector<std::string> res;
  int nvars;
  if ( nc_inq_varids( mNcid, &nvars, nullptr ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }

  std::vector<int> varids( static_cast<size_t>( nvars ) );
  if ( nc_inq_varids( mNcid, &nvars, varids.data() ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }

  for ( size_t i = 0; i < static_cast<size_t>( nvars ); ++i )
  {
    std::vector<char> cname( NC_MAX_NAME + 1 );
    if ( nc_inq_varname( mNcid, varids[i], cname.data() ) != NC_NOERR )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
    res.push_back( cname.data() );
  }

  return res;
}

int NetCDFFile::getAttrInt( const std::string &name, const std::string &attr_name ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }

  int val;
  if ( nc_get_att_int( mNcid, arr_id, attr_name.c_str(), &val ) )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return val;
}

std::string NetCDFFile::getAttrStr( const std::string &name, const std::string &attr_name ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) ) throw MDAL_Status::Err_UnknownFormat;
  return getAttrStr( attr_name, arr_id );
}

std::string NetCDFFile::getAttrStr( const std::string &name, int varid ) const
{
  assert( mNcid != 0 );

  size_t attlen = 0;

  if ( nc_inq_attlen( mNcid, varid, name.c_str(), &attlen ) )
  {
    // attribute is missing
    return std::string();
  }

  char *string_attr;
  string_attr = ( char * ) malloc( attlen + 1 );

  if ( nc_get_att_text( mNcid, varid, name.c_str(), string_attr ) ) throw MDAL_Status::Err_UnknownFormat;
  string_attr[attlen] = '\0';

  std::string res( string_attr );
  free( string_attr );

  return res;
}

double NetCDFFile::getFillValue( int varid ) const
{
  return getAttrDouble( varid, "_FillValue" );
}

double NetCDFFile::getAttrDouble( int varid, const std::string &attr_name ) const
{
  double res;
  if ( nc_get_att_double( mNcid, varid, attr_name.c_str(), &res ) )
    res = std::numeric_limits<double>::quiet_NaN(); // not present/set
  return res;
}

int NetCDFFile::getVarId( const std::string &name )
{
  int ncid_val;
  if ( nc_inq_varid( mNcid, name.c_str(), &ncid_val ) != NC_NOERR ) throw MDAL_Status::Err_UnknownFormat;
  return ncid_val;
}

void NetCDFFile::getDimension( const std::string &name, size_t *val, int *ncid_val ) const
{
  assert( mNcid != 0 );

  if ( nc_inq_dimid( mNcid, name.c_str(), ncid_val ) != NC_NOERR ) throw MDAL_Status::Err_UnknownFormat;
  if ( nc_inq_dimlen( mNcid, *ncid_val, val ) != NC_NOERR ) throw MDAL_Status::Err_UnknownFormat;
}

void NetCDFFile::getDimensionOptional( const std::string &name, size_t *val, int *ncid_val ) const
{
  assert( mNcid != 0 );

  try
  {
    getDimension( name, val, ncid_val );
  }
  catch ( MDAL_Status )
  {
    *ncid_val = -1;
    *val = 0;
  }
}
