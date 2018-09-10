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

#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"
#include "qgsgeometrycheck.h"
#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"

QgsGeometryCheckerContext::QgsGeometryCheckerContext( int _precision, const QgsCoordinateReferenceSystem &_mapCrs, const QMap<QString, QgsFeaturePool *> &_featurePools )
  : tolerance( std::pow( 10, -_precision ) )
  , reducedTolerance( std::pow( 10, -_precision / 2 ) )
  , mapCrs( _mapCrs )
  , featurePools( _featurePools )
{

}

QgsGeometryCheckError::QgsGeometryCheckError( const QgsGeometryCheck *check, const QString &layerId,
    QgsFeatureId featureId, QgsAbstractGeometry *geometry,
    const QgsPointXY &errorLocation,
    QgsVertexId vidx,
    const QVariant &value, ValueType valueType )
  : mCheck( check )
  , mLayerId( layerId )
  , mFeatureId( featureId )
  , mGeometry( geometry )
  , mErrorLocation( errorLocation )
  , mVidx( vidx )
  , mValue( value )
  , mValueType( valueType )
  , mStatus( StatusPending )
{
}

QgsGeometryCheckError::QgsGeometryCheckError( const QgsGeometryCheck *check,
    const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
    const QgsPointXY &errorLocation, QgsVertexId vidx,
    const QVariant &value, ValueType valueType )
  : mCheck( check )
  , mLayerId( layerFeature.layer()->id() )
  , mFeatureId( layerFeature.feature().id() )
  , mErrorLocation( errorLocation )
  , mVidx( vidx )
  , mValue( value )
  , mValueType( valueType )
  , mStatus( StatusPending )
{
  if ( vidx.part != -1 )
  {
    mGeometry = QgsGeometryCheckerUtils::getGeomPart( layerFeature.geometry(), vidx.part )->clone();
  }
  else
  {
    mGeometry = layerFeature.geometry()->clone();
  }
  if ( layerFeature.geometryCrs() != layerFeature.layerToMapTransform().destinationCrs().authid() )
  {
    mGeometry->transform( layerFeature.layerToMapTransform() );
    mErrorLocation = layerFeature.layerToMapTransform().transform( mErrorLocation );
  }
}

QgsRectangle QgsGeometryCheckError::affectedAreaBBox() const
{
  return mGeometry->boundingBox();
}

bool QgsGeometryCheckError::handleChanges( const QgsGeometryCheck::Changes &changes )
{
  if ( status() == StatusObsolete )
  {
    return false;
  }

  for ( const QgsGeometryCheck::Change &change : changes.value( layerId() ).value( featureId() ) )
  {
    if ( change.what == QgsGeometryCheck::ChangeFeature )
    {
      if ( change.type == QgsGeometryCheck::ChangeRemoved )
      {
        return false;
      }
      else if ( change.type == QgsGeometryCheck::ChangeChanged )
      {
        // If the check is checking the feature at geometry nodes level, the
        // error almost certainly invalid after a geometry change. In the other
        // cases, it might likely still be valid.
        return mCheck->checkType() != QgsGeometryCheck::FeatureNodeCheck;
      }
    }
    else if ( change.what == QgsGeometryCheck::ChangePart )
    {
      if ( mVidx.part == change.vidx.part )
      {
        return false;
      }
      else if ( mVidx.part > change.vidx.part )
      {
        mVidx.part += change.type == QgsGeometryCheck::ChangeAdded ? 1 : -1;
      }
    }
    else if ( change.what == QgsGeometryCheck::ChangeRing )
    {
      if ( mVidx.partEqual( change.vidx ) )
      {
        if ( mVidx.ring == change.vidx.ring )
        {
          return false;
        }
        else if ( mVidx.ring > change.vidx.ring )
        {
          mVidx.ring += change.type == QgsGeometryCheck::ChangeAdded ? 1 : -1;
        }
      }
    }
    else if ( change.what == QgsGeometryCheck::ChangeNode )
    {
      if ( mVidx.ringEqual( change.vidx ) )
      {
        if ( mVidx.vertex == change.vidx.vertex )
        {
          return false;
        }
        else if ( mVidx.vertex > change.vidx.vertex )
        {
          mVidx.vertex += change.type == QgsGeometryCheck::ChangeAdded ? 1 : -1;
        }
      }
    }
  }
  return true;
}

QMap<QString, QgsFeatureIds> QgsGeometryCheck::allLayerFeatureIds() const
{
  QMap<QString, QgsFeatureIds> featureIds;
  for ( QgsFeaturePool *pool : mContext->featurePools )
  {
    featureIds.insert( pool->layerId(), pool->getFeatureIds() );
  }
  return featureIds;
}

void QgsGeometryCheck::replaceFeatureGeometryPart( const QString &layerId, QgsFeature &feature, int partIdx, QgsAbstractGeometry *newPartGeom, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[layerId];
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

void QgsGeometryCheck::deleteFeatureGeometryPart( const QString &layerId, QgsFeature &feature, int partIdx, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[layerId];
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

void QgsGeometryCheck::deleteFeatureGeometryRing( const QString &layerId, QgsFeature &feature, int partIdx, int ringIdx, Changes &changes ) const
{
  QgsFeaturePool *featurePool = mContext->featurePools[layerId];
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *partGeom = QgsGeometryCheckerUtils::getGeomPart( featureGeom.get(), partIdx );
  if ( dynamic_cast<QgsCurvePolygon *>( partGeom ) )
  {
    // If we delete the exterior ring of a polygon, it makes no sense to keep the interiors
    if ( ringIdx == 0 )
    {
      deleteFeatureGeometryPart( layerId, feature, partIdx, changes );
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
    deleteFeatureGeometryPart( layerId, feature, partIdx, changes );
  }
}
