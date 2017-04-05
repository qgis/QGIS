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

#include "qgscrscache.h"
#include "qgsgeometryengine.h"
#include "qgsgeometrycontainedcheck.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometryContainedCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  for ( const QString &layerIdA : featureIds.keys() )
  {
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
      QgsRectangle bboxA = crstA.transform( featureGeomA->boundingBox() );

      for ( const QString &layerIdB : featureIds.keys() )
      {
        QgsFeaturePool *featurePoolB = mContext->featurePools[ layerIdB ];
        if ( !getCompatibility( featurePoolB->getLayer()->geometryType() ) )
        {
          continue;
        }
        QgsCoordinateTransform crstB = QgsCoordinateTransformCache::instance()->transform( featurePoolB->getLayer()->crs().authid(), mContext->mapCrs );

        QgsFeatureIds idsB = featurePoolB->getIntersects( crstB.transform( bboxA, QgsCoordinateTransform::ReverseTransform ) );
        for ( QgsFeatureId featureIdB : idsB )
        {
          QgsFeature featureB;
          if ( !featurePoolB->get( featureIdB, featureB ) )
          {
            continue;
          }
          QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
          featureGeomB->transform( crstB );

          QString errMsg;
          if ( geomEngineA->within( *featureGeomB, &errMsg ) )
          {
            errors.append( new QgsGeometryContainedCheckError( this, layerIdA, featureIdA, featureGeomA->centroid(), qMakePair( layerIdB, featureIdB ) ) );
          }
          else if ( !errMsg.isEmpty() )
          {
            messages.append( tr( "Feature %1:%2 within feature %3:%4: %5" ).arg( layerIdA ).arg( featureIdA ).arg( layerIdB ).arg( featureIdB ).arg( errMsg ) );
          }
          delete featureGeomB;
        }
      }
      delete geomEngineA;
      delete featureGeomA;
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
  QgsCoordinateTransform crstA = QgsCoordinateTransformCache::instance()->transform( featurePoolA->getLayer()->crs().authid(), mContext->mapCrs );
  QgsCoordinateTransform crstB = QgsCoordinateTransformCache::instance()->transform( featurePoolB->getLayer()->crs().authid(), mContext->mapCrs );

  // Check if error still applies
  QgsAbstractGeometry *featureGeomA = featureA.geometry().geometry()->clone();
  featureGeomA->transform( crstA );
  QgsGeometryEngine *geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->tolerance );

  QgsAbstractGeometry *featureGeomB = featureB.geometry().geometry()->clone();
  featureGeomB->transform( crstB );

  bool within = geomEngineA->within( *featureGeomB );
  delete geomEngineA;
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
