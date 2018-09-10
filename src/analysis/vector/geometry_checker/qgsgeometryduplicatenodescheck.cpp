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
#include "qgsfeaturepool.h"

void QgsGeometryDuplicateNodesCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, mContext );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
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
          if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < mContext->tolerance )
          {
            errors.append( new QgsGeometryCheckError( this, layerFeature, pj, QgsVertexId( iPart, iRing, jVert ) ) );
          }
        }
      }
    }
  }
}

void QgsGeometryDuplicateNodesCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->getFeature( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.get();
  QgsVertexId vidx = error->vidx();

  // Check if point still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, vidx.part, vidx.ring );
  QgsPoint pi = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex + nVerts - 1 ) % nVerts ) );
  QgsPoint pj = geom->vertexAt( error->vidx() );
  if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) >= mContext->tolerance )
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
      featurePool->updateFeature( feature );
      error->setFixed( method );
      changes[error->layerId()][error->featureId()].append( Change( ChangeNode, ChangeRemoved, error->vidx() ) );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryDuplicateNodesCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Delete duplicate node" ) << tr( "No action" );
  return methods;
}
