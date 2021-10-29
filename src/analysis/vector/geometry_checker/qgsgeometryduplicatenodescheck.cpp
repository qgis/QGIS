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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometryduplicatenodescheck.h"
#include "qgsgeometryutils.h"
#include "qgsfeaturepool.h"
#include "qgsgeometrycheckerror.h"

void QgsGeometryDuplicateNodesCheck::collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids ) const
{
  Q_UNUSED( messages )

  const QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds( featurePools ) : ids.toMap();
  const QgsGeometryCheckerUtils::LayerFeatures layerFeatures( featurePools, featureIds, compatibleGeometryTypes(), feedback, mContext );
  const double sqrTolerance = mContext->tolerance * mContext->tolerance;

  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry().constGet();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        const int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, iPart, iRing );
        if ( nVerts < 2 )
          continue;
        for ( int iVert = nVerts - 1, jVert = 0; jVert < nVerts; iVert = jVert++ )
        {
          const QgsPoint pi = geom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
          const QgsPoint pj = geom->vertexAt( QgsVertexId( iPart, iRing, jVert ) );
          if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < sqrTolerance )
          {
            errors.append( new QgsGeometryCheckError( this, layerFeature, pj, QgsVertexId( iPart, iRing, jVert ) ) );
          }
        }
      }
    }
  }
}

void QgsGeometryDuplicateNodesCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePool = featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->getFeature( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.get();
  const QgsVertexId vidx = error->vidx();

  // Check if point still exists
  if ( !vidx.isValid( geom ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  const int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, vidx.part, vidx.ring );
  const QgsPoint pi = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex + nVerts - 1 ) % nVerts ) );
  const QgsPoint pj = geom->vertexAt( error->vidx() );
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
  static const QStringList methods = QStringList() << tr( "Delete duplicate node" ) << tr( "No action" );
  return methods;
}

QgsGeometryCheck::CheckType QgsGeometryDuplicateNodesCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
