/***************************************************************************
    qgsgeometryanglecheck.cpp
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

#include "qgsgeometryanglecheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryAngleCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &/*messages*/, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
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
    const QgsAbstractGeometryV2* geom = feature.geometry()->geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        int nVerts = QgsGeomUtils::polyLineSize( geom, iPart, iRing );
        // Less than three points, no angles to check
        if ( nVerts < 3 )
        {
          continue;
        }
        for ( int iVert = 0; iVert < nVerts; ++iVert )
        {
          const QgsPointV2& p1 = geom->vertexAt( QgsVertexId( iPart, iRing, ( iVert - 1 + nVerts ) % nVerts ) );
          const QgsPointV2& p2 = geom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
          const QgsPointV2& p3 = geom->vertexAt( QgsVertexId( iPart, iRing, ( iVert + 1 ) % nVerts ) );
          QgsVector v21, v23;
          try
          {
            v21 = QgsVector( p1.x() - p2.x(), p1.y() - p2.y() ).normalized();
            v23 = QgsVector( p3.x() - p2.x(), p3.y() - p2.y() ).normalized();
          }
          catch ( const QgsException& )
          {
            // Zero length vectors
            continue;
          }

          double angle = std::acos( v21 * v23 ) / M_PI * 180.0;
          if ( angle < mMinAngle )
          {
            errors.append( new QgsGeometryCheckError( this, featureid, p2, QgsVertexId( iPart, iRing, iVert ), angle ) );
          }
        }
      }
    }
  }
}

void QgsGeometryAngleCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  QgsFeature feature;
  if ( !mFeaturePool->get( error->featureId(), feature ) )
  {
    error->setObsolete();
    return;
  }
  QgsAbstractGeometryV2* geometry = feature.geometry()->geometry();
  QgsVertexId vidx = error->vidx();

  // Check if point still exists
  if ( !vidx.isValid( geometry ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  int n = QgsGeomUtils::polyLineSize( geometry, vidx.part, vidx.ring );
  const QgsPointV2& p1 = geometry->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex - 1 + n ) % n ) );
  const QgsPointV2& p2 = geometry->vertexAt( vidx );
  const QgsPointV2& p3 = geometry->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex + 1 ) % n ) );
  QgsVector v21, v23;
  try
  {
    v21 = QgsVector( p1.x() - p2.x(), p1.y() - p2.y() ).normalized();
    v23 = QgsVector( p3.x() - p2.x(), p3.y() - p2.y() ).normalized();
  }
  catch ( const QgsException& )
  {
    error->setObsolete();
    return;
  }
  double angle = std::acos( v21 * v23 ) / M_PI * 180.0;
  if ( angle >= mMinAngle )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == DeleteNode )
  {

    if ( n <= 3 )
    {
      error->setFixFailed( tr( "Resulting geometry is degenerate" ) );
    }
    else
    {
      geometry->deleteVertex( vidx );
      mFeaturePool->updateFeature( feature );
      error->setFixed( method );
      changes[error->featureId()].append( Change( ChangeNode, ChangeRemoved, vidx ) );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

const QStringList& QgsGeometryAngleCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Delete node with small angle" ) << tr( "No action" );
  return methods;
}
