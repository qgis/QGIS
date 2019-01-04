/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <stddef.h>
#include <limits>
#include <assert.h>
#include <memory>

#include "mdal.h"
#include "mdal_driver_manager.hpp"
#include "mdal_data_model.hpp"
#include "mdal_utils.hpp"

#define NODATA std::numeric_limits<double>::quiet_NaN()

static const char *EMPTY_STR = "";

static MDAL_Status sLastStatus;

const char *MDAL_Version()
{
  return "0.1.3";
}

MDAL_Status MDAL_LastStatus()
{
  return sLastStatus;
}

// helper to return string data - without having to deal with memory too much.
// returned pointer is valid only next call. also not thread-safe.
const char *_return_str( const std::string &str )
{
  static std::string lastStr;
  lastStr = str;
  return lastStr.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////
/// DRIVERS
///////////////////////////////////////////////////////////////////////////////////////

int MDAL_driverCount()
{
  size_t count = MDAL::DriverManager::instance().driversCount();
  return static_cast<int>( count );
}

DriverH MDAL_driverFromIndex( int index )
{
  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return nullptr;
  }

  size_t idx = static_cast<size_t>( index );
  std::shared_ptr<MDAL::Driver> driver = MDAL::DriverManager::instance().driver( idx );
  return static_cast<DriverH>( driver.get() );
}

DriverH MDAL_driverFromName( const char *name )
{
  std::string nm = name;
  std::shared_ptr<MDAL::Driver> driver = MDAL::DriverManager::instance().driver( nm );
  return static_cast<DriverH>( driver.get() );
}

bool MDAL_DR_meshLoadCapability( DriverH driver )
{
  if ( !driver )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return false;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return d->hasCapability( MDAL::Capability::ReadMesh );
}

bool MDAL_DR_writeDatasetsCapability( DriverH driver )
{
  if ( !driver )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return false;
  }
  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return d->hasCapability( MDAL::Capability::WriteDatasets );
}

const char *MDAL_DR_longName( DriverH driver )
{
  if ( !driver )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return EMPTY_STR;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->longName() );
}

const char *MDAL_DR_name( DriverH driver )
{
  if ( !driver )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return EMPTY_STR;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->name() );
}

const char *MDAL_DR_filters( DriverH driver )
{
  if ( !driver )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return EMPTY_STR;
  }
  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->filters() );
}

///////////////////////////////////////////////////////////////////////////////////////
/// MESH
///////////////////////////////////////////////////////////////////////////////////////

MeshH MDAL_LoadMesh( const char *meshFile )
{
  if ( !meshFile )
  {
    sLastStatus = MDAL_Status::Err_FileNotFound;
    return nullptr;
  }

  std::string filename( meshFile );
  return static_cast< MeshH >( MDAL::DriverManager::instance().load( filename, &sLastStatus ).release() );
}


void MDAL_CloseMesh( MeshH mesh )
{
  if ( mesh )
  {
    MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
    delete m;
  }
}

const char *MDAL_M_projection( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return EMPTY_STR;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  return _return_str( m->crs() );
}

void MDAL_M_extent( MeshH mesh, double *minX, double *maxX, double *minY, double *maxY )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    *minX = std::numeric_limits<double>::quiet_NaN();
    *maxX = std::numeric_limits<double>::quiet_NaN();
    *minY = std::numeric_limits<double>::quiet_NaN();
    *maxY = std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
    const MDAL::BBox extent = m->extent();
    *minX = extent.minX;
    *maxX = extent.maxX;
    *minY = extent.minY;
    *maxY = extent.maxY;
  }
}

int MDAL_M_vertexCount( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->verticesCount() );
  return len;
}

int MDAL_M_faceCount( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->facesCount() );
  return len;
}

int MDAL_M_faceVerticesMaximumCount( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->faceVerticesMaximumCount() );
  return len;
}

void MDAL_M_LoadDatasets( MeshH mesh, const char *datasetFile )
{
  if ( !datasetFile )
  {
    sLastStatus = MDAL_Status::Err_FileNotFound;
    return;
  }

  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );

  std::string filename( datasetFile );
  MDAL::DriverManager::instance().loadDatasets( m, datasetFile, &sLastStatus );
}

int MDAL_M_datasetGroupCount( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->datasetGroups.size() );
  return len;
}

DatasetGroupH MDAL_M_datasetGroup( MeshH mesh, int index )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }

  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->datasetGroups.size() );
  if ( len <= index )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }
  size_t i = static_cast<size_t>( index );
  return static_cast< DatasetH >( m->datasetGroups[i].get() );
}

DatasetGroupH MDAL_M_addDatasetGroup(
  MeshH mesh,
  const char *name,
  bool isOnVertices,
  bool hasScalarData,
  DriverH driver,
  const char *datasetGroupFile )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }

  if ( !name )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return nullptr;
  }

  if ( !datasetGroupFile )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return nullptr;
  }

  if ( !driver )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return nullptr;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  MDAL::Driver *dr = static_cast< MDAL::Driver * >( driver );

  if ( !dr->hasCapability( MDAL::Capability::WriteDatasets ) )
  {
    sLastStatus = MDAL_Status::Err_MissingDriverCapability;
    return nullptr;
  }

  const size_t index = m->datasetGroups.size();
  dr->createDatasetGroup( m,
                          name,
                          isOnVertices,
                          hasScalarData,
                          datasetGroupFile
                        );
  if ( index < m->datasetGroups.size() ) // we have new dataset group
    return static_cast< DatasetGroupH >( m->datasetGroups[ index ].get() );
  else
    return nullptr;
}

const char *MDAL_M_driverName( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  return _return_str( m->driverName() );
}

///////////////////////////////////////////////////////////////////////////////////////
/// MESH VERTICES
///////////////////////////////////////////////////////////////////////////////////////

MeshVertexIteratorH MDAL_M_vertexIterator( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  std::unique_ptr<MDAL::MeshVertexIterator> it = m->readVertices();
  return static_cast< MeshVertexIteratorH >( it.release() );
}

int MDAL_VI_next( MeshVertexIteratorH iterator, int verticesCount, double *coordinates )
{
  if ( !iterator )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::MeshVertexIterator *it = static_cast< MDAL::MeshVertexIterator * >( iterator );
  size_t size = static_cast<size_t>( verticesCount );
  if ( size == 0 )
  {
    return 0;
  }
  size_t ret = it->next( size, coordinates );
  return static_cast<int>( ret );
}

void MDAL_VI_close( MeshVertexIteratorH iterator )
{
  if ( iterator )
  {
    MDAL::MeshVertexIterator *it = static_cast< MDAL::MeshVertexIterator * >( iterator );
    delete it;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
/// MESH FACES
///////////////////////////////////////////////////////////////////////////////////////

MeshFaceIteratorH MDAL_M_faceIterator( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  std::unique_ptr<MDAL::MeshFaceIterator > it = m->readFaces();
  return static_cast< MeshFaceIteratorH >( it.release() );
}

int MDAL_FI_next( MeshFaceIteratorH iterator,
                  int faceOffsetsBufferLen,
                  int *faceOffsetsBuffer,
                  int vertexIndicesBufferLen,
                  int *vertexIndicesBuffer )
{
  if ( !iterator )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::MeshFaceIterator *it = static_cast< MDAL::MeshFaceIterator * >( iterator );
  size_t ret = it->next( static_cast<size_t>( faceOffsetsBufferLen ),
                         faceOffsetsBuffer,
                         static_cast<size_t>( vertexIndicesBufferLen ),
                         vertexIndicesBuffer );
  return static_cast<int>( ret );
}


void MDAL_FI_close( MeshFaceIteratorH iterator )
{
  if ( iterator )
  {
    MDAL::MeshFaceIterator *it = static_cast< MDAL::MeshFaceIterator * >( iterator );
    delete it;
  }
}


///////////////////////////////////////////////////////////////////////////////////////
/// DATASET GROUPS
///////////////////////////////////////////////////////////////////////////////////////

MeshH MDAL_G_mesh( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDatasetGroup;
    return nullptr;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  MDAL::Mesh *m = g->mesh();
  return static_cast< MeshH >( m );
}

int MDAL_G_datasetCount( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDatasetGroup;
    return 0;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->datasets.size() );
  return len;
}

DatasetH MDAL_G_dataset( DatasetGroupH group, int index )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDatasetGroup;
    return nullptr;
  }

  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDatasetGroup;
    return nullptr;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->datasets.size() );
  if ( len <= index )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDatasetGroup;
    return nullptr;
  }
  size_t i = static_cast<size_t>( index );
  return static_cast< DatasetH >( g->datasets[i].get() );
}

int MDAL_G_metadataCount( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return 0;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->metadata.size() );
  return len;
}

const char *MDAL_G_metadataKey( DatasetGroupH group, int index )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->metadata.size() );
  if ( len <= index )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return EMPTY_STR;
  }
  size_t i = static_cast<size_t>( index );
  return _return_str( g->metadata[i].first );
}

const char *MDAL_G_metadataValue( DatasetGroupH group, int index )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->metadata.size() );
  if ( len <= index )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return EMPTY_STR;
  }
  size_t i = static_cast<size_t>( index );
  return _return_str( g->metadata[i].second );
}

const char *MDAL_G_name( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return _return_str( g->name() );
}

bool MDAL_G_hasScalarData( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return true;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->isScalar();
}

bool MDAL_G_isOnVertices( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return true;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->isOnVertices();
}

void MDAL_G_minimumMaximum( DatasetGroupH group, double *min, double *max )
{
  if ( !min || !max )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return;
  }

  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    *min = NODATA;
    *max = NODATA;
    return;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  MDAL::Statistics stats = g->statistics();
  *min = stats.minimum;
  *max = stats.maximum;
}

DatasetH MDAL_G_addDataset( DatasetGroupH group, double time, const double *values, const int *active )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return nullptr;
  }

  if ( !values )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return nullptr;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  if ( !g->isInEditMode() )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return nullptr;
  }

  const std::string driverName = g->driverName();
  std::shared_ptr<MDAL::Driver> dr = MDAL::DriverManager::instance().driver( driverName );
  if ( !dr )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return nullptr;
  }

  if ( !dr->hasCapability( MDAL::Capability::WriteDatasets ) )
  {
    sLastStatus = MDAL_Status::Err_MissingDriverCapability;
    return nullptr;
  }

  const size_t index = g->datasets.size();
  dr->createDataset( g,
                     time,
                     values,
                     active
                   );
  if ( index < g->datasets.size() ) // we have new dataset
    return static_cast< DatasetGroupH >( g->datasets[ index ].get() );
  else
    return nullptr;
}

bool MDAL_G_isInEditMode( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return true;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->isInEditMode();
}

void MDAL_G_closeEditMode( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );

  if ( !g->isInEditMode() )
  {
    return;
  }

  g->setStatistics( MDAL::calculateStatistics( g ) );
  g->stopEditing();

  const std::string driverName = g->driverName();
  std::shared_ptr<MDAL::Driver> dr = MDAL::DriverManager::instance().driver( driverName );
  if ( !dr )
  {
    sLastStatus = MDAL_Status::Err_MissingDriver;
    return;
  }

  if ( !dr->hasCapability( MDAL::Capability::WriteDatasets ) )
  {
    sLastStatus = MDAL_Status::Err_MissingDriverCapability;
    return;
  }

  bool error = dr->persist( g );
  if ( error )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
  }
}


void MDAL_G_setMetadata( DatasetGroupH group, const char *key, const char *val )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
  }

  if ( !key )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return;
  }

  if ( !val )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return;
  }

  const std::string k( key );
  const std::string v( val );
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  g->setMetadata( k, v );
}

const char *MDAL_G_driverName( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return _return_str( g->driverName() );
}

///////////////////////////////////////////////////////////////////////////////////////
/// DATASETS
///////////////////////////////////////////////////////////////////////////////////////

DatasetGroupH MDAL_D_group( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return nullptr;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return static_cast< MDAL::DatasetGroup * >( d->group() );
}

double MDAL_D_time( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return NODATA;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->time();

}

int MDAL_D_valueCount( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return 0;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->valuesCount() );
  return len;
}

bool MDAL_D_isValid( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return false;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->isValid();
}

int MDAL_D_data( DatasetH dataset, int indexStart, int count, MDAL_DataType dataType, void *buffer )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return 0;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  size_t indexStartSizeT = static_cast<size_t>( indexStart );
  size_t countSizeT = static_cast<size_t>( count );
  MDAL::DatasetGroup *g = d->group();
  assert( g );

  MDAL::Mesh *m = d->mesh();
  assert( m );

  size_t valuesCount = 0;

  // Check that we are requesting correct 1D/2D for given dataset
  switch ( dataType )
  {
    case MDAL_DataType::SCALAR_DOUBLE:
      if ( !g->isScalar() )
      {
        sLastStatus = MDAL_Status::Err_IncompatibleDataset;
        return 0;
      }
      valuesCount = d->valuesCount();
      break;
    case MDAL_DataType::VECTOR_2D_DOUBLE:
      if ( g->isScalar() )
      {
        sLastStatus = MDAL_Status::Err_IncompatibleDataset;
        return 0;
      }
      valuesCount = d->valuesCount();
      break;
    case MDAL_DataType::ACTIVE_INTEGER:
      valuesCount = m->facesCount();
      break;
  }

  // Check that we are not reaching out of values limit
  if ( valuesCount <= indexStartSizeT )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return 0;
  }

  if ( valuesCount < indexStartSizeT + countSizeT )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return 0;
  }

  // Request data
  size_t writtenValuesCount = 0;
  switch ( dataType )
  {
    case MDAL_DataType::SCALAR_DOUBLE:
      writtenValuesCount = d->scalarData( indexStartSizeT, countSizeT, static_cast<double *>( buffer ) );
      break;
    case MDAL_DataType::VECTOR_2D_DOUBLE:
      writtenValuesCount = d->vectorData( indexStartSizeT, countSizeT, static_cast<double *>( buffer ) );
      break;
    case MDAL_DataType::ACTIVE_INTEGER:
      writtenValuesCount = d->activeData( indexStartSizeT, countSizeT, static_cast<int *>( buffer ) );
      break;
  }

  return static_cast<int>( writtenValuesCount );
}

void MDAL_D_minimumMaximum( DatasetH dataset, double *min, double *max )
{
  if ( !min || !max )
  {
    sLastStatus = MDAL_Status::Err_InvalidData;
    return;
  }

  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    *min = NODATA;
    *max = NODATA;
    return;
  }

  MDAL::Dataset *ds = static_cast< MDAL::Dataset * >( dataset );
  MDAL::Statistics stats = ds->statistics();
  *min = stats.minimum;
  *max = stats.maximum;
}
