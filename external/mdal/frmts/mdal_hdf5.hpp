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
#include <numeric>

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
template <> inline void hdfClose<H5I_ATTR>( hid_t id ) { H5Aclose( id ); }
template <> inline void hdfClose<H5I_DATASPACE>( hid_t id ) { H5Sclose( id ); }

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
class HdfDataspace;

class HdfFile
{
  public:
    typedef HdfH<H5I_FILE> Handle;

    HdfFile( const std::string &path );

    bool isValid() const;
    hid_t id() const;

    inline std::vector<std::string> groups() const;

    inline HdfGroup group( const std::string &path ) const;
    inline HdfDataset dataset( const std::string &path ) const;
    inline HdfAttribute attribute( const std::string &attr_name ) const;
    inline bool pathExists( const std::string &path ) const;

  protected:
    std::shared_ptr<Handle> d;
};

class HdfGroup
{
  public:
    typedef HdfH<H5I_GROUP> Handle;

    HdfGroup( hid_t file, const std::string &path );

    bool isValid() const;
    hid_t id() const;
    hid_t file_id() const;

    std::string name() const;

    std::vector<std::string> groups() const;
    std::vector<std::string> datasets() const;
    std::vector<std::string> objects() const;

    std::string childPath( const std::string &childName ) const;

    inline HdfGroup group( const std::string &groupName ) const;
    inline HdfDataset dataset( const std::string &dsName ) const;
    inline HdfAttribute attribute( const std::string &attr_name ) const;
    inline bool pathExists( const std::string &path ) const;
  protected:
    std::vector<std::string> objects( H5G_obj_t type ) const;

  protected:
    std::shared_ptr<Handle> d;
};


class HdfAttribute
{
  public:
    typedef HdfH<H5I_ATTR> Handle;

    HdfAttribute( hid_t obj_id, const std::string &attr_name );

    bool isValid() const;
    hid_t id() const;

    std::string readString() const;
  protected:
    std::shared_ptr<Handle> d;
};

class HdfDataspace
{
  public:
    typedef HdfH<H5I_DATASPACE> Handle;
    //! memory dataspace for simple N-D array
    HdfDataspace( const std::vector<hsize_t> &dims );
    //! dataspace of the dataset
    HdfDataspace( hid_t dataset );
    //! select from 1D array
    void selectHyperslab( hsize_t start, hsize_t count );
    //! select from N-D array
    void selectHyperslab( const std::vector<hsize_t> offsets,
                          const std::vector<hsize_t> counts );

    bool isValid() const;
    hid_t id() const;

  protected:
    std::shared_ptr<Handle> d;
};

class HdfDataset
{
  public:
    typedef HdfH<H5I_DATASET> Handle;

    HdfDataset( hid_t file, const std::string &path );

    bool isValid() const;
    hid_t id() const;

    std::vector<hsize_t> dims() const;

    hsize_t elementCount() const;

    H5T_class_t type() const;

    //! Reads full array into vector
    //! Array can have any number of dimenstions
    //! and it is fully read into 1D vector
    std::vector<uchar> readArrayUint8() const;
    std::vector<float> readArray() const;
    std::vector<double> readArrayDouble() const;
    std::vector<int> readArrayInt() const;
    std::vector<std::string> readArrayString() const;

    //! Reads part of the N-D array into vector,
    //! for each dimension specified by offset and count
    //! size of offsets and counts must be same as rank (number of dims) of dataset
    //! the results array is 1D
    std::vector<uchar> readArrayUint8( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const;
    std::vector<float> readArray( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const;
    std::vector<double> readArrayDouble( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const;
    std::vector<int> readArrayInt( const std::vector<hsize_t> offsets, const std::vector<hsize_t> counts ) const;

    inline bool hasAttribute( const std::string &attr_name ) const;
    inline HdfAttribute attribute( const std::string &attr_name ) const;

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

    template <typename T> std::vector<T> readArray( hid_t mem_type_id,
        const std::vector<hsize_t> offsets,
        const std::vector<hsize_t> counts ) const
    {
      HdfDataspace dataspace( d->id );
      dataspace.selectHyperslab( offsets, counts );

      hsize_t totalItems = 1;
      for ( auto it = counts.begin(); it != counts.end(); ++it )
        totalItems *= *it;

      std::vector<hsize_t> dims = {totalItems};
      HdfDataspace memspace( dims );
      memspace.selectHyperslab( 0, totalItems );

      std::vector<T> data( totalItems );
      herr_t status = H5Dread( d->id, mem_type_id, memspace.id(), dataspace.id(), H5P_DEFAULT, data.data() );
      if ( status < 0 )
      {
        MDAL::debug( "Failed to read data!" );
        return std::vector<T>();
      }
      return data;
    }

    float readFloat() const;

    std::string readString() const;

  protected:
    std::shared_ptr<Handle> d;
};

inline std::vector<std::string> HdfFile::groups() const { return group( "/" ).groups(); }

inline HdfGroup HdfFile::group( const std::string &path ) const { return HdfGroup( d->id, path ); }

inline HdfDataset HdfFile::dataset( const std::string &path ) const { return HdfDataset( d->id, path ); }

inline HdfGroup HdfGroup::group( const std::string &groupName ) const { return HdfGroup( file_id(), childPath( groupName ) ); }

inline HdfDataset HdfGroup::dataset( const std::string &dsName ) const { return HdfDataset( file_id(), childPath( dsName ) ); }

inline bool HdfDataset::hasAttribute( const std::string &attr_name ) const
{
  htri_t res = H5Aexists( d->id, attr_name.c_str() );
  return  res > 0 ;
}

inline HdfAttribute HdfFile::attribute( const std::string &attr_name ) const { return HdfAttribute( d->id, attr_name ); }

inline HdfAttribute HdfGroup::attribute( const std::string &attr_name ) const { return HdfAttribute( d->id, attr_name ); }

inline HdfAttribute HdfDataset::attribute( const std::string &attr_name ) const { return HdfAttribute( d->id, attr_name ); }

inline bool HdfFile::pathExists( const std::string &path ) const { return H5Lexists( d->id, path.c_str(), H5P_DEFAULT ) > 0; }

inline bool HdfGroup::pathExists( const std::string &path ) const { return H5Lexists( d->id, path.c_str(), H5P_DEFAULT ) > 0; }

#endif // MDAL_HDF5_HPP
