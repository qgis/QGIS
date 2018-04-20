/***************************************************************************
                         qgstriangularmesh.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstriangularmesh.h"
#include "qgsrendercontext.h"
#include "qgscoordinatetransform.h"

void QgsTriangularMesh::update( QgsMesh *nativeMesh, QgsRenderContext *context )
{
  Q_ASSERT( nativeMesh );
  Q_ASSERT( context );

  mTriangularMesh.vertices.clear();
  mTriangularMesh.faces.clear();
  mTrianglesToNativeFaces.clear();

  // TRANSFORM VERTICES
  QgsCoordinateTransform transform = context->coordinateTransform();
  mTriangularMesh.vertices.resize( nativeMesh->vertices.size() );
  for ( int i = 0; i < nativeMesh->vertices.size(); ++i )
  {
    const QgsMeshVertex &vertex = nativeMesh->vertices.at( i );
    if ( transform.isValid() )
    {
      QgsPointXY mapPoint = transform.transform( QgsPointXY( vertex.x(), vertex.y() ) );
      QgsMeshVertex mapVertex( mapPoint );
      mapVertex.setZ( vertex.z() );
      mapVertex.setM( vertex.m() );
      mTriangularMesh.vertices[i] = mapVertex;
    }
    else
    {
      mTriangularMesh.vertices[i] = vertex;
    }
  }

  // CREATE TRIANGULAR MESH
  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    const QgsMeshFace &face = nativeMesh->faces.at( i ) ;
    if ( face.size() == 3 )
    {
      // triangle
      mTriangularMesh.faces.push_back( face );
      mTrianglesToNativeFaces.push_back( i );
    }
    else if ( face.size() == 4 )
    {
      // quad
      QgsMeshFace face1;
      face1.push_back( face[0] );
      face1.push_back( face[1] );
      face1.push_back( face[2] );

      mTriangularMesh.faces.push_back( face1 );
      mTrianglesToNativeFaces.push_back( i );

      QgsMeshFace face2;
      face2.push_back( face[0] );
      face2.push_back( face[2] );
      face2.push_back( face[3] );

      mTriangularMesh.faces.push_back( face2 );
      mTrianglesToNativeFaces.push_back( i );
    }
  }

}

const QVector<QgsMeshVertex> &QgsTriangularMesh::vertices() const
{
  return mTriangularMesh.vertices;
}

const QVector<QgsMeshFace> &QgsTriangularMesh::triangles() const
{
  return mTriangularMesh.faces;
}

const QVector<int> &QgsTriangularMesh::trianglesToNativeFaces() const
{
  return mTrianglesToNativeFaces;
}

