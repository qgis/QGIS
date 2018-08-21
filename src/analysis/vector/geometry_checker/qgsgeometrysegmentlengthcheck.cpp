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
#include "qgsfeaturepool.h"

void QgsGeometrySegmentLengthCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    double layerToMapUnits = layerFeature.layerToMapUnits();
    double minLength = mMinLengthMapUnits / layerToMapUnits;

    const QgsAbstractGeometry *geom = layerFeature.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        bool isClosed = false;
        int nVerts = QgsGeometryCheckerUtils::polyLineSize( geom, iPart, iRing, &isClosed );
        if ( nVerts < 2 )
        {
          continue;
        }
        for ( int iVert = isClosed ? 0 : 1, jVert = isClosed ? nVerts - 1 : 0; iVert < nVerts; jVert = iVert++ )
        {
          QgsPoint pi = geom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
          QgsPoint pj = geom->vertexAt( QgsVertexId( iPart, iRing, jVert ) );
          double dist = pi.distance( pj );
          // Don't report very small lengths, they are either duplicate nodes or degenerate geometries
          if ( dist < minLength && dist > mContext->tolerance )
          {
            QgsPointXY pos( 0.5 * ( pi.x() + pj.x() ), 0.5 * ( pi.y() + pj.y() ) );
            errors.append( new QgsGeometryCheckError( this, layerFeature, pos, QgsVertexId( iPart, iRing, iVert ), dist * layerToMapUnits, QgsGeometryCheckError::ValueLength ) );
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
  const QgsAbstractGeometry *geom = featureGeom.constGet();
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
  double dist = pi.distance( pj );
  double layerToMapUnits = featurePool->getLayerToMapUnits();
  double minLength = mMinLengthMapUnits / layerToMapUnits;
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

QStringList QgsGeometrySegmentLengthCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
