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
#include "qgsgeometrycollection.h"
#include "qgsfeaturepool.h"
#include "geos_c.h"

void QgsGeometryGapCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );

  QVector<QgsAbstractGeometry *> geomList;

  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, nullptr, true );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    geomList.append( layerFeature.geometry()->clone() );
  }

  if ( geomList.isEmpty() )
  {
    return;
  }

  std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( nullptr, mContext->tolerance );

  // Create union of geometry
  QString errMsg;
  QgsAbstractGeometry *unionGeom = geomEngine->combine( geomList, &errMsg );
  qDeleteAll( geomList );
  if ( !unionGeom )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    return;
  }

  // Get envelope of union
  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( unionGeom, mContext->tolerance );
  QgsAbstractGeometry *envelope = geomEngine->envelope( &errMsg );
  if ( !envelope )
  {
    messages.append( tr( "Gap check: %1" ).arg( errMsg ) );
    delete unionGeom;
    return;
  }

  // Buffer envelope
  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( envelope, mContext->tolerance );
  QgsAbstractGeometry *bufEnvelope = geomEngine->buffer( 2, 0, GEOSBUF_CAP_SQUARE, GEOSBUF_JOIN_MITRE, 4. );  //#spellok  //#spellok
  delete envelope;
  envelope = bufEnvelope;

  // Compute difference between envelope and union to obtain gap polygons
  geomEngine = QgsGeometryCheckerUtils::createGeomEngine( envelope, mContext->tolerance );
  QgsAbstractGeometry *diffGeom = geomEngine->difference( unionGeom, &errMsg );
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
    QgsAbstractGeometry *gapGeom = QgsGeometryCheckerUtils::getGeomPart( diffGeom, iPart )->clone();
    // Skip the gap between features and boundingbox
    if ( gapGeom->boundingBox() == envelope->boundingBox() )
    {
      continue;
    }

    // Skip gaps above threshold
    if ( gapGeom->area() > mThresholdMapUnits || gapGeom->area() < mContext->reducedTolerance )
    {
      continue;
    }

    QgsRectangle gapAreaBBox = gapGeom->boundingBox();

    // Get neighboring polygons
    QMap<QString, QgsFeatureIds> neighboringIds;
    QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds.keys(), gapAreaBBox, mCompatibleGeometryTypes );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
    {
      if ( QgsGeometryCheckerUtils::sharedEdgeLength( gapGeom, layerFeature.geometry(), mContext->reducedTolerance ) > 0 )
      {
        neighboringIds[layerFeature.layer().id()].insert( layerFeature.feature().id() );
        gapAreaBBox.combineExtentWith( layerFeature.geometry()->boundingBox() );
      }
    }

    if ( neighboringIds.isEmpty() )
    {
      delete gapGeom;
      continue;
    }

    // Add error
    errors.append( new QgsGeometryGapCheckError( this, "", gapGeom, neighboringIds, gapGeom->area(), gapAreaBBox ) );

  }
  delete unionGeom;
  delete envelope;
  delete diffGeom;
}

void QgsGeometryGapCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == MergeLongestEdge )
  {
    QString errMsg;
    if ( mergeWithNeighbor( static_cast<QgsGeometryGapCheckError *>( error ), changes, errMsg ) )
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

bool QgsGeometryGapCheck::mergeWithNeighbor( QgsGeometryGapCheckError *err, Changes &changes, QString &errMsg ) const
{
  double maxVal = 0.;
  QString mergeLayerId;
  QgsFeature mergeFeature;
  int mergePartIdx = -1;

  const QgsAbstractGeometry *errGeometry = QgsGeometryCheckerUtils::getGeomPart( err->geometry(), 0 );

  // Search for touching neighboring geometries
  for ( const QString &layerId : err->neighbors().keys() )
  {
    QgsFeaturePool *featurePool = mContext->featurePools[ layerId ];
    QgsAbstractGeometry *errLayerGeom = errGeometry->clone();
    errLayerGeom->transform( featurePool->getLayerToMapTransform(), QgsCoordinateTransform::ReverseTransform );

    for ( QgsFeatureId testId : err->neighbors()[layerId] )
    {
      QgsFeature testFeature;
      if ( !featurePool->get( testId, testFeature ) )
      {
        continue;
      }
      QgsGeometry featureGeom = testFeature.geometry();
      const QgsAbstractGeometry *testGeom = featureGeom.constGet();
      for ( int iPart = 0, nParts = testGeom->partCount(); iPart < nParts; ++iPart )
      {
        double len = QgsGeometryCheckerUtils::sharedEdgeLength( errLayerGeom, QgsGeometryCheckerUtils::getGeomPart( testGeom, iPart ), mContext->reducedTolerance );
        if ( len > maxVal )
        {
          maxVal = len;
          mergeFeature = testFeature;
          mergePartIdx = iPart;
          mergeLayerId = layerId;
        }
      }
    }
    delete errLayerGeom;
  }

  if ( maxVal == 0. )
  {
    return false;
  }

  // Merge geometries
  QgsFeaturePool *featurePool = mContext->featurePools[ mergeLayerId ];
  QgsAbstractGeometry *errLayerGeom = errGeometry->clone();
  errLayerGeom->transform( featurePool->getLayerToMapTransform(), QgsCoordinateTransform::ReverseTransform );
  QgsGeometry mergeFeatureGeom = mergeFeature.geometry();
  const QgsAbstractGeometry *mergeGeom = mergeFeatureGeom.constGet();
  std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( errLayerGeom, mContext->reducedTolerance );
  QgsAbstractGeometry *combinedGeom = geomEngine->combine( QgsGeometryCheckerUtils::getGeomPart( mergeGeom, mergePartIdx ), &errMsg );
  delete errLayerGeom;
  if ( !combinedGeom || combinedGeom->isEmpty() || !QgsWkbTypes::isSingleType( combinedGeom->wkbType() ) )
  {
    delete combinedGeom;
    return false;
  }

  // Add merged polygon to destination geometry
  replaceFeatureGeometryPart( mergeLayerId, mergeFeature, mergePartIdx, combinedGeom, changes );

  return true;
}


QStringList QgsGeometryGapCheck::resolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "Add gap area to neighboring polygon with longest shared edge" ) << tr( "No action" );
  return methods;
}
