/***************************************************************************
  qgsmesheditingdelaunaytriangulation.cpp - QgsMeshEditingDelaunayTriangulation

 ---------------------
 begin                : 9.7.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmesheditingdelaunaytriangulation.h"

#include "qgsmeshtriangulation.h"
#include "qgsmesheditor.h"
#include "qgsgeometryengine.h"

QgsMeshEditingDelaunayTriangulation::QgsMeshEditingDelaunayTriangulation(): QgsMeshAdvancedEditing() {}

QgsMeshEditingDelaunayTriangulation::~QgsMeshEditingDelaunayTriangulation() {}

QgsTopologicalMesh::Changes QgsMeshEditingDelaunayTriangulation::apply( QgsMeshEditor *meshEditor )
{
  //use only vertices that are on boundary or free, if boundary
  QList<int> vertexIndextoTriangulate;

  for ( const int vertexIndex : std::as_const( mInputVertices ) )
  {
    if ( meshEditor->isVertexFree( vertexIndex ) || meshEditor->isVertexOnBoundary( vertexIndex ) )
      vertexIndextoTriangulate.append( vertexIndex );
  }

  bool triangulationReady = false;
  bool giveUp = false;
  QgsTopologicalMesh::TopologicalFaces topologicFaces;

  while ( !triangulationReady )
  {
    QgsMeshTriangulation triangulation;

    QVector<int> triangulationVertexToMeshVertex( vertexIndextoTriangulate.count() );
    const QgsMesh destinationMesh = *meshEditor->topologicalMesh()->mesh();

    for ( int i = 0; i < vertexIndextoTriangulate.count(); ++i )
    {
      triangulationVertexToMeshVertex[i] = vertexIndextoTriangulate.at( i );
      triangulation.addVertex( destinationMesh.vertices.at( vertexIndextoTriangulate.at( i ) ) );
    }

    QgsMesh resultingTriangulation = triangulation.triangulatedMesh();

    //Transform the new mesh triangulation to destination mesh faces
    QVector<QgsMeshFace> rawDestinationFaces = resultingTriangulation.faces;

    for ( QgsMeshFace &destinationFace : rawDestinationFaces )
    {
      for ( int &vertexIndex : destinationFace )
        vertexIndex = triangulationVertexToMeshVertex[vertexIndex];
    }

    //The new triangulation may contains faces that intersect exisiting faces, we need to remove them
    QVector<QgsMeshFace> destinationFaces;
    for ( const QgsMeshFace &face : rawDestinationFaces )
    {
      if ( meshEditor->isFaceGeometricallyCompatible( face ) )
        destinationFaces.append( face );
    }

    bool facesReady = false;
    QgsMeshEditingError previousError;
    while ( !facesReady && !giveUp )
    {
      QgsMeshEditingError error;
      topologicFaces = meshEditor->topologicalMesh()->createNewTopologicalFaces( destinationFaces, true, error );

      if ( error == QgsMeshEditingError() )
        error = meshEditor->topologicalMesh()->canFacesBeAdded( topologicFaces );

      switch ( error.errorType )
      {
        case Qgis::MeshEditingErrorType::NoError:
          facesReady = true;
          triangulationReady = true;
          break;
        case Qgis::MeshEditingErrorType::InvalidFace:
        case Qgis::MeshEditingErrorType::FlatFace:
        case Qgis::MeshEditingErrorType::TooManyVerticesInFace:
        case Qgis::MeshEditingErrorType::FacesLinkWithSameClockwise:
          if ( error.elementIndex != -1 )
            destinationFaces.remove( error.elementIndex );
          else
            giveUp = true; //we don't know what happens, better to give up
          break;
        case Qgis::MeshEditingErrorType::InvalidVertex:
        case Qgis::MeshEditingErrorType::UniqueSharedVertex:
          facesReady = true;
          if ( error.elementIndex != -1 )
            vertexIndextoTriangulate.removeOne( error.elementIndex );
          else
            giveUp = true; //we don't know what happens, better to give up
          break;
      }
    }
  }

  mIsFinished = true;

  Q_ASSERT( meshEditor->topologicalMesh()->checkConsistency() == QgsMeshEditingError() );

  if ( triangulationReady && !giveUp )
    return meshEditor->topologicalMesh()->addFaces( topologicFaces );
  else
    return QgsTopologicalMesh::Changes();
}
