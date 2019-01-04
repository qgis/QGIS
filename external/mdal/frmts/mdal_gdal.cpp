/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_gdal.hpp"
#include <assert.h>
#include <limits>
#include <gdal.h>
#include <cmath>
#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "gdal_alg.h"
#include "mdal_utils.hpp"

#define MDAL_NODATA -9999

void MDAL::GdalDataset::init( const std::string &dsName )
{
  mDatasetName = dsName;

  // Open dataset
  mHDataset = GDALOpen( dsName.data(), GA_ReadOnly );
  if ( !mHDataset ) throw MDAL_Status::Err_UnknownFormat;

  // Now parse it
  parseParameters();
  parseProj();
}

void MDAL::GdalDataset::parseParameters()
{
  mNBands = static_cast<unsigned int>( GDALGetRasterCount( mHDataset ) );
  if ( mNBands == 0 ) throw MDAL_Status::Err_InvalidData;

  GDALGetGeoTransform( mHDataset, mGT ); // in case of error it returns Identid

  mXSize = static_cast<unsigned int>( GDALGetRasterXSize( mHDataset ) ); //raster width in pixels
  if ( mXSize == 0 ) throw MDAL_Status::Err_InvalidData;

  mYSize = static_cast<unsigned int>( GDALGetRasterYSize( mHDataset ) ); //raster height in pixels
  if ( mYSize == 0 ) throw MDAL_Status::Err_InvalidData;

  mNPoints = mXSize * mYSize;
  mNVolumes = ( mXSize - 1 ) * ( mYSize - 1 );
}

void MDAL::GdalDataset::parseProj()
{
  char *proj = const_cast<char *>( GDALGetProjectionRef( mHDataset ) );
  if ( proj != nullptr )
  {
    mProj = std::string( proj );
  }
}

/******************************************************************************************************/

bool MDAL::DriverGdal::meshes_equals( const MDAL::GdalDataset *ds1, const MDAL::GdalDataset *ds2 ) const
{
  return ( ( ds1->mXSize == ds2->mXSize ) &&
           ( ds1->mYSize == ds2->mYSize ) &&
           ( MDAL::equals( ds1->mGT[0], ds2->mGT[0] ) ) &&
           ( MDAL::equals( ds1->mGT[1], ds2->mGT[1] ) ) &&
           ( MDAL::equals( ds1->mGT[2], ds2->mGT[2] ) ) &&
           ( MDAL::equals( ds1->mGT[3], ds2->mGT[3] ) ) &&
           ( MDAL::equals( ds1->mGT[4], ds2->mGT[4] ) ) &&
           ( MDAL::equals( ds1->mGT[5], ds2->mGT[5] ) ) &&
           ds1->mProj == ds2->mProj );
}


bool MDAL::DriverGdal::initVertices( Vertices &vertices )
{
  Vertex *VertexsPtr = vertices.data();
  unsigned int mXSize = meshGDALDataset()->mXSize;
  unsigned int mYSize = meshGDALDataset()->mYSize;
  const double *mGT = meshGDALDataset()->mGT;

  for ( unsigned int y = 0; y < mYSize; ++y )
  {
    for ( unsigned int x = 0; x < mXSize; ++x, ++VertexsPtr )
    {
      // VertexsPtr->setId(x + mXSize*y);
      VertexsPtr->x = mGT[0] + ( x + 0.5 ) * mGT[1] + ( y + 0.5 ) * mGT[2];
      VertexsPtr->y = mGT[3] + ( x + 0.5 ) * mGT[4] + ( y + 0.5 ) * mGT[5];
      VertexsPtr->z = 0.0;
    }
  }

  BBox extent = computeExtent( vertices );
  // we want to detect situation when there is whole earth represented in dataset
  bool is_longitude_shifted = ( extent.minX >= 0.0 ) &&
                              ( fabs( extent.minX + extent.maxX - 360.0 ) < 1.0 ) &&
                              ( extent.minY >= -90.0 ) &&
                              ( extent.maxX <= 360.0 ) &&
                              ( extent.maxX > 180.0 ) &&
                              ( extent.maxY <= 90.0 );
  if ( is_longitude_shifted )
  {
    for ( Vertices::size_type n = 0; n < vertices.size(); ++n )
    {
      if ( vertices[n].x > 180.0 )
      {
        vertices[n].x -= 360.0;
      }
    }
  }

  return is_longitude_shifted;
}

void MDAL::DriverGdal::initFaces( Vertices &Vertexs, Faces &Faces, bool is_longitude_shifted )
{
  int reconnected = 0;
  unsigned int mXSize = meshGDALDataset()->mXSize;
  unsigned int mYSize = meshGDALDataset()->mYSize;

  size_t i = 0;

  for ( unsigned int y = 0; y < mYSize - 1; ++y )
  {
    for ( unsigned int x = 0; x < mXSize - 1; ++x )
    {
      if ( is_longitude_shifted &&
           ( Vertexs[x + mXSize * y].x > 0.0 ) &&
           ( Vertexs[x + 1 + mXSize * y].x < 0.0 ) )
        // omit border face
      {
        --reconnected;
        continue;
      }

      if ( is_longitude_shifted && ( x == 0 ) )
      {
        // create extra faces around prime meridian
        Faces[i].resize( 4 );
        Faces[i][0] = mXSize * ( y + 1 );
        Faces[i][3] = mXSize * y;
        Faces[i][2] = mXSize - 1 + mXSize * y;
        Faces[i][1] = mXSize - 1 + mXSize * ( y + 1 );

        ++reconnected;
        ++i;
      }

      // other faces
      Faces[i].resize( 4 );
      Faces[i][0] = x + 1 + mXSize * ( y + 1 );
      Faces[i][3] = x + 1 + mXSize * y;
      Faces[i][2] = x + mXSize * y;
      Faces[i][1] = x + mXSize * ( y + 1 );

      ++i;
    }
  }
  //make sure we have discarded same amount of faces that we have added
  assert( reconnected == 0 );
}

std::string MDAL::DriverGdal::GDALFileName( const std::string &fileName )
{
  return fileName;
}

double MDAL::DriverGdal::parseMetadataTime( const std::string &time_s )
{
  std::string time_trimmed = MDAL::trim( time_s );
  std::vector<std::string> times = MDAL::split( time_trimmed, " ", MDAL::SkipEmptyParts );
  return MDAL::toDouble( times[0] );
}

MDAL::DriverGdal::metadata_hash MDAL::DriverGdal::parseMetadata( GDALMajorObjectH gdalObject, const char *pszDomain /* = 0 */ )
{
  MDAL::DriverGdal::metadata_hash meta;
  char **GDALmetadata = nullptr;
  GDALmetadata = GDALGetMetadata( gdalObject, pszDomain );

  if ( GDALmetadata )
  {
    for ( int j = 0; GDALmetadata[j]; ++j )
    {
      std::string metadata_pair = GDALmetadata[j]; //KEY = VALUE
      std::vector<std::string> metadata = MDAL::split( metadata_pair, "=", MDAL::SkipEmptyParts );
      if ( metadata.size() > 1 )
      {
        std::string key = MDAL::toLower( metadata[0] );
        metadata.erase( metadata.begin() ); // remove key
        std::string value = MDAL::join( metadata, "=" );
        meta[key] = value;
      }
    }
  }

  return meta;
}

void MDAL::DriverGdal::parseRasterBands( const MDAL::GdalDataset *cfGDALDataset )
{
  for ( unsigned int i = 1; i <= cfGDALDataset->mNBands; ++i ) // starts with 1 .... ehm....
  {
    // Get Band
    GDALRasterBandH gdalBand = GDALGetRasterBand( cfGDALDataset->mHDataset, static_cast<int>( i ) );
    if ( !gdalBand )
    {
      throw MDAL_Status::Err_InvalidData;
    }

    // Reference time
    metadata_hash global_metadata = parseMetadata( cfGDALDataset->mHDataset );
    parseGlobals( global_metadata );

    // Get metadata
    metadata_hash metadata = parseMetadata( gdalBand );

    std::string band_name;
    double time = std::numeric_limits<double>::min();
    bool is_vector;
    bool is_x;
    if ( parseBandInfo( cfGDALDataset, metadata, band_name, &time, &is_vector, &is_x ) )
    {
      continue;
    }

    // Add to data structures
    std::vector<GDALRasterBandH>::size_type data_count = is_vector ? 2 : 1;
    std::vector<GDALRasterBandH>::size_type data_index = is_x ? 0 : 1;
    if ( mBands.find( band_name ) == mBands.end() )
    {
      // this Face is not yet added at all
      // => create new map
      timestep_map qMap;
      std::vector<GDALRasterBandH> raster_bands( data_count );

      raster_bands[data_index] = gdalBand;
      qMap[time] = raster_bands;
      mBands[band_name] = qMap;

    }
    else
    {
      timestep_map::iterator timestep = mBands[band_name].find( time );
      if ( timestep == mBands[band_name].end() )
      {
        // Face is there, but new timestep
        // => create just new map entry
        std::vector<GDALRasterBandH> raster_bands( data_count );
        raster_bands[data_index] = gdalBand;
        mBands[band_name][time] = raster_bands;

      }
      else
      {
        // Face is there, and timestep too, this must be other part
        // of the existing vector
        timestep->second[data_index] = gdalBand;
      }
    }
  }
}

void MDAL::DriverGdal::addDataToOutput( GDALRasterBandH raster_band, std::shared_ptr<MemoryDataset> tos, bool is_vector, bool is_x )
{
  assert( raster_band );

  double nodata =  GDALGetRasterNoDataValue( raster_band, nullptr );
  unsigned int mXSize = meshGDALDataset()->mXSize;
  unsigned int mYSize = meshGDALDataset()->mYSize;

  double *values = tos->values();

  for ( unsigned int y = 0; y < mYSize; ++y )
  {
    // buffering per-line
    CPLErr err = GDALRasterIO(
                   raster_band,
                   GF_Read,
                   0, //nXOff
                   static_cast<int>( y ), //nYOff
                   static_cast<int>( mXSize ), //nXSize
                   1, //nYSize
                   mPafScanline, //pData
                   static_cast<int>( mXSize ), //nBufXSize
                   1, //nBufYSize
                   GDT_Float64, //eBufType
                   0, //nPixelSpace
                   0 //nLineSpace
                 );
    if ( err != CE_None )
    {
      throw MDAL_Status::Err_InvalidData;
    }

    for ( unsigned int x = 0; x < mXSize; ++x )
    {
      unsigned int idx = x + mXSize * y;
      double val = mPafScanline[x];
      if ( !MDAL::equals( val, nodata ) )
      {
        // values is prepolulated with NODATA values, so store only legal values
        if ( is_vector )
        {
          if ( is_x )
          {
            values[2 * idx] = val;
          }
          else
          {
            values[2 * idx + 1] = val;
          }
        }
        else
        {
          values[idx] = val;
        }
      }
    }
  }
}

void MDAL::DriverGdal::activateFaces( std::shared_ptr<MemoryDataset> tos )
{
  // only for data on vertices
  if ( !tos->group()->isOnVertices() )
    return;

  bool isScalar = tos->group()->isScalar();

  // Activate only Faces that do all Vertex's outputs with some data
  int *active = tos->active();
  const double *values = tos->constValues();

  for ( unsigned int idx = 0; idx < meshGDALDataset()->mNVolumes; ++idx )
  {
    Face elem = mMesh->faces.at( idx );
    for ( size_t i = 0; i < 4; ++i )
    {
      if ( isScalar )
      {
        double val = values[elem[i]];
        if ( std::isnan( val ) )
        {
          active[idx] = 0; //NOT ACTIVE
          break;
        }
      }
      else
      {
        double x = values[elem[2 * i]];
        double y = values[elem[2 * i + 1]];
        if ( std::isnan( x ) || std::isnan( y ) )
        {
          active[idx] = 0; //NOT ACTIVE
          break;
        }
      }
    }
  }
}

void MDAL::DriverGdal::addDatasetGroups()
{
  // Add dataset to mMesh
  for ( data_hash::const_iterator band = mBands.begin(); band != mBands.end(); band++ )
  {
    if ( band->second.empty() )
      continue;

    std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                            name(),
                                            mMesh.get(),
                                            mFileName,
                                            band->first
                                          );
    group->setIsOnVertices( true );
    bool is_vector = ( band->second.begin()->second.size() > 1 );
    group->setIsScalar( !is_vector );

    for ( timestep_map::const_iterator time_step = band->second.begin(); time_step != band->second.end(); time_step++ )
    {
      std::vector<GDALRasterBandH> raster_bands = time_step->second;
      std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MDAL::MemoryDataset >( group.get() );

      dataset->setTime( time_step->first );
      for ( std::vector<GDALRasterBandH>::size_type i = 0; i < raster_bands.size(); ++i )
      {
        addDataToOutput( raster_bands[i], dataset, is_vector, i == 0 );
      }
      activateFaces( dataset );
      dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
      group->datasets.push_back( dataset );
    }

    // TODO use GDALComputeRasterMinMax
    group->setStatistics( MDAL::calculateStatistics( group ) );
    mMesh->datasetGroups.push_back( group );
  }
}

void MDAL::DriverGdal::createMesh()
{
  Vertices vertices( meshGDALDataset()->mNPoints );
  bool is_longitude_shifted = initVertices( vertices );

  Faces faces( meshGDALDataset()->mNVolumes );
  initFaces( vertices, faces, is_longitude_shifted );

  mMesh.reset( new MemoryMesh(
                 name(),
                 vertices.size(),
                 faces.size(),
                 4, //maximum quads
                 computeExtent( vertices ),
                 mFileName
               )
             );
  mMesh->vertices = vertices;
  mMesh->faces = faces;
  bool proj_added = addSrcProj();
  if ( ( !proj_added ) && is_longitude_shifted )
  {
    std::string wgs84( "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs" );
    mMesh->setSourceCrs( wgs84 );
  }
}

bool MDAL::DriverGdal::addSrcProj()
{
  std::string proj = meshGDALDataset()->mProj;
  if ( !proj.empty() )
  {
    mMesh->setSourceCrsFromWKT( proj );
    return true;
  }
  return false;
}

std::vector<std::string> MDAL::DriverGdal::parseDatasetNames( const std::string &fileName )
{
  std::string gdal_name = GDALFileName( fileName );
  std::vector<std::string> ret;

  GDALDatasetH hDataset = GDALOpen( gdal_name.data(), GA_ReadOnly );
  if ( hDataset == nullptr ) throw MDAL_Status::Err_UnknownFormat;

  metadata_hash metadata = parseMetadata( hDataset, "SUBDATASETS" );

  for ( auto iter = metadata.begin(); iter != metadata.end(); ++iter )
  {
    const std::string &key =  iter->first;

    if ( MDAL::endsWith( key, "_name" ) )
    {
      // skip subdataset desc keys, just register names
      ret.push_back( iter->second );
    }
  }

  // there are no GDAL subdatasets
  if ( ret.empty() )
  {
    ret.push_back( gdal_name );
  }

  GDALClose( hDataset );
  return ret;
}

void MDAL::DriverGdal::registerDriver()
{
  // re-register all
  GDALAllRegister();
  // check that our driver exists
  GDALDriverH hDriver = GDALGetDriverByName( mGdalDriverName.data() );
  if ( !hDriver ) throw MDAL_Status::Err_MissingDriver;
}

const MDAL::GdalDataset *MDAL::DriverGdal::meshGDALDataset()
{
  assert( gdal_datasets.size() > 0 );
  return gdal_datasets[0].get();
}

MDAL::DriverGdal::DriverGdal( const std::string &name,
                              const std::string &description,
                              const std::string &filter,
                              const std::string &gdalDriverName ):
  Driver( name, description, filter, Capability::ReadMesh ),
  mGdalDriverName( gdalDriverName ),
  mPafScanline( nullptr )
{}

bool MDAL::DriverGdal::canRead( const std::string &uri )
{
  try
  {
    registerDriver();
    parseDatasetNames( uri );
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverGdal::load( const std::string &fileName, MDAL_Status *status )
{
  mFileName = fileName;
  if ( status ) *status = MDAL_Status::None ;

  mPafScanline = nullptr;
  mMesh.reset();

  try
  {
    registerDriver();

    // some formats like NETCFD has data stored in subdatasets
    std::vector<std::string> subdatasets = parseDatasetNames( mFileName );

    // First parse ALL datasets/bands to gather vector quantities
    // if case they are splitted in different subdatasets
    for ( auto iter = subdatasets.begin(); iter != subdatasets.end(); ++iter )
    {
      std::string gdal_dataset_name = *iter;
      // Parse dataset parameters and projection
      std::shared_ptr<MDAL::GdalDataset> cfGDALDataset = std::make_shared<MDAL::GdalDataset>();
      cfGDALDataset->init( gdal_dataset_name );

      if ( !mMesh )
      {
        // If it is first dataset, create mesh from it
        gdal_datasets.push_back( cfGDALDataset );

        // Init memory for data reader
        mPafScanline = new double [cfGDALDataset->mXSize];

        // Create mMesh
        createMesh();

        // Parse bands
        parseRasterBands( cfGDALDataset.get() );

      }
      else if ( meshes_equals( meshGDALDataset(), cfGDALDataset.get() ) )
      {
        gdal_datasets.push_back( cfGDALDataset );
        // Parse bands
        parseRasterBands( cfGDALDataset.get() );
      }
    }

    // Create MDAL datasets
    addDatasetGroups();
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    mMesh.reset();
  }

  gdal_datasets.clear();

  if ( mPafScanline ) delete[] mPafScanline;

  // do not allow mesh without any valid datasets
  if ( mMesh && ( mMesh->datasetGroups.empty() ) )
  {
    if ( status ) *status = MDAL_Status::Err_InvalidData;
    mMesh.reset();
  }
  return std::unique_ptr<Mesh>( mMesh.release() );
}

void MDAL::DriverGdal::parseBandIsVector( std::string &band_name, bool *is_vector, bool *is_x )
{
  band_name = MDAL::trim( band_name );

  if ( MDAL::startsWith( band_name, "u-", MDAL::CaseInsensitive ) ||
       MDAL::startsWith( band_name, "x-", MDAL::CaseInsensitive ) ||
       MDAL::contains( band_name, "u-component", MDAL::CaseInsensitive ) ||
       MDAL::contains( band_name, "u component", MDAL::CaseInsensitive ) ||
       MDAL::contains( band_name, "U wind component", MDAL::CaseInsensitive ) ||
       MDAL::contains( band_name, "x-component", MDAL::CaseInsensitive ) ||
       MDAL::contains( band_name, "x component", MDAL::CaseInsensitive ) )
  {
    *is_vector = true; // vector
    *is_x =  true; //X-Axis
  }
  else if ( MDAL::startsWith( band_name, "v-", MDAL::CaseInsensitive ) ||
            MDAL::startsWith( band_name, "y-", MDAL::CaseInsensitive ) ||
            MDAL::contains( band_name, "v-component", MDAL::CaseInsensitive ) ||
            MDAL::contains( band_name, "v component", MDAL::CaseInsensitive ) ||
            MDAL::contains( band_name, "V wind component", MDAL::CaseInsensitive ) ||
            MDAL::contains( band_name, "y-component", MDAL::CaseInsensitive ) ||
            MDAL::contains( band_name, "y component", MDAL::CaseInsensitive ) )
  {
    *is_vector = true; // vector
    *is_x =  false; //Y-Axis
  }
  else
  {
    *is_vector = false; // scalar
    *is_x =  true; //X-Axis
  }

  if ( *is_vector )
  {
    band_name = MDAL::replace( band_name, "u-component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "v-component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "U wind component", "wind", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "V wind component", "wind", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "x-component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "y-component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "u-component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "v-component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "x-component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "y-component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "u component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "v component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "x component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "y component of", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "u component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "v component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "x component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "y component", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "u-", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "v-", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "x-", "", MDAL::CaseInsensitive );
    band_name = MDAL::replace( band_name, "y-", "", MDAL::CaseInsensitive );
    band_name = MDAL::trim( band_name );
  }
}
