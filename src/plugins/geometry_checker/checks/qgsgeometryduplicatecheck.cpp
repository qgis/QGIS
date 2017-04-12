/***************************************************************************
    qgsgeometryduplicatecheck.cpp
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
#include "qgsgeometryduplicatecheck.h"
#include "qgsspatialindex.h"
#include "qgsgeometry.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryDuplicateCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( featureIds, mContext->featurePools, mCompatibleGeometryTypes, progressCounter, mContext->mapCrs );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    // Don't check already checked layers
    layerIds.removeOne( layerFeatureA.layer().id() );

    QgsRectangle bboxA = layerFeatureA.geometry()->boundingBox();
    QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry(), mContext->tolerance );
    QMap<QString, QList<QgsFeatureId>> duplicates;

    QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( QList<QString>() << layerFeatureA.layer().id() << layerIds, bboxA, mContext->mapCrs, mContext->featurePools, mCompatibleGeometryTypes );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      // > : only report overlaps within same layer once
      if ( layerFeatureA.layer().id() == layerFeatureB.layer().id() && layerFeatureB.feature().id() >= layerFeatureA.feature().id() )
      {
        continue;
      }
      QString errMsg;
      QgsAbstractGeometry *diffGeom = geomEngineA->symDifference( *layerFeatureB.geometry(), &errMsg );
      if ( diffGeom && diffGeom->area() < mContext->tolerance )
      {
        duplicates[layerFeatureB.layer().id()].append( layerFeatureB.feature().id() );
      }
      else if ( !diffGeom )
      {
        messages.append( tr( "Duplicate check between features %1 and %2: %3" ).arg( layerFeatureA.id() ).arg( layerFeatureB.id() ).arg( errMsg ) );
      }
      delete diffGeom;
    }
    if ( !duplicates.isEmpty() )
    {
      errors.append( new QgsGeometryDuplicateCheckError( this, layerFeatureA.layer().id(), layerFeatureA.feature().id(), layerFeatureA.geometry()->clone(), layerFeatureA.geometry()->centroid(), duplicates ) );
    }
  }
}

void QgsGeometryDuplicateCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes &changes ) const
{
  QgsFeaturePool *featurePoolA = mContext->featurePools[ error->layerId() ];
  QgsFeature featureA;
  if ( !featurePoolA->get( error->featureId(), featureA ) )
  {
    error->setObsolete();
    return;
  }

  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else if ( method == RemoveDuplicates )
  {
    QgsCoordinateTransform crstA = QgsCoordinateTransformCache::instance()->transform( featurePoolA->getLayer()->crs().authid(), mContext->mapCrs );
    QgsAbstractGeometry *featureGeomA = featureA.geometry().geometry()->clone();
    featureGeomA->transform( crstA );
    QSharedPointer<QgsGeometryEngine> geomEngine = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->tolerance );

    QgsGeometryDuplicateCheckError *duplicateError = static_cast<QgsGeometryDuplicateCheckError *>( error );
    for ( const QString &layerIdB : duplicateError->duplicates().keys() )
    {
      QgsFeaturePool *featurePoolB = mContext->featurePools[ layerIdB ];
      QgsCoordinateTransform crstB = QgsCoordinateTransformCache::instance()->transform( featurePoolB->getLayer()->crs().authid(), mContext->mapCrs );
      for ( QgsFeatureId idB : duplicateError->duplicates()[layerIdB] )
      {
        QgsFeature featureB;
        if ( !featurePoolB->get( idB, featureB ) )
        {
          continue;
        }
        QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
        featureGeomB->transform( crstB );
        QgsAbstractGeometry *diffGeom = geomEngine->symDifference( *featureGeomB );
        if ( diffGeom && diffGeom->area() < mContext->tolerance )
        {
          featurePoolB->deleteFeature( featureB );
          changes[layerIdB][idB].append( Change( ChangeFeature, ChangeRemoved ) );
        }

        delete diffGeom;
        delete featureGeomB;
      }
    }
    delete featureGeomA;
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometryDuplicateCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList()
                               << tr( "No action" )
                               << tr( "Remove duplicates" );
  return methods;
}
