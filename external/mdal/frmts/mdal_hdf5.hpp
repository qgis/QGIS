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
#include "mdal_logger.hpp"
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
template <> inline void hdfClose<H5I_DATATYPE>( hid_t id ) { H5Tclose( id ); }

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
class HdfDataType;

class HdfFile
{
  public:
    enum Mode
    {
      ReadOnly,
      ReadWrite,
      Create
    };
    typedef HdfH<H5I_FILE> Handle;
    typedef std::shared_ptr<Handle> SharedHandle;

    HdfFile( const std::string &path, HdfFile::Mode mode );
    ~HdfFile();
    bool isValid() const;
    hid_t id() const;

    inline std::vector<std::string> groups() const;

    inline HdfGroup group( const std::string &path ) const;

    //!  Creates a group with an absolute path
    inline HdfGroup createGroup( const std::string &path ) const;

    /**
     *  Creates a group with the id of location and a path that can be relative from the location
     *  (see https://docs.hdfgroup.org/hdf5/v1_12/group___h5_g.html#ga86d93295965f750ef25dea2505a711d9)
     */
    inline HdfGroup createGroup( hid_t locationId, const std::string &path ) const;

    inline HdfDataset dataset( const std::string &path ) const;
    inline HdfDataset dataset( const std::string &path, HdfDataType dtype, size_t nItems = 1 ) const;
    inline HdfDataset dataset( const std::string &path, HdfDataType dtype, HdfDataspace dataspace ) const;
    inline HdfAttribute attribute( const std::string &attr_name ) const;
    inline bool pathExists( const std::string &path ) const;
    std::string filePath() const;

  protected:
    SharedHandle d;
    std::string mPath;
};

typedef std::shared_ptr<HdfFile> HdfFileShared;

class HdfDataType
{
  public:
    typedef HdfH<H5I_DATATYPE> Handle;
    HdfDataType();
    HdfDataType( hid_t type, bool isNativeType = true );
    ~HdfDataType();

    // Creates new string type with size, use HDF_MAX_NAME for maximum length
    static HdfDataType createString( int size = HDF_MAX_NAME );
    bool isValid() const;
    hid_t id() const;


  protected:
    std::shared_ptr<Handle> d;
    hid_t mNativeId = -1;
};

class HdfGroup
{
  public:
    typedef HdfH<H5I_GROUP> Handle;

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

  private:
    HdfGroup( HdfFile::SharedHandle file, const std::string &path );
    HdfGroup( std::shared_ptr<Handle> handle, HdfFile::SharedHandle file );

  private:
    HdfFile::SharedHandle mFile; //must be declared before "std::shared_ptr<Handle> d" to be sure it will be the last destroyed

  protected:
    std::shared_ptr<Handle> d;

    friend class HdfFile;
};

class HdfAttribute
{
  public:
    typedef HdfH<H5I_ATTR> Handle;

    //! Create new attribute for writing for 1 item
    HdfAttribute( hid_t obj_id, const std::string &attr_name, HdfDataType type );

    //! Open existing attribute for reading
    HdfAttribute( hid_t obj_id, const std::string &attr_name );
    ~HdfAttribute();
    bool isValid() const;
    hid_t id() const;

    std::string readString() const;
    double readDouble() const;

    void write( const std::string &value );
    void write( int value );

  protected:
    std::shared_ptr<Handle> d;
    hid_t m_objId;
    std::string m_name;
    HdfDataType mType; // when in write mode
};



class HdfDataspace
{
  public:
    typedef HdfH<H5I_DATASPACE> Handle;

    //! memory dataspace for simple N-D array
    HdfDataspace( const std::vector<hsize_t> &dims );
    //! dataspace of the dataset
    HdfDataspace( hid_t dataset = -1 );
    ~HdfDataspace( );
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

    //! creates invalid dataset
    HdfDataset() = default;
    ~HdfDataset();
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
        MDAL::Log::debug( "Failed to read data!" );
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
        MDAL::Log::debug( "Failed to read data!" );
        return std::vector<T>();
      }
      return data;
    }

    //! Reads float value
    float readFloat() const;

    //! Reads string value
    std::string readString() const;

    //! Writes string dataset with single entry
    void write( const std::string &value );

    //! Writes array of float data
    void write( std::vector<float> &value );
    void write( float value );

    //! Writes array of double data
    void write( std::vector<double> &value );

  private:
    //! Creates new, simple 1 dimensional dataset
    HdfDataset( HdfFile::SharedHandle file, const std::string &path, HdfDataType dtype, size_t nItems = 1 );
    //! Creates new dataset with custom dimensions
    HdfDataset( HdfFile::SharedHandle file, const std::string &path, HdfDataType dtype, HdfDataspace dataspace );
    //! Opens dataset for reading
    HdfDataset( HdfFile::SharedHandle file, const std::string &path );

  private:
    HdfFile::SharedHandle mFile; //must be declared before "std::shared_ptr<Handle> d" to be sure it will be the last destroyed

  protected:
    std::shared_ptr<Handle> d;
    HdfDataType mType; // when in write mode

    friend class HdfFile;
    friend class HdfGroup;
};

inline std::vector<std::string> HdfFile::groups() const { return group( "/" ).groups(); }

inline HdfGroup HdfFile::group( const std::string &path ) const { return HdfGroup( d, path ); }

inline HdfGroup HdfFile::createGroup( const std::string &path ) const
{
  return HdfGroup( std::make_shared< HdfGroup::Handle >( H5Gcreate2( d->id, path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ) ), d );
}

inline HdfGroup HdfFile::createGroup( hid_t locationId, const std::string &path ) const
{
  return HdfGroup( std::make_shared< HdfGroup::Handle >( H5Gcreate2( locationId, path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ) ), d );
}

inline HdfDataset HdfFile::dataset( const std::string &path ) const { return HdfDataset( d, path ); }

inline HdfDataset HdfFile::dataset( const std::string &path, HdfDataType dtype, size_t nItems ) const { return HdfDataset( d, path, dtype, nItems ); }

inline HdfDataset HdfFile::dataset( const std::string &path, HdfDataType dtype, HdfDataspace dataspace ) const {return HdfDataset( d, path, dtype, dataspace );}

inline HdfGroup HdfGroup::group( const std::string &groupName ) const { return HdfGroup( mFile, childPath( groupName ) ); }

inline HdfDataset HdfGroup::dataset( const std::string &dsName ) const { return HdfDataset( mFile, childPath( dsName ) ); }

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
