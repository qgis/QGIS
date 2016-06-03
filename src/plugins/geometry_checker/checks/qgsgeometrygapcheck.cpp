/***************************************************************************
    qgsgeometrygapcheck.cpp
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

#include "qgsgeometryengine.h"
#include "qgsgeometrygapcheck.h"
#include "qgsgeometrycollectionv2.h"
#include "../utils/qgsfeaturepool.h"


void QgsGeometryGapCheck::collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter , const QgsFeatureIds &ids ) const
{
  assert( mFeaturePool->getLayer()->geometryType() == QGis::Polygon );
  if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );

  // Collect geometries, build spatial index
  QList<QgsAbstractGeometryV2*> geomList;
  const QgsFeatureIds& featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  Q_FOREACH ( QgsFeatureId id, featureIds )
  {
    QgsFeature feature;
    if ( mFeaturePool->get( id, feature ) )
    {
      geomList.append( feature.geometry()->geometry()->clone() );
    }
  }

  if ( geomList.isEmpty() )
  {
    return;
  }

  QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( nullptr, QgsGeometryCheckPrecision::tolerance() );

  // Create union of geometry
  QString errMsg;
  QgsAbstractGeometryV2* unionGeom = geomEngine->combine( geomList, &errMsg );
  qDeleteAll( geomList );
  delete geomEngine;
  if ( !unionGeom )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    return;
  }

  // Get envelope of union
  geomEngine = QgsGeomUtils::createGeomEngine( unionGeom, QgsGeometryCheckPrecision::tolerance() );
  QgsAbstractGeometryV2* envelope = geomEngine->envelope( &errMsg );
  delete geomEngine;
  if ( !envelope )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    delete unionGeom;
    return;
  }

  // Buffer envelope
  geomEngine = QgsGeomUtils::createGeomEngine( envelope, QgsGeometryCheckPrecision::tolerance() );
  QgsAbstractGeometryV2* bufEnvelope = geomEngine->buffer( 2, 0, GEOSBUF_CAP_SQUARE, GEOSBUF_JOIN_MITRE, 4. );
  delete geomEngine;
  delete envelope;
  envelope = bufEnvelope;

  // Compute difference between envelope and union to obtain gap polygons
  geomEngine = QgsGeomUtils::createGeomEngine( envelope, QgsGeometryCheckPrecision::tolerance() );
  QgsAbstractGeometryV2* diffGeom = geomEngine->difference( *unionGeom, &errMsg );
  delete geomEngine;
  if ( !diffGeom )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    delete unionGeom;
    delete diffGeom;
    return;
  }

  // For each gap polygon which does not lie on the boundary, get neighboring polygons and add error
  for ( int iPart = 0, nParts = diffGeom->partCount(); iPart < nParts; ++iPart )
  {
    QgsAbstractGeometryV2* geom = QgsGeomUtils::getGeomPart( diffGeom, iPart );
    // Skip the gap between features and boundingbox
    if ( geom->boundingBox() == envelope->boundingBox() )
    {
      continue;
    }

    // Skip gaps above threshold
    if ( geom->area() > mThreshold || geom->area() < QgsGeometryCheckPrecision::reducedTolerance() )
    {
      continue;
    }

    // Get neighboring polygons
    QgsFeatureIds neighboringIds;
    QgsRectangle gapAreaBBox = geom->boundingBox();
    QgsFeatureIds intersectIds = mFeaturePool->getIntersects( geom->boundingBox() );

    Q_FOREACH ( QgsFeatureId id, intersectIds )
    {
      QgsFeature feature;
      if ( !mFeaturePool->get( id, feature ) )
      {
        continue;
      }
      QgsAbstractGeometryV2* geom2 = feature.geometry()->geometry();
      if ( QgsGeomUtils::sharedEdgeLength( geom, geom2, QgsGeometryCheckPrecision::reducedTolerance() ) > 0 )
      {
        neighboringIds.insert( feature.id() );
        gapAreaBBox.unionRect( geom2->boundingBox() );
      }
    }
    if ( neighboringIds.isEmpty() )
    {
      delete geom;
      continue;
    }

    // Add error
    errors.append( new QgsGeometryGapCheckError( this, geom->clone(), neighboringIds, geom->area(), gapAreaBBox ) );
  }

  delete unionGeom;
  delete envelope;
  delete diffGeom;
}

void QgsGeometryGapCheck::fixError( QgsGeometryCheckError* error, int method, int /*mergeAttributeIndex*/, Changes &changes ) const
{
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == MergeLongestEdge )
  {
    QString errMsg;
    if ( mergeWithNeighbor( static_cast<QgsGeometryGapCheckError*>( error ), changes, errMsg ) )
    {
      error->setFixed( method );
    }
    else
    {
      error->setFixFailed( tr( "Failed to merge with neighbor: %1" ).arg( errMsg ) );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

bool QgsGeometryGapCheck::mergeWithNeighbor( QgsGeometryGapCheckError* err, Changes &changes, QString& errMsg ) const
{
  double maxVal = 0.;
  QgsFeature mergeFeature;
  int mergePartIdx = -1;

  QgsAbstractGeometryV2* errGeometry = QgsGeomUtils::getGeomPart( err->geometry(), 0 );

  // Search for touching neighboring geometries
  Q_FOREACH ( QgsFeatureId testId, err->neighbors() )
  {
    QgsFeature testFeature;
    if ( !mFeaturePool->get( testId, testFeature ) )
    {
      continue;
    }
    QgsAbstractGeometryV2* testGeom = testFeature.geometry()->geometry();
    for ( int iPart = 0, nParts = testGeom->partCount(); iPart < nParts; ++iPart )
    {
      double len = QgsGeomUtils::sharedEdgeLength( errGeometry, QgsGeomUtils::getGeomPart( testGeom, iPart ), QgsGeometryCheckPrecision::reducedTolerance() );
      if ( len > maxVal )
      {
        maxVal = len;
        mergeFeature = testFeature;
        mergePartIdx = iPart;
      }
    }
  }

  if ( maxVal == 0. )
  {
    return false;
  }

  // Merge geometries
  QgsAbstractGeometryV2* mergeGeom = mergeFeature.geometry()->geometry();
  QgsGeometryEngine* geomEngine = QgsGeomUtils::createGeomEngine( errGeometry, QgsGeometryCheckPrecision::tolerance() );
  QgsAbstractGeometryV2* combinedGeom = geomEngine->combine( *QgsGeomUtils::getGeomPart( mergeGeom, mergePartIdx ), &errMsg );
  delete geomEngine;
  if ( !combinedGeom || combinedGeom->isEmpty() )
  {
    delete combinedGeom;
    return false;
  }

  // Add merged polygon to destination geometry
  replaceFeatureGeometryPart( mergeFeature, mergePartIdx, combinedGeom, changes );

  return true;
}


const QStringList& QgsGeometryGapCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Add gap area to neighboring polygon with longest shared edge" ) << tr( "No action" );
  return methods;
}
