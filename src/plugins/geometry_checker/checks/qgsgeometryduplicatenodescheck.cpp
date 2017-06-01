/***************************************************************************
    qgsgeometryduplicatenodescheck.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryduplicatenodescheck.h"
#include "qgsgeometryutils.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryDuplicateNodesCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds &featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( QgsFeatureId featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }

    QgsGeometry featureGeom = feature.geometry();
    QgsAbstractGeometry *geom = featureGeom.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, iPart, iRing );
        if ( nVerts < 2 )
          continue;
        for ( int iVert = nVerts - 1, jVert = 0; jVert < nVerts; iVert = jVert++ )
        {
          QgsPoint pi = geom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
          QgsPoint pj = geom->vertexAt( QgsVertexId( iPart, iRing, jVert ) );
          if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < QgsGeometryCheckPrecision::tolerance() * QgsGeometryCheckPrecision::tolerance() )
          {
            errors.append( new QgsGeometryCheckError( this, featureid, pj, QgsVertexId( iPart, iRing, jVert ) ) );
          }
        }
      }
    }
  }
}

void QgsGeometryDuplicateNodesCheck::fixError( QgsGeometryCheckError *error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.geometry();
  QgsVertexId vidx = error->vidx();

  // Check if point still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, vidx.part, vidx.ring );
  if ( nVerts == 0 )
  {
    error->setObsolete();
    return;
  }
  QgsPoint pi = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex + nVerts - 1 ) % nVerts ) );
  QgsPoint pj = geom->vertexAt( error->vidx() );
  if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) >= QgsGeometryCheckPrecision::tolerance() * QgsGeometryCheckPrecision::tolerance() )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == RemoveDuplicates )
  {
    if ( !QgsGeometryCheckerUtils::canDeleteVertex( geom, vidx.part, vidx.ring ) )
    {
      error->setFixFailed( tr( "Resulting geometry is degenerate" ) );
    }
    else if ( !geom->deleteVertex( error->vidx() ) )
    {
      error->setFixFailed( tr( "Failed to delete vertex" ) );
    }
    else
    {
      feature.setGeometry( featureGeom );
      mFeaturePool->updateFeature( feature );
      error->setFixed( method );
      changes[error->featureId()].append( Change( ChangeNode, ChangeRemoved, error->vidx() ) );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryDuplicateNodesCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Delete duplicate node" ) << tr( "No action" );
  return methods;
}
