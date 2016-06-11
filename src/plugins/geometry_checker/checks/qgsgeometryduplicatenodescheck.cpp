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

void QgsGeometryDuplicateNodesCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds& featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( QgsFeatureId featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }

    QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        int nVerts = QgsGeomUtils::polyLineSize( geom, iPart, iRing );
        if ( nVerts < 2 )
          continue;
        for ( int iVert = nVerts - 1, jVert = 0; jVert < nVerts; iVert = jVert++ )
        {
          QgsPointV2 pi = geom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
          QgsPointV2 pj = geom->vertexAt( QgsVertexId( iPart, iRing, jVert ) );
          if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < QgsGeometryCheckPrecision::tolerance() * QgsGeometryCheckPrecision::tolerance() )
          {
            errors.append( new QgsGeometryCheckError( this, featureid, pj, QgsVertexId( iPart, iRing, jVert ) ) );
          }
        }
      }
    }
  }
}

void QgsGeometryDuplicateNodesCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
  QgsVertexId vidx = error->vidx();

  // Check if point still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  int nVerts = QgsGeomUtils::polyLineSize( geom, vidx.part, vidx.ring );
  QgsPointV2 pi = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex + nVerts - 1 ) % nVerts ) );
  QgsPointV2 pj = geom->vertexAt( error->vidx() );
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
    geom->deleteVertex( error->vidx() );
    if ( QgsGeomUtils::polyLineSize( geom, vidx.part, vidx.ring ) < 3 )
    {
      error->setFixFailed( tr( "Resulting geometry is degenerate" ) );
    }
    else
    {
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

const QStringList& QgsGeometryDuplicateNodesCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Delete duplicate node" ) << tr( "No action" );
  return methods;
}
