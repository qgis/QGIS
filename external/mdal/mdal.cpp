/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <stddef.h>
#include <limits>

#include "mdal.h"
#include "mdal_loader.hpp"
#include "mdal_data_model.hpp"

#define NODATA std::numeric_limits<double>::quiet_NaN()
static const char *EMPTY_STR = "";

static MDAL_Status sLastStatus;

const char *MDAL_Version()
{
  return "0.0.7";
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
  return static_cast< MeshH >( MDAL::Loader::load( filename, &sLastStatus ).release() );
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
  return _return_str( m->crs );
}


int MDAL_M_vertexCount( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }

  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->vertices.size() );
  return len;
}

double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  size_t i = static_cast<size_t>( index );
  if ( m->vertices.size() <= i )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  return m->vertices[i].x;
}

double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  size_t i = static_cast<size_t>( index );
  if ( m->vertices.size() <= i )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  return m->vertices[i].y;
}

double MDAL_M_vertexZCoordinatesAt( MeshH mesh, int index )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  size_t i = static_cast<size_t>( index );
  if ( m->vertices.size() <= i )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return NODATA;
  }
  return m->vertices[i].z;
}

int MDAL_M_faceCount( MeshH mesh )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->faces.size() );
  return len;
}

int MDAL_M_faceVerticesCountAt( MeshH mesh, int index )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  if ( index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  size_t i = static_cast<size_t>( index );
  if ( m->faces.size() <= i )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  int len = static_cast<int>( m->faces[i].size() );
  return len;
}

int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index )
{
  if ( !mesh )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  if ( face_index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  size_t fi = static_cast<size_t>( face_index );
  if ( m->faces.size() <= fi )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  if ( vertex_index < 0 )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  size_t vi = static_cast<size_t>( vertex_index );
  if ( m->faces[fi].size() <= vi )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleMesh;
    return 0;
  }
  int len = static_cast<int>( m->faces[fi][vi] );
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
  MDAL::Loader::loadDatasets( m, datasetFile, &sLastStatus );
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

///////////////////////////////////////////////////////////////////////////////////////
/// DATASET GROUPS
///////////////////////////////////////////////////////////////////////////////////////


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
  return g->isScalar;
}

bool MDAL_G_isOnVertices( DatasetGroupH group )
{
  if ( !group )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return true;
  }
  MDAL::DatasetGroup *g = static_cast< MDAL::DatasetGroup * >( group );
  return g->isOnVertices;
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
  return static_cast< MDAL::DatasetGroup * >( d->parent );
}

double MDAL_D_time( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return NODATA;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->time;

}

int MDAL_D_valueCount( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return 0;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->values.size() );
  return len;
}

double MDAL_D_value( DatasetH dataset, int valueIndex )
{
  return MDAL_D_valueX( dataset, valueIndex );
}

double MDAL_D_valueX( DatasetH dataset, int valueIndex )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return NODATA;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->values.size() );
  if ( len <= valueIndex )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return NODATA;
  }
  size_t i = static_cast<size_t>( valueIndex );
  if ( d->values[i].noData )
  {
    return NODATA;
  }
  else
    return d->values[i].x;
}

double MDAL_D_valueY( DatasetH dataset, int valueIndex )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return NODATA;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->values.size() );
  if ( len <= valueIndex )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return NODATA;
  }
  size_t i = static_cast<size_t>( valueIndex );
  if ( d->values[i].noData )
  {
    return NODATA;
  }
  else
    return d->values[i].y;
}

bool MDAL_D_isValid( DatasetH dataset )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return false;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->isValid;
}

bool MDAL_D_active( DatasetH dataset, int faceIndex )
{
  if ( !dataset )
  {
    sLastStatus = MDAL_Status::Err_IncompatibleDataset;
    return false;
  }
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  size_t i = static_cast<size_t>( faceIndex );
  return d->isActive( i );
}
