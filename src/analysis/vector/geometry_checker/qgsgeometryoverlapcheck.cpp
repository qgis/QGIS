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
#include "qgsfeaturepool.h"


void QgsGeometryOverlapCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  double overlapThreshold = mThresholdMapUnits;
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  const QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, mContext, true );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    // Ensure each pair of layers only gets compared once: remove the current layer from the layerIds, but add it to the layerList for layerFeaturesB
    layerIds.removeOne( layerFeatureA.layer()->id() );

    QgsRectangle bboxA = layerFeatureA.geometry().constGet()->boundingBox();
    std::unique_ptr< QgsGeometryEngine > geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry().constGet(), mContext->tolerance );
    if ( !geomEngineA->isValid() )
    {
      messages.append( tr( "Overlap check failed for (%1): the geometry is invalid" ).arg( layerFeatureA.id() ) );
      continue;
    }

    const QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( mContext->featurePools, QList<QString>() << layerFeatureA.layer()->id() << layerIds, bboxA, mCompatibleGeometryTypes, mContext );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      // > : only report overlaps within same layer once
      if ( layerFeatureA.layer()->id() == layerFeatureB.layer()->id() && layerFeatureB.feature().id() >= layerFeatureA.feature().id() )
      {
        continue;
      }
      QString errMsg;
      if ( geomEngineA->overlaps( layerFeatureB.geometry().constGet(), &errMsg ) )
      {
        QgsAbstractGeometry *interGeom = geomEngineA->intersection( layerFeatureB.geometry().constGet() );
        if ( interGeom && !interGeom->isEmpty() )
        {
          QgsGeometryCheckerUtils::filter1DTypes( interGeom );
          for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
          {
            QgsAbstractGeometry *interPart = QgsGeometryCheckerUtils::getGeomPart( interGeom, iPart );
            double area = interPart->area();
            if ( area > mContext->reducedTolerance && area < overlapThreshold )
            {
              errors.append( new QgsGeometryOverlapCheckError( this, layerFeatureA, QgsGeometry( interPart->clone() ), interPart->centroid(), area, layerFeatureB ) );
            }
          }
        }
        else if ( !errMsg.isEmpty() )
        {
          messages.append( tr( "Overlap check between features %1 and %2 %3" ).arg( layerFeatureA.id(), layerFeatureB.id(), errMsg ) );
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
  if ( !featurePoolA->getFeature( overlapError->featureId(), featureA ) ||
       !featurePoolB->getFeature( overlapError->overlappedFeature().second, featureB ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  QgsGeometryCheckerUtils::LayerFeature layerFeatureA( featurePoolA, featureA, mContext, true );
  QgsGeometryCheckerUtils::LayerFeature layerFeatureB( featurePoolB, featureB, mContext, true );
  std::unique_ptr< QgsGeometryEngine > geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry().constGet(), mContext->reducedTolerance );

  if ( !geomEngineA->overlaps( layerFeatureB.geometry().constGet() ) )
  {
    error->setObsolete();
    return;
  }
  std::unique_ptr< QgsAbstractGeometry > interGeom( geomEngineA->intersection( layerFeatureB.geometry().constGet(), &errMsg ) );
  if ( !interGeom )
  {
    error->setFixFailed( tr( "Failed to compute intersection between overlapping features: %1" ).arg( errMsg ) );
    return;
  }

  // Search which overlap part this error parametrizes (using fuzzy-matching of the area and centroid...)
  QgsAbstractGeometry *interPart = nullptr;
  for ( int iPart = 0, nParts = interGeom->partCount(); iPart < nParts; ++iPart )
  {
    QgsAbstractGeometry *part = QgsGeometryCheckerUtils::getGeomPart( interGeom.get(), iPart );
    if ( std::fabs( part->area() - overlapError->value().toDouble() ) < mContext->reducedTolerance &&
         QgsGeometryCheckerUtils::pointsFuzzyEqual( part->centroid(), overlapError->location(), mContext->reducedTolerance ) )
    {
      interPart = part;
      break;
    }
  }
  if ( !interPart || interPart->isEmpty() )
  {
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
    std::unique_ptr< QgsAbstractGeometry > diff1( geomEngineA->difference( interPart, &errMsg ) );
    if ( !diff1 || diff1->isEmpty() )
    {
      diff1.reset();
    }
    else
    {
      QgsGeometryCheckerUtils::filter1DTypes( diff1.get() );
    }
    std::unique_ptr< QgsGeometryEngine > geomEngineB = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureB.geometry().constGet(), mContext->reducedTolerance );
    std::unique_ptr< QgsAbstractGeometry > diff2( geomEngineB->difference( interPart, &errMsg ) );
    if ( !diff2 || diff2->isEmpty() )
    {
      diff2.reset();
    }
    else
    {
      QgsGeometryCheckerUtils::filter1DTypes( diff2.get() );
    }
    double shared1 = diff1 ? QgsGeometryCheckerUtils::sharedEdgeLength( diff1.get(), interPart, mContext->reducedTolerance ) : 0;
    double shared2 = diff2 ? QgsGeometryCheckerUtils::sharedEdgeLength( diff2.get(), interPart, mContext->reducedTolerance ) : 0;
    if ( !diff1 || !diff2 || shared1 == 0. || shared2 == 0. )
    {
      error->setFixFailed( tr( "Could not find shared edges between intersection and overlapping features" ) );
    }
    else
    {
      if ( shared1 < shared2 )
      {
        diff1->transform( mContext->layerTransform( featurePoolA->layer() ), QgsCoordinateTransform::ReverseTransform );
        featureA.setGeometry( QgsGeometry( std::move( diff1 ) ) );

        changes[error->layerId()][featureA.id()].append( Change( ChangeFeature, ChangeChanged ) );
        featurePoolA->updateFeature( featureA );
      }
      else
      {
        diff2->transform( mContext->layerTransform( featurePoolB->layer() ), QgsCoordinateTransform::ReverseTransform );
        featureB.setGeometry( QgsGeometry( std::move( diff2 ) ) );

        changes[overlapError->overlappedFeature().first][featureB.id()].append( Change( ChangeFeature, ChangeChanged ) );
        featurePoolB->updateFeature( featureB );
      }

      error->setFixed( method );
    }
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryOverlapCheck::resolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Remove overlapping area from neighboring polygon with shortest shared edge" )
                               << tr( "No action" );
  return methods;
}
