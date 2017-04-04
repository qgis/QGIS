/***************************************************************************
    qgsgeometrysegmentlengthcheck.cpp
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

#include "qgsgeometrysegmentlengthcheck.h"
#include "qgsgeometryutils.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometrySegmentLengthCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  for ( const QString &layerId : featureIds.keys() )
  {
    QgsFeaturePool *featurePool = mContext->featurePools[ layerId ];
    if ( !getCompatibility( featurePool->getLayer()->geometryType() ) )
    {
      continue;
    }
    double mapToLayerUnits = featurePool->getMapToLayerUnits();
    double minLength = mMinLengthMapUnits * mapToLayerUnits;
    for ( QgsFeatureId featureid : featureIds[layerId] )
    {
      if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
      QgsFeature feature;
      if ( !featurePool->get( featureid, feature ) )
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
          {
            continue;
          }
          for ( int iVert = 0, jVert = nVerts - 1; iVert < nVerts; jVert = iVert++ )
          {
            QgsPoint pi = geom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
            QgsPoint pj = geom->vertexAt( QgsVertexId( iPart, iRing, jVert ) );
            double dist = qSqrt( QgsGeometryUtils::sqrDistance2D( pi, pj ) );
            if ( dist < minLength )
            {
              errors.append( new QgsGeometryCheckError( this, layerId, featureid, QgsPoint( 0.5 * ( pi.x() + pj.x() ), 0.5 * ( pi.y() + pj.y() ) ), QgsVertexId( iPart, iRing, iVert ), dist / mapToLayerUnits, QgsGeometryCheckError::ValueLength ) );
            }
          }
        }
      }
    }
  }
}

void QgsGeometrySegmentLengthCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &/*changes*/ ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[ error->layerId() ];
  QgsFeature feature;
  if ( !featurePool->get( error->featureId(), feature ) )
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

  QgsPoint pi = geom->vertexAt( error->vidx() );
  QgsPoint pj = geom->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex - 1 + nVerts ) % nVerts ) );
  double dist = qSqrt( QgsGeometryUtils::sqrDistance2D( pi, pj ) );
  double mapToLayerUnits = featurePool->getMapToLayerUnits();
  double minLength = mMinLengthMapUnits * mapToLayerUnits;
  if ( dist >= minLength )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometrySegmentLengthCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
