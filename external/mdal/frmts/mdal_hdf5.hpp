/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#ifndef MDAL_HDF5_HPP
#define MDAL_HDF5_HPP

/** A simple C++ wrapper around HDF5 library API */

// for compatibility (older hdf5 version in Travis)
#define H5Gopen_vers 1

#include "mdal_utils.hpp"
#include "hdf5.h"
typedef unsigned char uchar;

#include <memory>
#include <vector>
#include <string>
#include <assert.h>
#include "stdlib.h"

#define HDF_MAX_NAME 1024
struct HdfString
{
  char data [HDF_MAX_NAME];
};

template <int TYPE> inline void hdfClose( hid_t id ) { MDAL_UNUSED( id ); assert( false ); }
template <> inline void hdfClose<H5I_FILE>( hid_t id ) { H5Fclose( id ); }
template <> inline void hdfClose<H5I_GROUP>( hid_t id ) { H5Gclose( id ); }
template <> inline void hdfClose<H5I_DATASET>( hid_t id ) { H5Dclose( id ); }
template <> inline void hdfClose<H5I_ATTR>( hid_t id ) { H5Dclose( id ); }

template <int TYPE>
class HdfH
{
  public:
    HdfH( hid_t hid ) : id( hid ) {}
    HdfH( const HdfH &other ) : id( other.id ) { }
    ~HdfH() { if ( id >= 0 ) hdfClose<TYPE>( id ); }

    hid_t id;
};

class HdfGroup;
class HdfDataset;
class HdfAttribute;

class HdfFile
{
  public:
    typedef HdfH<H5I_FILE> Handle;

    HdfFile( const std::string &path )
      : d( std::make_shared< Handle >( H5Fopen( path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT ) ) )
    {
    }

    bool isValid() const { return d->id >= 0; }
    hid_t id() const { return d->id; }

    inline std::vector<std::string> groups() const;

    inline HdfGroup group( const std::string &path ) const;
    inline HdfDataset dataset( const std::string &path ) const;
    inline HdfAttribute attribute( const std::string &attr_name ) const;

  protected:
    std::shared_ptr<Handle> d;
};

class HdfGroup
{
  public:
    typedef HdfH<H5I_GROUP> Handle;

    HdfGroup( hid_t file, const std::string &path )
      : d( std::make_shared< Handle >( H5Gopen( file, path.c_str() ) ) )
    {}

    bool isValid() const { return d->id >= 0; }
    hid_t id() const { return d->id; }
    hid_t file_id() const { return H5Iget_file_id( d->id ); }

    std::string name() const
    {
      char name[HDF_MAX_NAME];
      H5Iget_name( d->id, name, HDF_MAX_NAME );
      return std::string( name );
    }

    std::vector<std::string> groups() const { return objects( H5G_GROUP ); }
    std::vector<std::string> datasets() const { return objects( H5G_DATASET ); }
    std::vector<std::string> objects() const { return objects( H5G_UNKNOWN ); }

    std::string childPath( const std::string &childName ) const { return name() + "/" + childName; }

    inline HdfGroup group( const std::string &groupName ) const;
    inline HdfDataset dataset( const std::string &dsName ) const;
    inline HdfAttribute attribute( const std::string &attr_name ) const;

  protected:
    std::vector<std::string> objects( H5G_obj_t type ) const
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

  protected:
    std::shared_ptr<Handle> d;
};


class HdfAttribute
{
  public:
    typedef HdfH<H5I_ATTR> Handle;

    HdfAttribute( hid_t obj_id, const std::string &attr_name )
      : d( std::make_shared< Handle >( H5Aopen( obj_id, attr_name.c_str(), H5P_DEFAULT ) ) )
    {}

    bool isValid() const { return d->id >= 0; }
    hid_t id() const { return d->id; }

    std::string readString() const
    {
      char name[HDF_MAX_NAME];
      hid_t datatype = H5Tcopy( H5T_C_S1 );
      H5Tset_size( datatype, HDF_MAX_NAME );
      herr_t status = H5Aread( d->id, datatype, name );
      if ( status < 0 )
      {
        //MDAL::debug("Failed to read data!");
        return std::string();
      }
      H5Tclose( datatype );
      return std::string( name );
    }
  protected:
    std::shared_ptr<Handle> d;
};

class HdfDataset
{
  public:
    typedef HdfH<H5I_DATASET> Handle;

    HdfDataset( hid_t file, const std::string &path )
      : d( std::make_shared< Handle >( H5Dopen2( file, path.c_str(), H5P_DEFAULT ) ) )
    {}

    bool isValid() const { return d->id >= 0; }
    hid_t id() const { return d->id; }

    std::vector<hsize_t> dims() const
    {
      hid_t sid = H5Dget_space( d->id );
      std::vector<hsize_t> d( H5Sget_simple_extent_ndims( sid ) );
      H5Sget_simple_extent_dims( sid, d.data(), NULL );
      H5Sclose( sid );
      return d;
    }

    hsize_t elementCount() const
    {
      hsize_t count = 1;
      for ( hsize_t dsize : dims() )
        count *= dsize;
      return count;
    }

    H5T_class_t type() const
    {
      hid_t tid = H5Dget_type( d->id );
      H5T_class_t t_class = H5Tget_class( tid );
      H5Tclose( tid );
      return t_class;
    }

    std::vector<uchar> readArrayUint8() const { return readArray<uchar>( H5T_NATIVE_UINT8 ); }

    std::vector<float> readArray() const { return readArray<float>( H5T_NATIVE_FLOAT ); }

    std::vector<double> readArrayDouble() const { return readArray<double>( H5T_NATIVE_DOUBLE ); }

    std::vector<int> readArrayInt() const { return readArray<int>( H5T_NATIVE_INT ); }

    std::vector<std::string> readArrayString() const
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


    template <typename T> std::vector<T> readArray( hid_t mem_type_id ) const
    {
      hsize_t cnt = elementCount();
      std::vector<T> data( cnt );
      herr_t status = H5Dread( d->id, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data() );
      if ( status < 0 )
      {
        MDAL::debug( "Failed to read data!" );
        return std::vector<T>();
      }
      return data;
    }

    float readFloat() const
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

    std::string readString() const
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

  protected:
    std::shared_ptr<Handle> d;
};

inline std::vector<std::string> HdfFile::groups() const { return group( "/" ).groups(); }

inline HdfGroup HdfFile::group( const std::string &path ) const { return HdfGroup( d->id, path ); }

inline HdfDataset HdfFile::dataset( const std::string &path ) const { return HdfDataset( d->id, path ); }

inline HdfGroup HdfGroup::group( const std::string &groupName ) const { return HdfGroup( file_id(), childPath( groupName ) ); }

inline HdfDataset HdfGroup::dataset( const std::string &dsName ) const { return HdfDataset( file_id(), childPath( dsName ) ); }

inline HdfAttribute HdfFile::attribute( const std::string &attr_name ) const { return HdfAttribute( d->id, attr_name ); }

inline HdfAttribute HdfGroup::attribute( const std::string &attr_name ) const { return HdfAttribute( d->id, attr_name ); }

#endif // MDAL_HDF5_HPP
