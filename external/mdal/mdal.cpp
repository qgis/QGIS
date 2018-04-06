#include <string>
#include <cassert>
#include <stddef.h>

#include "mdal.h"
#include "mdal_loader.hpp"
#include "mdal_defines.hpp"

static Status sLastStatus;

const char *MDAL_Version()
{
  return "0.0.1";
}

Status MDAL_LastStatus()
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


size_t MDAL_M_vertexCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  return m->vertices.size();
}

double MDAL_M_vertexXCoordinatesAt( MeshH mesh, size_t index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( m->vertices.size() > index );
  return m->vertices[index].x;
}

double MDAL_M_vertexYCoordinatesAt( MeshH mesh, size_t index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( m->vertices.size() > index );
  return m->vertices[index].y;
}

size_t MDAL_M_faceCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  return m->faces.size();
}

size_t MDAL_M_faceVerticesCountAt( MeshH mesh, size_t index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( m->faces.size() > index );
  return m->faces[index].size();
}

size_t MDAL_M_faceVerticesIndexAt( MeshH mesh, size_t face_index, size_t vertex_index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( m->faces.size() > face_index );
  assert( m->faces[face_index].size() > vertex_index );
  return m->faces[face_index][vertex_index];
}
