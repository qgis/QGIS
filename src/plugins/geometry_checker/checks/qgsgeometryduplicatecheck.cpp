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

#include "qgsgeometryengine.h"
#include "qgsgeometryduplicatecheck.h"
#include "qgsspatialindex.h"
#include "qgsgeometry.h"
#include "../utils/qgsfeaturepool.h"


bool QgsGeometryDuplicateCheckError::handleFidChanges( const QString &layerId, const QMap<QgsFeatureId, QgsFeatureId> &oldNewFidMap )
{
  bool changed = QgsGeometryCheckError::handleFidChanges( layerId, oldNewFidMap );
  if ( mDuplicates.contains( layerId ) )
  {
    QList<QgsFeatureId> &fids = mDuplicates[layerId];
    for ( int i = 0, n = fids.size(); i < n; ++i )
    {
      QgsFeatureId oldId = fids[i];
      fids[i] = oldNewFidMap.value( fids[i], fids[i] );
      changed |= ( oldId != fids[i] );
    }
  }
  return changed;
}

QString QgsGeometryDuplicateCheckError::duplicatesString( const QMap<QString, QgsFeaturePool *> &featurePools, const QMap<QString, QList<QgsFeatureId>> &duplicates )
{
  QStringList str;
  for ( const QString &layerId : duplicates.keys() )
  {
    str.append( featurePools[layerId]->getLayer()->name() + ":" );
    QStringList ids;
    for ( QgsFeatureId id : duplicates[layerId] )
    {
      ids.append( QString::number( id ) );
    }
    str.back() += ids.join( "," );
  }
  return str.join( QStringLiteral( "; " ) );
}


void QgsGeometryDuplicateCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeaturesA( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter, true );
  QList<QString> layerIds = featureIds.keys();
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureA : layerFeaturesA )
  {
    // Ensure each pair of layers only gets compared once: remove the current layer from the layerIds, but add it to the layerList for layerFeaturesB
    layerIds.removeOne( layerFeatureA.layer().id() );

    QgsRectangle bboxA = layerFeatureA.geometry()->boundingBox();
    QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry(), mContext->tolerance );
    QMap<QString, QList<QgsFeatureId>> duplicates;

    QgsWkbTypes::GeometryType geomType = layerFeatureA.feature().geometry().type();
    QgsGeometryCheckerUtils::LayerFeatures layerFeaturesB( mContext->featurePools, QList<QString>() << layerFeatureA.layer().id() << layerIds, bboxA, {geomType} );
    for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeatureB : layerFeaturesB )
    {
      // > : only report overlaps within same layer once
      if ( layerFeatureA.layer().id() == layerFeatureB.layer().id() && layerFeatureB.feature().id() >= layerFeatureA.feature().id() )
      {
        continue;
      }
      if ( geomType == QgsWkbTypes::PointGeometry )
      {
        const QgsPoint *p = dynamic_cast<const QgsPoint *>( layerFeatureA.geometry() );
        const QgsPoint *q = dynamic_cast<const QgsPoint *>( layerFeatureB.geometry() );
        if ( p && q && p->distanceSquared( *q ) < mContext->tolerance * mContext->tolerance )
        {
          duplicates[layerFeatureB.layer().id()].append( layerFeatureB.feature().id() );
        }
      }
      else if ( geomType == QgsWkbTypes::PolygonGeometry )
      {
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
    }
    if ( !duplicates.isEmpty() )
    {
      errors.append( new QgsGeometryDuplicateCheckError( this, layerFeatureA, layerFeatureA.geometry()->centroid(), duplicates ) );
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
    QgsWkbTypes::GeometryType geomType = featureA.geometry().type();

    QgsGeometryCheckerUtils::LayerFeature layerFeatureA( featurePoolA, featureA, true );
    QSharedPointer<QgsGeometryEngine> geomEngineA = QgsGeometryCheckerUtils::createGeomEngine( layerFeatureA.geometry(), mContext->tolerance );

    QgsGeometryDuplicateCheckError *duplicateError = static_cast<QgsGeometryDuplicateCheckError *>( error );
    for ( const QString &layerIdB : duplicateError->duplicates().keys() )
    {
      QgsFeaturePool *featurePoolB = mContext->featurePools[ layerIdB ];
      for ( QgsFeatureId idB : duplicateError->duplicates()[layerIdB] )
      {
        QgsFeature featureB;
        if ( !featurePoolB->get( idB, featureB ) )
        {
          continue;
        }
        QgsGeometryCheckerUtils::LayerFeature layerFeatureB( featurePoolB, featureB, true );
        if ( geomType == QgsWkbTypes::PointGeometry )
        {
          const QgsPoint *p = dynamic_cast<const QgsPoint *>( layerFeatureA.geometry() );
          const QgsPoint *q = dynamic_cast<const QgsPoint *>( layerFeatureB.geometry() );
          if ( p && q && p->distanceSquared( *q ) < mContext->tolerance * mContext->tolerance )
          {
            featurePoolB->deleteFeature( featureB );
            changes[layerIdB][idB].append( Change( ChangeFeature, ChangeRemoved ) );
          }
        }
        else if ( geomType == QgsWkbTypes::PolygonGeometry )
        {
          QgsAbstractGeometry *diffGeom = geomEngineA->symDifference( *layerFeatureB.geometry() );
          if ( diffGeom && diffGeom->area() < mContext->tolerance )
          {
            featurePoolB->deleteFeature( featureB );
            changes[layerIdB][idB].append( Change( ChangeFeature, ChangeRemoved ) );
          }

          delete diffGeom;
        }
      }
    }
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
