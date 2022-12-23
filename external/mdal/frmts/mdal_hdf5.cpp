/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include "mdal_hdf5.hpp"
#include <cstring>
#include <algorithm>

HdfFile::HdfFile( const std::string &path, HdfFile::Mode mode )
  : mPath( path )
{
  switch ( mode )
  {
    case HdfFile::ReadOnly:
      if ( H5Fis_hdf5( mPath.c_str() ) > 0 )
        d = std::make_shared< Handle >( H5Fopen( path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT ) );
      break;
    case HdfFile::ReadWrite:
      if ( H5Fis_hdf5( mPath.c_str() ) > 0 )
        d = std::make_shared< Handle >( H5Fopen( path.c_str(), H5F_ACC_RDWR, H5P_DEFAULT ) );
      break;
    case HdfFile::Create:
      d = std::make_shared< Handle >( H5Fcreate( path.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT ) );
      break;
  }
}

HdfFile::~HdfFile() = default;

bool HdfFile::isValid() const { return d && ( d->id >= 0 ); }

hid_t HdfFile::id() const { return d->id; }

std::string HdfFile::filePath() const
{
  return mPath;
}

HdfGroup::HdfGroup( HdfFile::SharedHandle file, const std::string &path )
  : mFile( file )
{
  d = std::make_shared< Handle >( H5Gopen( file->id, path.c_str() ) );
}

HdfGroup::HdfGroup( std::shared_ptr<Handle> handle, HdfFile::SharedHandle file )
  : mFile( file )
  , d( handle )
{
}

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

HdfAttribute::HdfAttribute( hid_t obj_id, const std::string &attr_name, HdfDataType type )
  : mType( type )
{
  std::vector<hsize_t> dimsSingle = {1};
  HdfDataspace dsc( dimsSingle );
  d = std::make_shared< Handle >( H5Acreate2( obj_id, attr_name.c_str(), type.id(), dsc.id(), H5P_DEFAULT, H5P_DEFAULT ) );
}

HdfAttribute::HdfAttribute( hid_t obj_id, const std::string &attr_name )
  : m_objId( obj_id ), m_name( attr_name )
{
  d = std::make_shared< Handle >( H5Aopen( obj_id, attr_name.c_str(), H5P_DEFAULT ) );
}

HdfAttribute::~HdfAttribute() = default;

bool HdfAttribute::isValid() const { return d->id >= 0; }

hid_t HdfAttribute::id() const { return d->id; }

std::string HdfAttribute::readString() const
{
  HdfDataType datatype( H5Aget_type( id() ) );
  char name[HDF_MAX_NAME + 1];
  std::memset( name, '\0', HDF_MAX_NAME + 1 );
  herr_t status = H5Aread( d->id, datatype.id(), name );
  if ( status < 0 )
  {
    return std::string();
  }
  std::string res( name );
  res = MDAL::trim( res );
  return res;
}

double HdfAttribute::readDouble() const
{
  HdfDataType datatype( H5Aget_type( id() ) );
  double value;
  herr_t status = H5Aread( d->id, H5T_NATIVE_DOUBLE, &value );
  if ( status < 0 )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return value;
}

void HdfAttribute::write( const std::string &value )
{
  if ( !isValid() || !mType.isValid() )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Write failed due to invalid data" );

  // make sure you do not store more than it is possible
  std::vector<char> buf( HDF_MAX_NAME + 1, '\0' );
  size_t size = value.size() < HDF_MAX_NAME  ? value.size() : HDF_MAX_NAME;
  memcpy( buf.data(), value.c_str(), size );

  if ( H5Awrite( d->id, mType.id(), buf.data() ) < 0 )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Could not write data" );
}

void HdfAttribute::write( int value )
{
  if ( !isValid() || !mType.isValid() )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Write failed due to invalid data" );

  if ( H5Awrite( d->id, mType.id(), &value ) < 0 )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Could not write data" );
}

HdfDataset::HdfDataset( HdfFile::SharedHandle file, const std::string &path, HdfDataType dtype, size_t nItems )
  : mFile( file ),
    mType( dtype )
{
  // Crete dataspace for attribute
  std::vector<hsize_t> dimsSingle = {nItems};
  HdfDataspace dsc( dimsSingle );

  d = std::make_shared< Handle >( H5Dcreate2( file->id, path.c_str(), dtype.id(), dsc.id(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ) );
}


HdfDataset::HdfDataset( HdfFile::SharedHandle file, const std::string &path, HdfDataType dtype, HdfDataspace dataspace )
  : mFile( file ),
    mType( dtype )
{
  d = std::make_shared< Handle >( H5Dcreate2( file->id, path.c_str(), dtype.id(), dataspace.id(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ) );
}

HdfDataset::HdfDataset( HdfFile::SharedHandle file, const std::string &path )
  : mFile( file ),
    d( std::make_shared< Handle >( H5Dopen2( file->id, path.c_str(), H5P_DEFAULT ) ) )
{
}

HdfDataset::~HdfDataset() = default;

bool HdfDataset::isValid() const { return  d && d->id >= 0; }

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
  if ( mType.isValid() )
    return H5Tget_class( mType.id() );
  else
  {
    HdfDataType dt( H5Dget_type( d->id ) );
    return H5Tget_class( dt.id() );
  }
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

  HdfDataType datatype = HdfDataType::createString();
  std::vector<HdfString> arr = readArray<HdfString>( datatype.id() );

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
    MDAL::Log::debug( "Not scalar!" );
    return 0;
  }

  float value;
  herr_t status = H5Dread( d->id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value );
  if ( status < 0 )
  {
    MDAL::Log::debug( "Failed to read data!" );
    return 0;
  }
  return value;
}

void HdfDataset::write( std::vector<float> &value )
{
  if ( !isValid() || !mType.isValid() )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Write failed due to invalid data" );

  // Write float array to dataset
  if ( H5Dwrite( d->id, mType.id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, value.data() ) < 0 )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Could not write float array to dataset" );
}

void HdfDataset::write( float value )
{
  if ( !isValid() || !mType.isValid() )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Write failed due to invalid data" );

  // Write float array to dataset
  if ( H5Dwrite( d->id, mType.id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, &value ) < 0 )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Could not write float to dataset" );
}

void HdfDataset::write( std::vector<double> &value )
{
  if ( !isValid() || !mType.isValid() )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Write failed due to invalid data" );

  // Write double array to dataset.
  if ( H5Dwrite( d->id, mType.id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, value.data() ) < 0 )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Could not write double array to dataset" );
}

void HdfDataset::write( const std::string &value )
{
  if ( !isValid() || !mType.isValid() )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Write failed due to invalid data" );

  // make sure you do not store more than it is possible
  std::vector<char> buf( HDF_MAX_NAME + 1, '\0' );
  size_t size = value.size() < HDF_MAX_NAME  ? value.size() : HDF_MAX_NAME;
  memcpy( buf.data(), value.c_str(), size );

  // Write string to dataset.
  if ( H5Dwrite( d->id, mType.id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, buf.data() ) < 0 )
    throw MDAL::Error( MDAL_Status::Err_FailToWriteToDisk, "Could not write string to dataset" );
}

std::string HdfDataset::readString() const
{
  if ( elementCount() != 1 )
  {
    MDAL::Log::debug( "Not scalar!" );
    return std::string();
  }

  char name[HDF_MAX_NAME];
  HdfDataType datatype = HdfDataType::createString();
  herr_t status = H5Dread( d->id, datatype.id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, name );
  if ( status < 0 )
  {
    MDAL::Log::debug( "Failed to read data!" );
    return std::string();
  }
  return std::string( name );
}

HdfDataspace::HdfDataspace( const std::vector<hsize_t> &dims )
{
  d = std::make_shared< Handle >( H5Screate_simple(
                                    static_cast<int>( dims.size() ),
                                    dims.data(),
                                    dims.data()
                                  ) );
}


HdfDataspace::HdfDataspace( hid_t dataset )
{
  if ( dataset >= 0 )
    d = std::make_shared< Handle >( H5Dget_space( dataset ) );
}

HdfDataspace::~HdfDataspace() = default;

void HdfDataspace::selectHyperslab( hsize_t start, hsize_t count )
{
  // this function works only for 1D arrays
  assert( H5Sget_simple_extent_ndims( d->id ) == 1 );

  herr_t status = H5Sselect_hyperslab( d->id, H5S_SELECT_SET, &start, NULL, &count, NULL );
  if ( status < 0 )
  {
    MDAL::Log::debug( "Failed to select 1D hyperslab!" );
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
    MDAL::Log::debug( "Failed to select 1D hyperslab!" );
  }
}

bool HdfDataspace::isValid() const { return d->id >= 0; }

hid_t HdfDataspace::id() const { return d->id; }


HdfDataType::HdfDataType() = default;

HdfDataType::HdfDataType( hid_t type, bool isNativeType )
{
  if ( isNativeType )
    mNativeId = type;
  else
    d = std::make_shared< Handle >( type );
}

HdfDataType HdfDataType::createString( int size )
{
  assert( size > 0 );
  if ( size > HDF_MAX_NAME )
    size = HDF_MAX_NAME;

  hid_t atype = H5Tcopy( H5T_C_S1 );
  H5Tset_size( atype, static_cast<size_t>( size ) );
  H5Tset_strpad( atype, H5T_STR_NULLTERM );
  return HdfDataType( atype, false );
}

HdfDataType::~HdfDataType() = default;

bool HdfDataType::isValid() const
{
  if ( d )
    return d->id >= 0;
  else
    return mNativeId >= 0;
}

hid_t HdfDataType::id() const
{
  if ( d )
    return d->id;
  else
    return mNativeId;
}
