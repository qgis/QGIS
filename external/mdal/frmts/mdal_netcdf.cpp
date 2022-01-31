/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <vector>
#include <assert.h>
#include <netcdf.h>
#include <cmath>

#include "mdal_netcdf.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

NetCDFFile::NetCDFFile(): mNcid( 0 ) {}

NetCDFFile::~NetCDFFile()
{
  if ( mNcid != 0 )
  {
    nc_close( mNcid );
    mNcid = 0;
  }
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
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + fileName );
  }
  mFileName = fileName;
}

std::vector<int> NetCDFFile::readIntArr( const std::string &name, size_t dim ) const
{
  assert( mNcid != 0 );
  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Internal error in Netcfd - unknown format" );
  std::vector<int> arr_val( dim );
  if ( nc_get_var_int( mNcid, arr_id, arr_val.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Internal error in Netcfd - unknown format" );
  return arr_val;
}

std::vector<int> NetCDFFile::readIntArr( int arr_id, size_t start_dim1, size_t start_dim2, size_t count_dim1, size_t count_dim2 ) const
{
  assert( mNcid != 0 );

  const std::vector<size_t> startp = {start_dim1, start_dim2};
  const std::vector<size_t> countp = {count_dim1, count_dim2};
  const std::vector<ptrdiff_t> stridep = {1, 1};

  std::vector<int> arr_val( count_dim1 * count_dim2 );
  int res = nc_get_vars_int( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val.data() );
  if ( res != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read numeric array" );
  return arr_val;
}

std::vector<int> NetCDFFile::readIntArr( int arr_id, size_t start_dim, size_t count_dim ) const
{
  assert( mNcid != 0 );

  const std::vector<size_t> startp = {start_dim};
  const std::vector<size_t> countp = {count_dim};
  const std::vector<ptrdiff_t> stridep = {1};

  std::vector<int> arr_val( count_dim );
  int res = nc_get_vars_int( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val.data() );
  if ( res != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read numeric array" );
  return arr_val;
}

std::vector<double> NetCDFFile::readDoubleArr( const std::string &name, size_t dim ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  std::vector<double> arr_val( dim );

  nc_type typep;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  if ( nc_inq_vartype( mNcid, arr_id, &typep ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );

  if ( typep == NC_FLOAT )
  {
    std::vector<float> arr_val_f( dim );
    if ( nc_get_var_float( mNcid, arr_id, arr_val_f.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
    for ( size_t i = 0; i < dim; ++i )
    {
      const float val = arr_val_f[i];
      if ( std::isnan( val ) )
        arr_val[i] = std::numeric_limits<double>::quiet_NaN();
      else
        arr_val[i] = static_cast<double>( val );
    }
  }
  else if ( typep == NC_DOUBLE )
  {
    if ( nc_get_var_double( mNcid, arr_id, arr_val.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  }
  else
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  }
  return arr_val;
}

std::vector<double> NetCDFFile::readDoubleArr( int arr_id,
    size_t start_dim1, size_t start_dim2,
    size_t count_dim1, size_t count_dim2 ) const
{
  assert( mNcid != 0 );

  const std::vector<size_t> startp = {start_dim1, start_dim2};
  const std::vector<size_t> countp = {count_dim1, count_dim2};
  const std::vector<ptrdiff_t> stridep = {1, 1};

  std::vector<double> arr_val( count_dim1 * count_dim2 );

  nc_type typep;
  if ( nc_inq_vartype( mNcid, arr_id, &typep ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );

  if ( typep == NC_FLOAT )
  {
    std::vector<float> arr_val_f( count_dim1 * count_dim2 );
    if ( nc_get_vars_float( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val_f.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
    for ( size_t i = 0; i < count_dim1 * count_dim2; ++i )
    {
      const float val = arr_val_f[i];
      if ( std::isnan( val ) )
        arr_val[i] = std::numeric_limits<double>::quiet_NaN();
      else
        arr_val[i] = static_cast<double>( val );
    }
  }
  else if ( typep == NC_BYTE )
  {
    std::vector<unsigned char> arr_val_b( count_dim1 * count_dim2 );
    if ( nc_get_vars_uchar( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val_b.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
    for ( size_t i = 0; i < count_dim1 * count_dim2; ++i )
    {
      const unsigned char val = arr_val_b[i];
      if ( val == 129 )
        arr_val[i] = std::numeric_limits<double>::quiet_NaN();
      else
        arr_val[i] = double( int( val ) );
    }
  }
  else if ( typep == NC_DOUBLE )
  {
    if ( nc_get_vars_double( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  }
  else
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  }
  return arr_val;
}

std::vector<double> NetCDFFile::readDoubleArr( int arr_id,
    size_t start_dim,
    size_t count_dim
                                             ) const
{
  assert( mNcid != 0 );

  const std::vector<size_t> startp = {start_dim};
  const std::vector<size_t> countp = {count_dim};
  const std::vector<ptrdiff_t> stridep = {1, 1};

  std::vector<double> arr_val( count_dim );

  nc_type typep;
  if ( nc_inq_vartype( mNcid, arr_id, &typep ) != NC_NOERR )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );

  if ( typep == NC_FLOAT )
  {
    std::vector<float> arr_val_f( count_dim );
    if ( nc_get_vars_float( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val_f.data() ) != NC_NOERR )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
    for ( size_t i = 0; i < count_dim; ++i )
    {
      const float val = arr_val_f[i];
      if ( std::isnan( val ) )
        arr_val[i] = std::numeric_limits<double>::quiet_NaN();
      else
        arr_val[i] = static_cast<double>( val );
    }
  }
  else if ( typep == NC_INT )
  {
    std::vector<int> arr_val_int( count_dim );
    if ( nc_get_vars_int( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val_int.data() ) != NC_NOERR )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
    for ( size_t i = 0; i < count_dim; ++i )
    {
      arr_val[i] = static_cast<double>( arr_val_int[i] );
    }
  }
  else if ( typep == NC_DOUBLE )
  {
    if ( nc_get_vars_double( mNcid, arr_id, startp.data(), countp.data(), stridep.data(), arr_val.data() ) != NC_NOERR )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  }
  else
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read double array" );
  }
  return arr_val;
}

bool NetCDFFile::hasArr( const std::string &name ) const
{
  assert( mNcid != 0 );
  int arr_id;
  return nc_inq_varid( mNcid, name.c_str(), &arr_id ) == NC_NOERR;
}

int NetCDFFile::arrId( const std::string &name ) const
{
  int arr_id = -1;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR )
  {
    arr_id = -1;
  }
  return arr_id;
}

std::vector<std::string> NetCDFFile::readArrNames() const
{
  assert( mNcid != 0 );

  std::vector<std::string> res;
  int nvars;
  if ( nc_inq_varids( mNcid, &nvars, nullptr ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read variable names" );

  std::vector<int> varids( static_cast<size_t>( nvars ) );
  if ( nc_inq_varids( mNcid, &nvars, varids.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read variable names" );

  for ( size_t i = 0; i < static_cast<size_t>( nvars ); ++i )
  {
    std::vector<char> cname( NC_MAX_NAME + 1 );
    if ( nc_inq_varname( mNcid, varids[i], cname.data() ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not read variable names" );
    res.push_back( cname.data() );
  }

  return res;
}

bool NetCDFFile::hasAttrInt( const std::string &name, const std::string &attr_name ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR ) return false;

  int val;
  if ( nc_get_att_int( mNcid, arr_id, attr_name.c_str(), &val ) ) return false;

  return true;
}

int NetCDFFile::getAttrInt( const std::string &name, const std::string &attr_name ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get numeric attribute" );

  int val;
  if ( nc_get_att_int( mNcid, arr_id, attr_name.c_str(), &val ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get numeric attribute" );
  return val;
}

std::string NetCDFFile::getAttrStr( const std::string &name, const std::string &attr_name ) const
{
  assert( mNcid != 0 );

  int arr_id;
  if ( nc_inq_varid( mNcid, name.c_str(), &arr_id ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get string attribute" );
  return getAttrStr( attr_name, arr_id );
}

std::string NetCDFFile::getAttrStr( const std::string &attr_name, int varid ) const
{
  assert( mNcid != 0 );

  size_t attlen = 0;

  if ( nc_inq_attlen( mNcid, varid, attr_name.c_str(), &attlen ) )
  {
    // attribute is missing
    return std::string();
  }

  char *string_attr = static_cast<char *>( malloc( attlen + 1 ) );

  if ( nc_get_att_text( mNcid, varid, attr_name.c_str(), string_attr ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get string attribute" );
  string_attr[attlen] = '\0';

  std::string res( string_attr );
  free( string_attr );

  return res;
}

double NetCDFFile::getFillValue( int varid ) const
{
  return getAttrDouble( varid, "_FillValue" );
}

bool NetCDFFile::hasAttrDouble( int varid, const std::string &attr_name ) const
{
  double res;
  if ( nc_get_att_double( mNcid, varid, attr_name.c_str(), &res ) )
    return false;
  return true;
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
  if ( nc_inq_varid( mNcid, name.c_str(), &ncid_val ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get variable id" );
  return ncid_val;
}

void NetCDFFile::getDimension( const std::string &name, size_t *val, int *ncid_val ) const
{
  assert( mNcid != 0 );

  if ( nc_inq_dimid( mNcid, name.c_str(), ncid_val ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get dimension, invalid dimension ID or name" );
  if ( nc_inq_dimlen( mNcid, *ncid_val, val ) != NC_NOERR ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get dimension, invalid dimension ID or name" );
}

void NetCDFFile::getDimensions( const std::string &variableName, std::vector<size_t> &dimensions, std::vector<int> &dimensionIds )
{
  assert( mNcid != 0 );

  int n;
  int varId;
  if ( nc_inq_varid( mNcid, variableName.c_str(), &varId ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get dimensions" );
  if ( nc_inq_varndims( mNcid, varId, &n ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get dimensions" );

  dimensionIds.resize( size_t( n ) );
  dimensions.resize( size_t( n ) );

  if ( nc_inq_vardimid( mNcid, varId, dimensionIds.data() ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not get dimensions" );

  for ( int i = 0; i < n; ++i )
  {
    nc_inq_dimlen( mNcid, dimensionIds[size_t( i )], &dimensions[size_t( i )] );
  }
}

bool NetCDFFile::hasDimension( const std::string &name ) const
{
  int ncid_val;
  return nc_inq_dimid( mNcid, name.c_str(), &ncid_val ) == NC_NOERR;
}

void NetCDFFile::createFile( const std::string &fileName )
{
  int res = nc_create( fileName.c_str(), NC_CLOBBER, &mNcid );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
}

int NetCDFFile::defineDimension( const std::string &name, size_t size )
{
  int dimId = 0;
  int res = nc_def_dim( mNcid, name.c_str(), size, &dimId );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
  return dimId;
}

int NetCDFFile::defineVar( const std::string &varName,
                           int ncType, int dimensionCount, const int *dimensions )
{
  int varIdp;

  int res = nc_def_var( mNcid, varName.c_str(), ncType, dimensionCount, dimensions, &varIdp );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }

  return varIdp;
}

void NetCDFFile::putAttrStr( int varId, const std::string &attrName, const std::string &value )
{
  int res = nc_put_att_text( mNcid, varId, attrName.c_str(), value.size(), value.c_str() );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
}

void NetCDFFile::putAttrInt( int varId, const std::string &attrName, int value )
{
  int res = nc_put_att_int( mNcid, varId, attrName.c_str(), NC_INT, 1, &value );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
}

void NetCDFFile::putAttrDouble( int varId, const std::string &attrName, double value )
{
  int res = nc_put_att_double( mNcid, varId, attrName.c_str(), NC_DOUBLE, 1, &value );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
}

void NetCDFFile::putDataDouble( int varId, const size_t index, const double value )
{
  int res = nc_put_var1_double( mNcid, varId, &index, &value );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
}

void NetCDFFile::putDataArrayInt( int varId, size_t line, size_t faceVerticesMax, int *values )
{
  // Configuration of these two vectors determines how is value array read and stored in the file
  // https://www.unidata.ucar.edu/software/netcdf/docs/programming_notes.html#specify_hyperslabfileNameToSave
  const size_t start[] = { line, 0 };
  const size_t count[] = { 1, faceVerticesMax };

  int res = nc_put_vara_int( mNcid, varId, start, count, values );
  if ( res != NC_NOERR )
  {
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, nc_strerror( res ) );
  }
}

std::string NetCDFFile::getFileName() const
{
  return mFileName;
}
