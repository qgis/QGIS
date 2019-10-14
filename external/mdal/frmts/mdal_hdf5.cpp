/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include "mdal_hdf5.hpp"
#include <cstring>


HdfFile::HdfFile( const std::string &path )
  : d( std::make_shared< Handle >( H5Fopen( path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT ) ) )
{
}

bool HdfFile::isValid() const { return d->id >= 0; }

hid_t HdfFile::id() const { return d->id; }

HdfGroup::HdfGroup( hid_t file, const std::string &path )
  : d( std::make_shared< Handle >( H5Gopen( file, path.c_str() ) ) )
{}

bool HdfGroup::isValid() const { return d->id >= 0; }

hid_t HdfGroup::id() const { return d->id; }

hid_t HdfGroup::file_id() const { return H5Iget_file_id( d->id ); }

std::string HdfGroup::name() const
{
  char name[HDF_MAX_NAME];
  H5Iget_name( d->id, name, HDF_MAX_NAME );
  return std::string( name );
}

std::vector<std::string> HdfGroup::groups() const { return objects( H5G_GROUP ); }

std::vector<std::string> HdfGroup::datasets() const { return objects( H5G_DATASET ); }

std::vector<std::string> HdfGroup::objects() const { return objects( H5G_UNKNOWN ); }

std::string HdfGroup::childPath( const std::string &childName ) const { return name() + "/" + childName; }

std::vector<std::string> HdfGroup::objects( H5G_obj_t type ) const
{
  std::vector<std::string> lst;

  hsize_t nobj;
  H5Gget_num_objs( d->id, &nobj );
  for ( hsize_t i = 0; i < nobj; ++i )
  {
    if ( type == H5G_UNKNOWN || H5Gget_objtype_by_idx( d->id, i ) == type )
    {
      char name[HDF_MAX_NAME];
      H5Gget_objname_by_idx( d->id, i, name, ( size_t )HDF_MAX_NAME );
      lst.push_back( std::string( name ) );
    }
  }
  return lst;
}

HdfAttribute::HdfAttribute( hid_t obj_id, const std::string &attr_name )
  : d( std::make_shared< Handle >( H5Aopen( obj_id, attr_name.c_str(), H5P_DEFAULT ) ) )
{}

bool HdfAttribute::isValid() const { return d->id >= 0; }

hid_t HdfAttribute::id() const { return d->id; }

std::string HdfAttribute::readString() const
{
  hid_t datatype = H5Aget_type( id() );
  char name[HDF_MAX_NAME + 1];
  std::memset( name, '\0', HDF_MAX_NAME + 1 );
  herr_t status = H5Aread( d->id, datatype, name );
  if ( status < 0 )
  {
    //MDAL::debug("Failed to read data!");
    H5Tclose( datatype );
    return std::string();
  }

  H5Tclose( datatype );
  std::string res( name );
  res = MDAL::trim( res );
  return res;
}

HdfDataset::HdfDataset( hid_t file, const std::string &path )
  : d( std::make_shared< Handle >( H5Dopen2( file, path.c_str(), H5P_DEFAULT ) ) )
{}

bool HdfDataset::isValid() const { return d->id >= 0; }

hid_t HdfDataset::id() const { return d->id; }

std::vector<hsize_t> HdfDataset::dims() const
{
  hid_t sid = H5Dget_space( d->id );
  std::vector<hsize_t> ret( static_cast<size_t>( H5Sget_simple_extent_ndims( sid ) ) );
  H5Sget_simple_extent_dims( sid, ret.data(), nullptr );
  H5Sclose( sid );
  return ret;
}

hsize_t HdfDataset::elementCount() const
{
  hsize_t count = 1;
  for ( hsize_t dsize : dims() )
    count *= dsize;
  return count;
}

H5T_class_t HdfDataset::type() const
{
  hid_t tid = H5Dget_type( d->id );
  H5T_class_t t_class = H5Tget_class( tid );
  H5Tclose( tid );
  return t_class;
}

std::vector<uchar> HdfDataset::readArrayUint8( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const { return readArray<uchar>( H5T_NATIVE_UINT8, offsets, counts ); }

std::vector<float> HdfDataset::readArray( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const { return readArray<float>( H5T_NATIVE_FLOAT, offsets, counts ); }

std::vector<double> HdfDataset::readArrayDouble( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const { return readArray<double>( H5T_NATIVE_DOUBLE, offsets, counts ); }

std::vector<int> HdfDataset::readArrayInt( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const { return readArray<int>( H5T_NATIVE_INT, offsets, counts ); }

std::vector<uchar> HdfDataset::readArrayUint8() const { return readArray<uchar>( H5T_NATIVE_UINT8 ); }

std::vector<float> HdfDataset::readArray() const { return readArray<float>( H5T_NATIVE_FLOAT ); }

std::vector<double> HdfDataset::readArrayDouble() const { return readArray<double>( H5T_NATIVE_DOUBLE ); }

std::vector<int> HdfDataset::readArrayInt() const { return readArray<int>( H5T_NATIVE_INT ); }

std::vector<std::string> HdfDataset::readArrayString() const
{
  std::vector<std::string> ret;

  hid_t datatype = H5Tcopy( H5T_C_S1 );
  H5Tset_size( datatype, HDF_MAX_NAME );

  std::vector<HdfString> arr = readArray<HdfString>( datatype );

  H5Tclose( datatype );

  for ( const HdfString &str : arr )
  {
    std::string dat = std::string( str.data );
    ret.push_back( MDAL::trim( dat ) );
  }

  return ret;
}

float HdfDataset::readFloat() const
{
  if ( elementCount() != 1 )
  {
    MDAL::debug( "Not scalar!" );
    return 0;
  }

  float value;
  herr_t status = H5Dread( d->id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value );
  if ( status < 0 )
  {
    MDAL::debug( "Failed to read data!" );
    return 0;
  }
  return value;
}

std::string HdfDataset::readString() const
{
  if ( elementCount() != 1 )
  {
    MDAL::debug( "Not scalar!" );
    return std::string();
  }

  char name[HDF_MAX_NAME];
  hid_t datatype = H5Tcopy( H5T_C_S1 );
  H5Tset_size( datatype, HDF_MAX_NAME );
  herr_t status = H5Dread( d->id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, name );
  if ( status < 0 )
  {
    MDAL::debug( "Failed to read data!" );
    return std::string();
  }
  H5Tclose( datatype );
  return std::string( name );
}

HdfDataspace::HdfDataspace( const std::vector<hsize_t> &dims )
  : d( std::make_shared< Handle >( H5Screate_simple(
                                     static_cast<int>( dims.size() ),
                                     dims.data(),
                                     dims.data()
                                   )
                                 )
     )
{
}

HdfDataspace::HdfDataspace( hid_t dataset )
  : d( std::make_shared< Handle >( H5Dget_space( dataset ) ) )
{
}

void HdfDataspace::selectHyperslab( hsize_t start, hsize_t count )
{
  // this function works only for 1D arrays
  assert( H5Sget_simple_extent_ndims( d->id ) == 1 );

  herr_t status = H5Sselect_hyperslab( d->id, H5S_SELECT_SET, &start, NULL, &count, NULL );
  if ( status < 0 )
  {
    MDAL::debug( "Failed to select 1D hyperslab!" );
  }
}

void HdfDataspace::selectHyperslab( const std::vector<hsize_t> offsets,
                                    const std::vector<hsize_t> counts )
{
  assert( H5Sget_simple_extent_ndims( d->id ) == static_cast<int>( offsets.size() ) );
  assert( offsets.size() == counts.size() );

  herr_t status = H5Sselect_hyperslab( d->id,
                                       H5S_SELECT_SET,
                                       offsets.data(),
                                       NULL,
                                       counts.data(),
                                       NULL );
  if ( status < 0 )
  {
    MDAL::debug( "Failed to select 1D hyperslab!" );
  }
}

bool HdfDataspace::isValid() const { return d->id >= 0; }

hid_t HdfDataspace::id() const { return d->id; }
