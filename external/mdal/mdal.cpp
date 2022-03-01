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
#include "mdal_logger.hpp"

#define NODATA std::numeric_limits<double>::quiet_NaN()

static const char *EMPTY_STR = "";

const char *MDAL_Version()
{
  return "0.9.90";
}

MDAL_Status MDAL_LastStatus()
{
  return MDAL::Log::getLastStatus();
}

MDAL_EXPORT void MDAL_ResetStatus()
{
  return MDAL::Log::resetLastStatus();
}

MDAL_EXPORT void MDAL_SetStatus( MDAL_LogLevel level, MDAL_Status status, const char *message )
{
  MDAL::Log::resetLastStatus();
  switch ( level )
  {
    case MDAL_LogLevel::Error:
      return MDAL::Log::error( status, message );
    case MDAL_LogLevel::Warn:
      return MDAL::Log::warning( status, message );
    case MDAL_LogLevel::Info:
      return MDAL::Log::info( message );
    case MDAL_LogLevel::Debug:
      return MDAL::Log::debug( message );
  }
}

void MDAL_SetLoggerCallback( MDAL_LoggerCallback callback )
{
  MDAL::Log::setLoggerCallback( callback );
}

void MDAL_SetLogVerbosity( MDAL_LogLevel verbosity )
{
  MDAL::Log::setLogVerbosity( verbosity );
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

MDAL_DriverH MDAL_driverFromIndex( int index )
{
  if ( index < 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "No driver with index: " + std::to_string( index ) );
    return nullptr;
  }

  size_t idx = static_cast<size_t>( index );
  std::shared_ptr<MDAL::Driver> driver = MDAL::DriverManager::instance().driver( idx );
  return static_cast<MDAL_DriverH>( driver.get() );
}

MDAL_DriverH MDAL_driverFromName( const char *name )
{
  std::string nm = name;
  std::shared_ptr<MDAL::Driver> driver = MDAL::DriverManager::instance().driver( nm );
  return static_cast<MDAL_DriverH>( driver.get() );
}

bool MDAL_DR_meshLoadCapability( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return false;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return d->hasCapability( MDAL::Capability::ReadMesh );
}

bool MDAL_DR_writeDatasetsCapability( MDAL_DriverH driver, MDAL_DataLocation location )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return false;
  }


  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return d->hasWriteDatasetCapability( location );
}

bool MDAL_DR_saveMeshCapability( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return false;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return d->hasCapability( MDAL::Capability::SaveMesh );
}

const char *MDAL_DR_longName( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return EMPTY_STR;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->longName() );
}

const char *MDAL_DR_name( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return EMPTY_STR;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->name() );
}

const char *MDAL_DR_filters( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->filters() );
}

int MDAL_DR_faceVerticesMaximumCount( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return -1;
  }
  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return d->faceVerticesMaximumCount();
}

///////////////////////////////////////////////////////////////////////////////////////
/// MESH
///////////////////////////////////////////////////////////////////////////////////////

MDAL_MeshH MDAL_LoadMesh( const char *uri )
{
  if ( !uri )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "Mesh file is not valid (null)" );
    return nullptr;
  }

  std::string uriString( uri ), driverName, meshFile, meshName;

  MDAL::parseDriverAndMeshFromUri( uriString, driverName, meshFile, meshName );

  if ( !driverName.empty() )
  {
    return static_cast< MDAL_MeshH >( MDAL::DriverManager::instance().load( driverName, meshFile, meshName ).release() );
  }
  else
    return static_cast< MDAL_MeshH >( MDAL::DriverManager::instance().load( meshFile, meshName ).release() );
}

const char *MDAL_MeshNames( const char *uri )
{
  if ( !uri )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "Mesh file is not valid (null)" );
    return nullptr;
  }

  std::string uriString( uri ), driver, file, uris;

  MDAL::parseDriverFromUri( uriString, driver );
  MDAL::parseMeshFileFromUri( uriString, file );

  uris = MDAL::DriverManager::instance().getUris( file, driver );

  return _return_str( uris );
}

void MDAL_SaveMesh( MDAL_MeshH mesh, const char *meshFile, const char *driver )
{
  MDAL::Log::resetLastStatus();
  if ( !meshFile )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "Mesh file is not valid (null)" );
    return;
  }

  std::string driverName( driver );
  auto d = MDAL::DriverManager::instance().driver( driver );

  if ( !d )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "No driver with name: " + driverName );
    return;
  }

  if ( !d->hasCapability( MDAL::Capability::SaveMesh ) )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, "Driver " + driverName + " does not have SaveMesh capability" );
    return;
  }

  if ( d->faceVerticesMaximumCount() < MDAL_M_faceVerticesMaximumCount( mesh ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is incompatible with driver " + driverName );
    return;
  }

  std::string filename( meshFile );

  std::string uri = MDAL::buildMeshUri( filename, "", driverName );

  MDAL::DriverManager::instance().save( static_cast< MDAL::Mesh * >( mesh ), uri );
}


void MDAL_SaveMeshWithUri( MDAL_MeshH mesh, const char *uri )
{
  MDAL::Log::resetLastStatus();

  std::string meshFile;
  std::string driverName;
  std::string meshName;

  MDAL::parseDriverAndMeshFromUri( uri, driverName, meshFile, meshName );

  if ( meshFile.empty() )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "Mesh file is not valid (null)" );
    return;
  }

  auto d = MDAL::DriverManager::instance().driver( driverName );

  if ( !d )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "No driver with name: " + driverName );
    return;
  }

  if ( !d->hasCapability( MDAL::Capability::SaveMesh ) )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, "Driver " + driverName + " does not have SaveMesh capability" );
    return;
  }

  if ( d->faceVerticesMaximumCount() < MDAL_M_faceVerticesMaximumCount( mesh ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is incompatible with driver " + driverName );
    return;
  }

  MDAL::DriverManager::instance().save( static_cast< MDAL::Mesh * >( mesh ), uri );
}

void MDAL_CloseMesh( MDAL_MeshH mesh )
{
  if ( mesh )
  {
    MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
    delete m;
  }
}

const char *MDAL_M_projection( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return EMPTY_STR;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  return _return_str( m->crs() );
}

void MDAL_M_extent( MDAL_MeshH mesh, double *minX, double *maxX, double *minY, double *maxY )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
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

int MDAL_M_vertexCount( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return 0;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->verticesCount() );
  return len;
}


int MDAL_M_edgeCount( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return 0;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->edgesCount() );
  return len;
}

int MDAL_M_faceCount( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->facesCount() );
  return len;
}

int MDAL_M_faceVerticesMaximumCount( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->faceVerticesMaximumCount() );
  return len;
}

void MDAL_M_LoadDatasets( MDAL_MeshH mesh, const char *datasetFile )
{
  if ( !datasetFile )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, "Dataset file is not valid (null)" );
    return;
  }

  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );

  std::string filename( datasetFile );
  MDAL::DriverManager::instance().loadDatasets( m, datasetFile );
}

int MDAL_M_metadataCount( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->metadata.size() );
  return len;
}

const char *MDAL_M_metadataKey( MDAL_MeshH mesh, int index )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->metadata.size() );
  if ( len <= index )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Requested index: " + std::to_string( index ) + " is out of scope for metadata" );
    return EMPTY_STR;
  }
  size_t i = static_cast<size_t>( index );
  return _return_str( m->metadata[i].first );
}

const char *MDAL_M_metadataValue( MDAL_MeshH mesh, int index )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh,  "Mesh is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->metadata.size() );
  if ( len <= index )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Requested index: " + std::to_string( index ) + " is out of scope for metadata" );
    return EMPTY_STR;
  }
  size_t i = static_cast<size_t>( index );
  return _return_str( m->metadata[i].second );
}

void MDAL_M_setMetadata( MDAL_MeshH mesh, const char *key, const char *val )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh,  "Mesh is not valid (null)" );
    return;
  }

  if ( !key )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointer key is not valid (null)" );
    return;
  }

  if ( !val )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointer val is not valid (null)" );
    return;
  }

  const std::string k( key );
  const std::string v( val );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  m->setMetadata( k, v );
}

int MDAL_M_datasetGroupCount( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->datasetGroups.size() );
  return len;
}

MDAL_DatasetGroupH MDAL_M_datasetGroup( MDAL_MeshH mesh, int index )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return nullptr;
  }

  if ( index < 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Requested index is not valid: " + std::to_string( index ) );
    return nullptr;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->datasetGroups.size() );
  if ( len <= index )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Requested index " + std::to_string( index ) + " is bigger than datasets count" );
    return nullptr;
  }
  size_t i = static_cast<size_t>( index );

  return static_cast< MDAL_DatasetH >( m->datasetGroups[i].get() );
}

MDAL_DatasetGroupH MDAL_M_addDatasetGroup(
  MDAL_MeshH mesh,
  const char *name,
  MDAL_DataLocation dataLocation,
  bool hasScalarData,
  MDAL_DriverH driver,
  const char *datasetGroupFile )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return nullptr;
  }

  if ( !name )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Name is not valid (null)" );
    return nullptr;
  }

  if ( !datasetGroupFile )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Dataset group file is not valid (null)" );
    return nullptr;
  }

  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return nullptr;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  MDAL::Driver *dr = static_cast< MDAL::Driver * >( driver );

  if ( !dr->hasWriteDatasetCapability( dataLocation ) )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, dr->name(), "does not have Write Dataset capability" );
    return nullptr;
  }

  const size_t index = m->datasetGroups.size();
  dr->createDatasetGroup( m,
                          name,
                          dataLocation,
                          hasScalarData,
                          datasetGroupFile
                        );
  if ( index < m->datasetGroups.size() ) // we have new dataset group
    return static_cast< MDAL_DatasetGroupH >( m->datasetGroups[ index ].get() );
  else
    return nullptr;
}

const char *MDAL_M_driverName( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return nullptr;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  return _return_str( m->driverName() );
}

///////////////////////////////////////////////////////////////////////////////////////
/// MESH VERTICES
///////////////////////////////////////////////////////////////////////////////////////

MDAL_MeshVertexIteratorH MDAL_M_vertexIterator( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return nullptr;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  std::unique_ptr<MDAL::MeshVertexIterator> it = m->readVertices();
  return static_cast< MDAL_MeshVertexIteratorH >( it.release() );
}

int MDAL_VI_next( MDAL_MeshVertexIteratorH iterator, int verticesCount, double *coordinates )
{
  if ( verticesCount < 1 )
    return 0;

  if ( !iterator )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh Vertex Iterator is not valid (null)" );
    return 0;
  }
  if ( !coordinates )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Coordinates pointer is not valid (null)" );
    return 0;
  }
  MDAL::MeshVertexIterator *it = static_cast< MDAL::MeshVertexIterator * >( iterator );
  size_t size = static_cast<size_t>( verticesCount );
  size_t ret = it->next( size, coordinates );
  return static_cast<int>( ret );
}

void MDAL_VI_close( MDAL_MeshVertexIteratorH iterator )
{
  if ( iterator )
  {
    MDAL::MeshVertexIterator *it = static_cast< MDAL::MeshVertexIterator * >( iterator );
    delete it;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
/// MESH EDGES
///////////////////////////////////////////////////////////////////////////////////////

MDAL_MeshEdgeIteratorH MDAL_M_edgeIterator( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return nullptr;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  std::unique_ptr<MDAL::MeshEdgeIterator> it = m->readEdges();
  return static_cast< MDAL_MeshEdgeIteratorH >( it.release() );
}

int MDAL_EI_next( MDAL_MeshEdgeIteratorH iterator, int edgesCount, int *startVertexIndices, int *endVertexIndices )
{
  if ( edgesCount < 1 )
    return 0;

  if ( !iterator )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh Edge Iterator is not valid (null)" );
    return 0;
  }

  if ( !startVertexIndices || !endVertexIndices )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Start or End Vertex Index is not valid (null)" );
    return 0;
  }

  MDAL::MeshEdgeIterator *it = static_cast< MDAL::MeshEdgeIterator * >( iterator );
  size_t size = static_cast<size_t>( edgesCount );
  size_t ret = it->next( size, startVertexIndices, endVertexIndices );
  return static_cast<int>( ret );
}

void MDAL_EI_close( MDAL_MeshEdgeIteratorH iterator )
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

MDAL_MeshFaceIteratorH MDAL_M_faceIterator( MDAL_MeshH mesh )
{
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return nullptr;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  std::unique_ptr<MDAL::MeshFaceIterator > it = m->readFaces();
  return static_cast< MDAL_MeshFaceIteratorH >( it.release() );
}

int MDAL_FI_next( MDAL_MeshFaceIteratorH iterator,
                  int faceOffsetsBufferLen,
                  int *faceOffsetsBuffer,
                  int vertexIndicesBufferLen,
                  int *vertexIndicesBuffer )
{
  if ( ( faceOffsetsBufferLen < 1 ) || ( vertexIndicesBufferLen < 1 ) )
    return 0;

  if ( !iterator )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh Face Iterator is not valid (null)" );
    return 0;
  }
  MDAL::MeshFaceIterator *it = static_cast< MDAL::MeshFaceIterator * >( iterator );
  size_t ret = it->next( static_cast<size_t>( faceOffsetsBufferLen ),
                         faceOffsetsBuffer,
                         static_cast<size_t>( vertexIndicesBufferLen ),
                         vertexIndicesBuffer );
  return static_cast<int>( ret );
}


void MDAL_FI_close( MDAL_MeshFaceIteratorH iterator )
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

MDAL_MeshH MDAL_G_mesh( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDatasetGroup, "Dataset group is not valid (null)" );
    return nullptr;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  MDAL::Mesh *m = g->mesh();
  return static_cast< MDAL_MeshH >( m );
}

int MDAL_G_datasetCount( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDatasetGroup, "Dataset group is not valid (null)" );
    return 0;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->datasets.size() );
  return len;
}

MDAL_DatasetH MDAL_G_dataset( MDAL_DatasetGroupH group, int index )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDatasetGroup, "Dataset group is not valid (null)" );
    return nullptr;
  }

  if ( index < 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDatasetGroup, "Requested index: " + std::to_string( index ) + " is out of scope for dataset groups" );
    return nullptr;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->datasets.size() );
  if ( len <= index )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDatasetGroup, "Requested index: " + std::to_string( index ) + " is out of scope for dataset groups" );
    return nullptr;
  }
  size_t i = static_cast<size_t>( index );
  return static_cast< MDAL_DatasetH >( g->datasets[i].get() );
}

int MDAL_G_metadataCount( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return 0;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->metadata.size() );
  return len;
}

const char *MDAL_G_metadataKey( MDAL_DatasetGroupH group, int index )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->metadata.size() );
  if ( len <= index )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Requested index: " + std::to_string( index ) + " is out of scope for dataset groups" );
    return EMPTY_STR;
  }
  size_t i = static_cast<size_t>( index );
  return _return_str( g->metadata[i].first );
}

const char *MDAL_G_metadataValue( MDAL_DatasetGroupH group, int index )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->metadata.size() );
  if ( len <= index )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Requested index: " + std::to_string( index ) + " is out of scope for metadata" );
    return EMPTY_STR;
  }
  size_t i = static_cast<size_t>( index );
  return _return_str( g->metadata[i].second );
}

const char *MDAL_G_name( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return _return_str( g->name() );
}

bool MDAL_G_hasScalarData( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return true;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->isScalar();
}

MDAL_DataLocation MDAL_G_dataLocation( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return DataInvalidLocation;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->dataLocation();
}

int MDAL_G_maximumVerticalLevelCount( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return 0;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  int len = static_cast<int>( g->maximumVerticalLevelsCount() );
  return len;
}

void MDAL_G_minimumMaximum( MDAL_DatasetGroupH group, double *min, double *max )
{
  if ( !min || !max )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointers min or max are not valid (null)" );
    return;
  }

  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    *min = NODATA;
    *max = NODATA;
    return;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  MDAL::Statistics stats = g->statistics();
  *min = stats.minimum;
  *max = stats.maximum;
}

MDAL_DatasetH MDAL_G_addDataset( MDAL_DatasetGroupH group, double time, const double *values, const int *active )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return nullptr;
  }

  if ( !values )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointer Values is not valid" );
    return nullptr;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  if ( !g->isInEditMode() )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not in edit mode" );
    return nullptr;
  }

  const std::string driverName = g->driverName();
  std::shared_ptr<MDAL::Driver> dr = MDAL::DriverManager::instance().driver( driverName );
  if ( !dr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver name " + driverName + " saved in dataset group could not be found" );
    return nullptr;
  }

  if ( !dr->hasWriteDatasetCapability( g->dataLocation() ) )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, "Driver " + driverName + " does not have Write Dataset capability" );
    return nullptr;
  }

  if ( g->dataLocation() == MDAL_DataLocation::DataOnVolumes )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, "Cannot save 3D dataset as a 2D dataset" );
    return nullptr;
  }

  if ( active && g->dataLocation() != MDAL_DataLocation::DataOnVertices )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Active flag is only supported on datasets with data on vertices" );
    return nullptr;
  }

  const size_t index = g->datasets.size();
  MDAL::RelativeTimestamp t( time, MDAL::RelativeTimestamp::hours );
  dr->createDataset( g,
                     t,
                     values,
                     active
                   );
  if ( index < g->datasets.size() ) // we have new dataset
    return static_cast< MDAL_DatasetGroupH >( g->datasets[ index ].get() );
  else
    return nullptr;
}

MDAL_DatasetH MDAL_G_addDataset3D( MDAL_DatasetGroupH group, double time, const double *values, const int *verticalLevelCount, const double *verticalExtrusions )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return nullptr;
  }

  if ( !values || !verticalLevelCount || !verticalExtrusions )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointer Values are not valid" );
    return nullptr;
  }

  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  if ( !g->isInEditMode() )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not in edit mode" );
    return nullptr;
  }

  const std::string driverName = g->driverName();
  std::shared_ptr<MDAL::Driver> dr = MDAL::DriverManager::instance().driver( driverName );
  if ( !dr )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver name " + driverName + " saved in dataset group could not be found" );
    return nullptr;
  }

  if ( g->dataLocation() != MDAL_DataLocation::DataOnVolumes )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, "Cannot write 3D data to a Dataset Group that does not have Data On Volumes" );
    return nullptr;
  }

  const size_t index = g->datasets.size();
  MDAL::RelativeTimestamp t( time, MDAL::RelativeTimestamp::hours );
  dr->createDataset( g,
                     t,
                     values,
                     verticalLevelCount,
                     verticalExtrusions
                   );
  if ( index < g->datasets.size() ) // we have new dataset
    return static_cast< MDAL_DatasetGroupH >( g->datasets[ index ].get() );
  else
    return nullptr;
}

bool MDAL_G_isInEditMode( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return true;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->isInEditMode();
}

void MDAL_G_closeEditMode( MDAL_DatasetGroupH group )
{
  MDAL::Log::resetLastStatus();
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
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
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver name " + driverName + " saved in dataset group could not be found" );
    return;
  }

  if ( !dr->hasWriteDatasetCapability( g->dataLocation() ) )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriverCapability, "Driver " + driverName + " does not have Write Dataset capability" );
    return;
  }

  bool error = dr->persist( g );
  if ( error )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Persist error occurred in driver" );
  }
}

const char *MDAL_G_referenceTime( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return _return_str( g->referenceTime().toStandardCalendarISO8601() );
}

void MDAL_G_setReferenceTime( MDAL_DatasetGroupH group, const char *referenceTimeISO8601 )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  const std::string datetime( referenceTimeISO8601 );
  g->setReferenceTime( MDAL::DateTime( datetime ) );
}

void MDAL_G_setMetadata( MDAL_DatasetGroupH group, const char *key, const char *val )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
  }

  if ( !key )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointer key is not valid (null)" );
    return;
  }

  if ( !val )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointer val is not valid (null)" );
    return;
  }

  const std::string k( key );
  const std::string v( val );
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  g->setMetadata( k, v );
}

const char *MDAL_G_driverName( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return _return_str( g->driverName() );
}

///////////////////////////////////////////////////////////////////////////////////////
/// DATASETS
///////////////////////////////////////////////////////////////////////////////////////

MDAL_DatasetGroupH MDAL_D_group( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return nullptr;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return static_cast< MDAL::DatasetGroup * >( d->group() );
}

double MDAL_D_time( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return NODATA;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->time( MDAL::RelativeTimestamp::hours );
}

int MDAL_D_volumesCount( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return 0;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->volumesCount() );
  return len;
}

int MDAL_D_maximumVerticalLevelCount( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return 0;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->maximumVerticalLevelsCount() );
  return len;
}

int MDAL_D_valueCount( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return 0;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->valuesCount() );
  return len;
}

bool MDAL_D_isValid( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return false;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->isValid();
}

int MDAL_D_data( MDAL_DatasetH dataset, int indexStart, int count, MDAL_DataType dataType, void *buffer )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
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
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not scalar" );
        return 0;
      }
      if ( ( g->dataLocation() != MDAL_DataLocation::DataOnVertices ) && ( g->dataLocation() != MDAL_DataLocation::DataOnFaces ) && ( g->dataLocation() != MDAL_DataLocation::DataOnEdges ) )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Scalar access only supported on datasets with data on vertices or faces" );
        return 0;
      }
      valuesCount = d->valuesCount();
      break;
    case MDAL_DataType::VECTOR_2D_DOUBLE:
      if ( g->isScalar() )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is scalar" );
        return 0;
      }
      if ( ( g->dataLocation() != MDAL_DataLocation::DataOnVertices ) && ( g->dataLocation() != MDAL_DataLocation::DataOnFaces ) && ( g->dataLocation() != MDAL_DataLocation::DataOnEdges ) )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Vector access only supported on datasets with data on vertices or faces" );
        return 0;
      }
      valuesCount = d->valuesCount();
      break;
    case MDAL_DataType::ACTIVE_INTEGER:
      if ( !d->supportsActiveFlag() )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group does not support Active Flag" );
        return 0;
      }
      valuesCount = m->facesCount();
      break;
    case MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER:
      if ( g->dataLocation() != MDAL_DataLocation::DataOnVolumes )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group does not have data on volumes in 3D" );
        return 0;
      }
      valuesCount = m->facesCount();
      break;
    case MDAL_DataType::VERTICAL_LEVEL_DOUBLE:
      if ( g->dataLocation() != MDAL_DataLocation::DataOnVolumes )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group does not have data on volumes in 3D" );
        return 0;
      }
      valuesCount = m->facesCount() + d->volumesCount();
      break;
    case MDAL_DataType::FACE_INDEX_TO_VOLUME_INDEX_INTEGER:
      if ( g->dataLocation() != MDAL_DataLocation::DataOnVolumes )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group does not have data on volumes in 3D" );
        return 0;
      }
      valuesCount = m->facesCount();
      break;
    case MDAL_DataType::SCALAR_VOLUMES_DOUBLE:
      if ( g->dataLocation() != MDAL_DataLocation::DataOnVolumes )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group does not have data on volumes in 3D" );
        return 0;
      }
      if ( !g->isScalar() )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not scalar" );
        return 0;
      }
      valuesCount = d->volumesCount();
      break;
    case MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE:
      if ( g->dataLocation() != MDAL_DataLocation::DataOnVolumes )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group does not have data on volumes in 3D" );
        return 0;
      }
      if ( g->isScalar() )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is scalar" );
        return 0;
      }
      valuesCount = 2 * d->volumesCount();
      break;
  }

  // Check that we are not reaching out of values limit
  if ( valuesCount <= indexStartSizeT )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Reached out of values limit" );
    return 0;
  }

  if ( valuesCount < indexStartSizeT + countSizeT )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Reached out of values limit" );
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
    case MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER:
      writtenValuesCount = d->verticalLevelCountData( indexStartSizeT, countSizeT, static_cast<int *>( buffer ) );
      break;
    case MDAL_DataType::VERTICAL_LEVEL_DOUBLE:
      writtenValuesCount = d->verticalLevelData( indexStartSizeT, countSizeT, static_cast<double *>( buffer ) );
      break;
    case MDAL_DataType::FACE_INDEX_TO_VOLUME_INDEX_INTEGER:
      writtenValuesCount = d->faceToVolumeData( indexStartSizeT, countSizeT, static_cast<int *>( buffer ) );
      break;
    case MDAL_DataType::SCALAR_VOLUMES_DOUBLE:
      writtenValuesCount = d->scalarVolumesData( indexStartSizeT, countSizeT, static_cast<double *>( buffer ) );
      break;
    case MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE:
      writtenValuesCount = d->vectorVolumesData( indexStartSizeT, countSizeT, static_cast<double *>( buffer ) );
      break;
  }

  return static_cast<int>( writtenValuesCount );
}

void MDAL_D_minimumMaximum( MDAL_DatasetH dataset, double *min, double *max )
{
  if ( !min || !max )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, "Passed pointers min or max are not valid (null)" );
    return;
  }

  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    *min = NODATA;
    *max = NODATA;
    return;
  }

  MDAL::Dataset *ds = static_cast< MDAL::Dataset * >( dataset );
  MDAL::Statistics stats = ds->statistics();
  *min = stats.minimum;
  *max = stats.maximum;
}

bool MDAL_D_hasActiveFlagCapability( MDAL_DatasetH dataset )
{
  if ( !dataset )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset is not valid (null)" );
    return false;
  }

  MDAL::Dataset *ds = static_cast< MDAL::Dataset * >( dataset );
  return ds->supportsActiveFlag();
}

bool MDAL_G_isTemporal( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return false;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->datasets.size() > 1;
}

const char *MDAL_G_uri( MDAL_DatasetGroupH group )
{
  if ( !group )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleDataset, "Dataset Group is not valid (null)" );
    return EMPTY_STR;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return _return_str( g->uri() );
}
const char *MDAL_DR_writeDatasetsSuffix( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return EMPTY_STR;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->writeDatasetOnFileSuffix() );
}

const char *MDAL_DR_saveMeshSuffix( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return EMPTY_STR;
  }

  MDAL::Driver *d = static_cast< MDAL::Driver * >( driver );
  return _return_str( d->saveMeshOnFileSuffix() );
}

MDAL_MeshH MDAL_CreateMesh( MDAL_DriverH driver )
{
  if ( !driver )
  {
    MDAL::Log::error( MDAL_Status::Err_MissingDriver, "Driver is not valid (null)" );
    return nullptr;
  }

  MDAL::Driver *d =  static_cast<MDAL::Driver *>( driver );
  return new MDAL::MemoryMesh( d->name(),
                               0, // empty mesh, so faceVerticesMaximumCount=0, this attribute will be updated if faces are added
                               "" );
}

void MDAL_M_addVertices( MDAL_MeshH mesh, int vertexCount, double *coordinates )
{
  MDAL::Log::resetLastStatus();
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return;
  }

  MDAL::Mesh *m = static_cast<MDAL::Mesh *>( mesh );

  if ( ! m->isEditable() )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not editable" );
  }

  m->datasetGroups.clear();
  m->addVertices( vertexCount, coordinates );
}

void MDAL_M_addFaces( MDAL_MeshH mesh, int faceCount, int *faceSizes, int *vertexIndices )
{
  MDAL::Log::resetLastStatus();
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return;
  }

  MDAL::Mesh *m = static_cast<MDAL::Mesh *>( mesh );

  if ( ! m->isEditable() )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not editable" );
  }

  m->datasetGroups.clear();
  std::shared_ptr<MDAL::Driver> driver = MDAL::DriverManager::instance().driver( m->driverName() );
  int maxVerticesPerFace = std::numeric_limits<int>::max();
  if ( driver )
    maxVerticesPerFace = driver->faceVerticesMaximumCount();

  m->addFaces( faceCount, maxVerticesPerFace, faceSizes, vertexIndices );
}

void MDAL_M_setProjection( MDAL_MeshH mesh, const char *projection )
{
  MDAL::Log::resetLastStatus();
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return;
  }

  static_cast<MDAL::Mesh *>( mesh )->setSourceCrsFromWKT( std::string( projection ) );
}

void MDAL_M_addEdges( MDAL_MeshH mesh,
                      int edgesCount,
                      int *startVertexIndices,
                      int *endVertexIndices )
{
  MDAL::Log::resetLastStatus();
  if ( !mesh )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not valid (null)" );
    return;
  }

  MDAL::Mesh *m = static_cast<MDAL::Mesh *>( mesh );

  if ( ! m->isEditable() )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, "Mesh is not editable" );
  }

  m->datasetGroups.clear();
  std::shared_ptr<MDAL::Driver> driver = MDAL::DriverManager::instance().driver( m->driverName() );

  m->addEdges( edgesCount, startVertexIndices, endVertexIndices );
}
