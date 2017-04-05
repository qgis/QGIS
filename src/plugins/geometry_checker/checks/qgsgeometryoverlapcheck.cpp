/***************************************************************************
    qgsgeometryoverlapcheck.cpp
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

#include "qgscrscache.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryoverlapcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryOverlapCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  double overlapThreshold = mThresholdMapUnits;
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QList<QString> layerIds = featureIds.keys();
  for ( int i = 0, n = layerIds.length(); i < n; ++i )
  {
    QString layerIdA = layerIds[i];
    QgsFeaturePool *featurePoolA = mContext->featurePools[ layerIdA ];
    if ( !getCompatibility( featurePoolA->getLayer()->geometryType() ) )
    {
      continue;
    }
    QgsCoordinateTransform crstA = QgsCoordinateTransformCache::instance()->transform( featurePoolA->getLayer()->crs().authid(), mContext->mapCrs );

    for ( QgsFeatureId featureIdA : featureIds[layerIdA] )
    {
      if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
      QgsFeature featureA;
      if ( !featurePoolA->get( featureIdA, featureA ) )
      {
        continue;
      }

      QgsAbstractGeometry *featureGeomA = featureA.geometry().geometry()->clone();
      featureGeomA->transform( crstA );
      QgsGeometryEngine *geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->tolerance );
      QgsRectangle bboxA = featureGeomA->boundingBox();

      for ( int j = i; j < n; ++j )
      {
        QString layerIdB = layerIds[j];
        QgsFeaturePool *featurePoolB = mContext->featurePools[ layerIdA ];
        if ( !getCompatibility( featurePoolB->getLayer()->geometryType() ) )
        {
          continue;
        }
        QgsCoordinateTransform crstB = QgsCoordinateTransformCache::instance()->transform( featurePoolB->getLayer()->crs().authid(), mContext->mapCrs );

        QgsFeatureIds idsB = featurePoolB->getIntersects( crstB.transform( bboxA, QgsCoordinateTransform::ReverseTransform ) );
        for ( QgsFeatureId featureIdB : idsB )
        {
          // > : only report overlaps within same layer once
          if ( layerIdA == layerIdB && featureIdB >= featureIdA )
          {
            continue;
          }
          QgsFeature featureB;
          if ( !featurePoolB->get( featureIdB, featureB ) )
          {
            continue;
          }
          QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
          featureGeomB->transform( crstB );
          QString errMsg;
          if ( geomEngineA->overlaps( *featureGeomB, &errMsg ) )
          {
            QgsAbstractGeometry *interGeom = geomEngineA->intersection( *featureGeomB );
            if ( interGeom && !interGeom->isEmpty() )
            {
              QgsGeometryCheckerUtils::filter1DTypes( interGeom );
              for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
              {
                double area = QgsGeometryCheckerUtils::getGeomPart( interGeom, iPart )->area();
                if ( area > mContext->reducedTolerance && area < overlapThreshold )
                {
                  errors.append( new QgsGeometryOverlapCheckError( this, layerIdA, featureIdA, QgsGeometryCheckerUtils::getGeomPart( interGeom, iPart )->centroid(), area, qMakePair( layerIdB, featureIdB ) ) );
                }
              }
            }
            else if ( !errMsg.isEmpty() )
            {
              messages.append( tr( "Overlap check between features %1:%2 and %3:%4 %5" ).arg( layerIdA ).arg( featureIdA ).arg( layerIdB ).arg( featureIdB ).arg( errMsg ) );
            }
            delete interGeom;
          }
          delete featureGeomB;
        }
      }
      delete geomEngineA;
      delete featureGeomA;
    }
  }
}

void QgsGeometryOverlapCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QString errMsg;
  QgsGeometryOverlapCheckError *overlapError = static_cast<QgsGeometryOverlapCheckError *>( error );

  QgsFeaturePool *featurePoolA = mContext->featurePools[ overlapError->layerId() ];
  QgsFeaturePool *featurePoolB = mContext->featurePools[ overlapError->overlappedFeature().first ];
  QgsFeature featureA;
  QgsFeature featureB;
  if ( !featurePoolA->get( overlapError->featureId(), featureA ) ||
       !featurePoolB->get( overlapError->overlappedFeature().second, featureB ) )
  {
    error->setObsolete();
    return;
  }
  QgsCoordinateTransform crstA = QgsCoordinateTransformCache::instance()->transform( featurePoolA->getLayer()->crs().authid(), mContext->mapCrs );
  QgsCoordinateTransform crstB = QgsCoordinateTransformCache::instance()->transform( featurePoolB->getLayer()->crs().authid(), mContext->mapCrs );

  // Check if error still applies
  QgsAbstractGeometry *featureGeomA = featureA.geometry().geometry()->clone();
  featureGeomA->transform( crstA );
  QgsGeometryEngine *geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->reducedTolerance );

  QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
  featureGeomB->transform( crstB );

  if ( !geomEngineA->overlaps( *featureGeomB ) )
  {
    delete geomEngineA;
    delete featureGeomA;
    delete featureGeomB;
    error->setObsolete();
    return;
  }
  QgsAbstractGeometry *interGeom = geomEngineA->intersection( *featureGeomB, &errMsg );
  if ( !interGeom )
  {
    delete geomEngineA;
    delete featureGeomA;
    delete featureGeomB;
    error->setFixFailed( tr( "Failed to compute intersection between overlapping features: %1" ).arg( errMsg ) );
    return;
  }

  // Search which overlap part this error parametrizes (using fuzzy-matching of the area and centroid...)
  QgsAbstractGeometry *interPart = nullptr;
  for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
  {
    QgsAbstractGeometry *part = QgsGeometryCheckerUtils::getGeomPart( interGeom, iPart );
    if ( qAbs( part->area() - overlapError->value().toDouble() ) < mContext->reducedTolerance &&
         QgsGeometryCheckerUtils::pointsFuzzyEqual( part->centroid(), overlapError->location(), mContext->reducedTolerance ) )
    {
      interPart = part;
      break;
    }
  }
  if ( !interPart || interPart->isEmpty() )
  {
    delete interGeom;
    delete geomEngineA;
    delete featureGeomA;
    delete featureGeomB;
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Subtract )
  {
    QgsAbstractGeometry *diff1 = geomEngineA->difference( *interPart, &errMsg );
    if ( !diff1 || diff1->isEmpty() )
    {
      delete diff1;
      diff1 = nullptr;
    }
    else
    {
      QgsGeometryCheckerUtils::filter1DTypes( diff1 );
    }
    QgsGeometryEngine *geomEngineB = QgsGeometryCheckerUtils::createGeomEngine( featureGeomB, mContext->reducedTolerance );
    QgsAbstractGeometry *diff2 = geomEngineB->difference( *interPart, &errMsg );
    delete geomEngineB;
    if ( !diff2 || diff2->isEmpty() )
    {
      delete diff2;
      diff2 = nullptr;
    }
    else
    {
      QgsGeometryCheckerUtils::filter1DTypes( diff2 );
    }
    double shared1 = diff1 ? QgsGeometryCheckerUtils::sharedEdgeLength( diff1, interPart, mContext->reducedTolerance ) : 0;
    double shared2 = diff2 ? QgsGeometryCheckerUtils::sharedEdgeLength( diff2, interPart, mContext->reducedTolerance ) : 0;
    if ( shared1 == 0. || shared2 == 0. )
    {
      error->setFixFailed( tr( "Could not find shared edges between intersection and overlapping features" ) );
    }
    else
    {
      if ( shared1 < shared2 )
      {
        featureA.setGeometry( QgsGeometry( diff1 ) );

        changes[error->layerId()][featureA.id()].append( Change( ChangeFeature, ChangeChanged ) );
        featurePoolA->updateFeature( featureA );

        delete diff2;
      }
      else
      {
        featureB.setGeometry( QgsGeometry( diff2 ) );

        changes[overlapError->overlappedFeature().first][featureB.id()].append( Change( ChangeFeature, ChangeChanged ) );
        featurePoolB->updateFeature( featureB );

        delete diff1;
      }

      error->setFixed( method );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
  delete interGeom;
  delete geomEngineA;
  delete featureGeomA;
  delete featureGeomB;
}

QStringList QgsGeometryOverlapCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Remove overlapping area from neighboring polygon with shortest shared edge" )
                               << tr( "No action" );
  return methods;
}
