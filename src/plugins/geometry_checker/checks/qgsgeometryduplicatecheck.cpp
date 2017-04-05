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

      QMap<QString, QList<QgsFeatureId>> duplicates;

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
          QgsAbstractGeometry *diffGeom = geomEngineA->symDifference( *featureGeomB, &errMsg );
          if ( diffGeom && diffGeom->area() < mContext->tolerance )
          {
            duplicates[layerIdB].append( featureIdB );
          }
          else if ( !diffGeom )
          {
            messages.append( tr( "Duplicate check between features %1:%2 and %3:%4 %5" ).arg( layerIdA ).arg( featureA.id() ).arg( layerIdB ).arg( featureB.id() ).arg( errMsg ) );
          }
          delete diffGeom;
          delete featureGeomB;
        }
        if ( !duplicates.isEmpty() )
        {
          errors.append( new QgsGeometryDuplicateCheckError( this, layerIdA, featureIdA, featureA.geometry().geometry()->centroid(), duplicates ) );
        }
        delete geomEngineA;
        delete featureGeomA;
      }

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
    QgsGeometryEngine *geomEngine = QgsGeometryCheckerUtils::createGeomEngine( featureGeomA, mContext->tolerance );

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
    delete geomEngine;
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
