#include <string>
#include <cassert>
#include <stddef.h>

#include "mdal.h"
#include "mdal_loader.hpp"
#include "mdal_defines.hpp"

static MDAL_Status sLastStatus;

const char *MDAL_Version()
{
  return "0.0.2";
}

MDAL_Status MDAL_LastStatus()
{
  return sLastStatus;
}

MeshH MDAL_LoadMesh( const char *meshFile )
{
  if ( !meshFile )
    return nullptr;

  std::string filename( meshFile );
  return ( MeshH ) MDAL::Loader::load( filename, &sLastStatus );
}


void MDAL_CloseMesh( MeshH mesh )
{
  if ( mesh )
  {
    MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
    delete m;
  }
}


int MDAL_M_vertexCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  int len = static_cast<int>( m->vertices.size() );
  return len;
}

double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( index > -1 );
  size_t i = static_cast<size_t>( index );
  assert( m->vertices.size() > i );
  return m->vertices[i].x;
}

double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( index > -1 );
  size_t i = static_cast<size_t>( index );
  assert( m->vertices.size() > i );
  return m->vertices[i].y;
}

int MDAL_M_faceCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  int len = static_cast<int>( m->faces.size() );
  return len;
}

int MDAL_M_faceVerticesCountAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( index > -1 );
  size_t i = static_cast<size_t>( index );
  assert( m->faces.size() > i );
  int len = static_cast<int>( m->faces[i].size() );
  return len;
}

int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( face_index > -1 );
  size_t fi = static_cast<size_t>( face_index );
  assert( m->faces.size() > fi );
  assert( vertex_index > -1 );
  size_t vi = static_cast<size_t>( vertex_index );
  assert( m->faces[fi].size() > vi );
  int len = static_cast<int>( m->faces[fi][vi] );
  return len;
}
