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

#include "qgsgeometryengine.h"
#include "qgsgeometryoverlapcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryOverlapCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  double overlapThreshold = mThresholdMapUnits;
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( featureIds, mContext->featurePools, mCompatibleGeometryTypes, progressCounter, mContext->mapCrs );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    // Don't check already checked layers
    layerIds.removeOne( layerFeatureA.layer().id() );

    QgsRectangle bboxA = layerFeatureA.geometry()->boundingBox();
    QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry(), mContext->tolerance );

    QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( QList<QString>() << layerFeatureA.layer().id() << layerIds, bboxA, mContext->mapCrs, mContext->featurePools, mCompatibleGeometryTypes );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      // > : only report overlaps within same layer once
      if ( layerFeatureA.layer().id() == layerFeatureB.layer().id() && layerFeatureB.feature().id() >= layerFeatureA.feature().id() )
      {
        continue;
      }
      QString errMsg;
      if ( geomEngineA->overlaps( *layerFeatureB.geometry(), &errMsg ) )
      {
        QgsAbstractGeometry *interGeom = geomEngineA->intersection( *layerFeatureB.geometry() );
        if ( interGeom && !interGeom->isEmpty() )
        {
          QgsGeometryCheckerUtils::filter1DTypes( interGeom );
          for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
          {
            double area = QgsGeometryCheckerUtils::getGeomPart( interGeom, iPart )->area();
            if ( area > mContext->reducedTolerance && area < overlapThreshold )
            {
              errors.append( new QgsGeometryOverlapCheckError( this, layerFeatureA.layer().id(), layerFeatureA.feature().id(), interGeom->clone(), QgsGeometryCheckerUtils::getGeomPart( interGeom, iPart )->centroid(), area, qMakePair( layerFeatureB.layer().id(), layerFeatureB.feature().id() ) ) );
            }
          }
        }
        else if ( !errMsg.isEmpty() )
        {
          messages.append( tr( "Overlap check between features %1 and %2 %3" ).arg( layerFeatureA.id() ).arg( layerFeatureB.id() ).arg( errMsg ) );
        }
        delete interGeom;
      }
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

  // Check if error still applies
  QgsAbstractGeometry *featureGeomA = featureA.geometry().geometry()->clone();
  featureGeomA->transform( featurePoolA->getMapToLayerTransform(), QgsCoordinateTransform::ReverseTransform );
  QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->reducedTolerance );

  QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
  featureGeomB->transform( featurePoolB->getMapToLayerTransform(), QgsCoordinateTransform::ReverseTransform );

  if ( !geomEngineA->overlaps( *featureGeomB ) )
  {
    delete featureGeomA;
    delete featureGeomB;
    error->setObsolete();
    return;
  }
  QgsAbstractGeometry *interGeom = geomEngineA->intersection( *featureGeomB, &errMsg );
  if ( !interGeom )
  {
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
    QSharedPointer<QgsGeometryEngine> geomEngineB = QgsGeometryCheckerUtils::createGeomEngine( featureGeomB, mContext->reducedTolerance );
    QgsAbstractGeometry *diff2 = geomEngineB->difference( *interPart, &errMsg );
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
