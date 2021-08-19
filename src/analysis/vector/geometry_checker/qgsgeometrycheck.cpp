/***************************************************************************
    qgsgeometrycheck.cpp
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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"
#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"
#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"
#include "qgsreadwritelocker.h"
#include "qgsthreadingutils.h"




QgsGeometryCheck::QgsGeometryCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration )
  : mContext( context )
  , mConfiguration( configuration )
{}

void QgsGeometryCheck::prepare( const QgsGeometryCheckContext *context, const QVariantMap &configuration )
{
  Q_UNUSED( context )
  Q_UNUSED( configuration )
}

bool QgsGeometryCheck::isCompatible( QgsVectorLayer *layer ) const
{
  return compatibleGeometryTypes().contains( layer->geometryType() );
}

QgsGeometryCheck::Flags QgsGeometryCheck::flags() const
{
  return QgsGeometryCheck::Flags();
}

void QgsGeometryCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, QgsGeometryCheck::Changes &changes ) const
{
  Q_UNUSED( featurePools )
  Q_UNUSED( error )
  Q_UNUSED( method )
  Q_UNUSED( mergeAttributeIndices )
  Q_UNUSED( changes )
}

QList<QgsGeometryCheckResolutionMethod> QgsGeometryCheck::availableResolutionMethods() const
{
  QList<QgsGeometryCheckResolutionMethod> fixes;
  // Once the deprecated `resolutionMethods()` function is gone, this default implementation
  // should be removed and each check will need to implement this method instead of resolutionMethods().
  Q_NOWARN_DEPRECATED_PUSH
  const QStringList methods = resolutionMethods();
  Q_NOWARN_DEPRECATED_POP

  const int i = 0;
  for ( const QString &method : methods )
  {
    fixes.append( QgsGeometryCheckResolutionMethod( i, method, QString(), false ) );
  }

  return fixes;
}

QStringList QgsGeometryCheck::resolutionMethods() const
{
  return QStringList();
}

QMap<QString, QgsFeatureIds> QgsGeometryCheck::allLayerFeatureIds( const QMap<QString, QgsFeaturePool *> &featurePools ) const
{
  QMap<QString, QgsFeatureIds> featureIds;
  for ( QgsFeaturePool *pool : featurePools )
  {
    featureIds.insert( pool->layerId(), pool->allFeatureIds() );
  }
  return featureIds;
}

void QgsGeometryCheck::replaceFeatureGeometryPart( const QMap<QString, QgsFeaturePool *> &featurePools,
    const QString &layerId, QgsFeature &feature,
    int partIdx, QgsAbstractGeometry *newPartGeom, Changes &changes ) const
{
  QgsFeaturePool *featurePool = featurePools[layerId];
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.get();
  if ( QgsGeometryCollection *geomCollection = dynamic_cast< QgsGeometryCollection *>( geom ) )
  {
    geomCollection->removeGeometry( partIdx );
    geomCollection->addGeometry( newPartGeom );
    changes[layerId][feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( partIdx ) ) );
    changes[layerId][feature.id()].append( Change( ChangePart, ChangeAdded, QgsVertexId( geomCollection->partCount() - 1 ) ) );
    feature.setGeometry( featureGeom );
  }
  else
  {
    feature.setGeometry( QgsGeometry( newPartGeom ) );
    changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
  }
  featurePool->updateFeature( feature );
}

void QgsGeometryCheck::deleteFeatureGeometryPart( const QMap<QString, QgsFeaturePool *> &featurePools, const QString &layerId, QgsFeature &feature, int partIdx, Changes &changes ) const
{
  QgsFeaturePool *featurePool = featurePools[layerId];
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.get();
  if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
  {
    static_cast<QgsGeometryCollection *>( geom )->removeGeometry( partIdx );
    if ( static_cast<QgsGeometryCollection *>( geom )->numGeometries() == 0 )
    {
      featurePool->deleteFeature( feature.id() );
      changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
    }
    else
    {
      feature.setGeometry( featureGeom );
      featurePool->updateFeature( feature );
      changes[layerId][feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( partIdx ) ) );
    }
  }
  else
  {
    featurePool->deleteFeature( feature.id() );
    changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
  }
}

void QgsGeometryCheck::deleteFeatureGeometryRing( const QMap<QString, QgsFeaturePool *> &featurePools,
    const QString &layerId, QgsFeature &feature,
    int partIdx, int ringIdx, Changes &changes ) const
{
  QgsFeaturePool *featurePool = featurePools[layerId];
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *partGeom = QgsGeometryCheckerUtils::getGeomPart( featureGeom.get(), partIdx );
  if ( dynamic_cast<QgsCurvePolygon *>( partGeom ) )
  {
    // If we delete the exterior ring of a polygon, it makes no sense to keep the interiors
    if ( ringIdx == 0 )
    {
      deleteFeatureGeometryPart( featurePools, layerId, feature, partIdx, changes );
    }
    else
    {
      static_cast<QgsCurvePolygon *>( partGeom )->removeInteriorRing( ringIdx - 1 );
      feature.setGeometry( featureGeom );
      featurePool->updateFeature( feature );
      changes[layerId][feature.id()].append( Change( ChangeRing, ChangeRemoved, QgsVertexId( partIdx, ringIdx ) ) );
    }
  }
  // Other geometry types do not have rings, remove the entire part
  else
  {
    deleteFeatureGeometryPart( featurePools, layerId, feature, partIdx, changes );
  }
}

double QgsGeometryCheck::scaleFactor( const QPointer<QgsVectorLayer> &layer ) const
{
  double scaleFactor = 1.0;

  QgsVectorLayer *lyr = layer.data();
  if ( lyr )
  {
    const QgsCoordinateTransform ct( lyr->crs(), mContext->mapCrs, mContext->transformContext );
    scaleFactor = ct.scaleFactor( lyr->extent() );
  }
  return scaleFactor;
}


