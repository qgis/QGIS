/***************************************************************************
    qgsgeometrycontainedcheck.cpp
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
#include "qgsgeometrycontainedcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryContainedCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( featureIds, mContext->featurePools, mCompatibleGeometryTypes, progressCounter, mContext->mapCrs );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    QgsRectangle bboxA = layerFeatureA.geometry()->boundingBox();
    QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry(), mContext->tolerance );
    QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( featureIds.keys(), bboxA, mContext->mapCrs, mContext->featurePools, mCompatibleGeometryTypes );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      QString errMsg;
      if ( geomEngineA->within( *layerFeatureB.geometry(), &errMsg ) )
      {
        QgsAbstractGeometry *g = layerFeatureA.geometry()->clone();
        QgsPoint pos = g->centroid();
        errors.append( new QgsGeometryContainedCheckError( this, layerFeatureA.layer().id(), layerFeatureA.feature().id(), g, pos, qMakePair( layerFeatureB.layer().id(), layerFeatureB.feature().id() ) ) );
      }
      else if ( !errMsg.isEmpty() )
      {
        messages.append( tr( "Feature %1 within feature %2: %3" ).arg( layerFeatureA.id() ).arg( layerFeatureB.id() ).arg( errMsg ) );
      }
    }
  }
}

void QgsGeometryContainedCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsGeometryContainedCheckError *containerError = static_cast<QgsGeometryContainedCheckError *>( error );
  QgsFeaturePool *featurePoolA = mContext->featurePools[ error->layerId() ];
  QgsFeaturePool *featurePoolB = mContext->featurePools[ containerError->containingFeature().first ];

  QgsFeature featureA;
  QgsFeature featureB;
  if ( !featurePoolA->get( error->featureId(), featureA ) ||
       !featurePoolB->get( containerError->containingFeature().second, featureB ) )
  {
    error->setObsolete();
    return;
  }

  // Check if error still applies
  QgsAbstractGeometry *featureGeomA = featureA.geometry().geometry()->clone();
  featureGeomA->transform( featurePoolA->getMapToLayerTransform(), QgsCoordinateTransform::ReverseTransform );
  QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->tolerance );

  QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
  featureGeomB->transform( featurePoolB->getMapToLayerTransform(), QgsCoordinateTransform::ReverseTransform );

  bool within = geomEngineA->within( *featureGeomB );
  delete featureGeomA;
  delete featureGeomB;

  if ( !within )
  {
    error->setObsolete();
    return;
  }

  // Fix error
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == Delete )
  {
    changes[error->layerId()][featureA.id()].append( Change( ChangeFeature, ChangeRemoved ) );
    featurePoolA->deleteFeature( featureA );
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryContainedCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "Delete feature" )
                               << tr( "No action" );
  return methods;
}
