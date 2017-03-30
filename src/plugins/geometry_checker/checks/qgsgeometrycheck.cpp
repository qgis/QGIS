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
#include "../utils/qgsfeaturepool.h"

QgsGeometryCheckPrecision::QgsGeometryCheckPrecision()
{
  mPrecision = 10;
  mReducedPrecision = 6;
}

QgsGeometryCheckPrecision *QgsGeometryCheckPrecision::get()
{
  static QgsGeometryCheckPrecision sInstance;
  return &sInstance;
}

void QgsGeometryCheckPrecision::setPrecision( int tolerance )
{
  get()->mPrecision = tolerance;
  get()->mReducedPrecision = tolerance / 2;
}

int QgsGeometryCheckPrecision::precision()
{
  return get()->mPrecision;
}

int QgsGeometryCheckPrecision::reducedPrecision()
{
  return get()->mReducedPrecision;
}

double QgsGeometryCheckPrecision::tolerance()
{
  return std::pow( 10, -get()->mPrecision );
}

double QgsGeometryCheckPrecision::reducedTolerance()
{
  return std::pow( 10, -get()->mReducedPrecision );
}

QgsGeometryCheckError::QgsGeometryCheckError( const QgsGeometryCheck *check, const QString &layerId,
    QgsFeatureId featureId,
    const QgsPoint &errorLocation,
    QgsVertexId vidx,
    const QVariant &value, ValueType valueType )
  : mCheck( check )
  , mLayerId( layerId )
  , mFeatureId( featureId )
  , mErrorLocation( errorLocation )
  , mVidx( vidx )
  , mValue( value )
  , mValueType( valueType )
  , mStatus( StatusPending )
{}

QgsAbstractGeometry *QgsGeometryCheckError::geometry()
{
  QgsFeature f;
  if ( mCheck->getFeaturePool( layerId() )->get( featureId(), f ) && f.hasGeometry() )
  {
    QgsGeometry featureGeom = f.geometry();
    QgsAbstractGeometry *geom = featureGeom.geometry();
    return mVidx.part >= 0 ? QgsGeometryCheckerUtils::getGeomPart( geom, mVidx.part )->clone() : geom->clone();
  }
  return nullptr;
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
        return mCheck->getCheckType() == QgsGeometryCheck::FeatureNodeCheck ? false : true;
      }
    }
    else if ( change.what == QgsGeometryCheck::ChangePart )
    {
      if ( change.type == QgsGeometryCheck::ChangeChanged || mVidx.part == change.vidx.part )
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
        if ( change.type == QgsGeometryCheck::ChangeChanged || mVidx.ring == change.vidx.ring )
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
  for ( QgsFeaturePool *pool : mFeaturePools )
  {
    featureIds.insert( pool->getLayer()->id(), pool->getFeatureIds() );
  }
  return featureIds;
}

void QgsGeometryCheck::replaceFeatureGeometryPart( const QString &layerId, QgsFeature &feature, int partIdx, QgsAbstractGeometry *newPartGeom, Changes &changes ) const
{
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.geometry();
  if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
  {
    QgsGeometryCollection *GeomCollection = static_cast<QgsGeometryCollection *>( geom );
    GeomCollection->removeGeometry( partIdx );
    GeomCollection->addGeometry( newPartGeom );
    changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeRemoved, QgsVertexId( partIdx ) ) );
    changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeAdded, QgsVertexId( GeomCollection->partCount() - 1 ) ) );
    feature.setGeometry( featureGeom );
  }
  else
  {
    feature.setGeometry( QgsGeometry( newPartGeom ) );
    changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeChanged ) );
  }
  mFeaturePools[layerId]->updateFeature( feature );
}

void QgsGeometryCheck::deleteFeatureGeometryPart( const QString &layerId, QgsFeature &feature, int partIdx, Changes &changes ) const
{
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *geom = featureGeom.geometry();
  if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
  {
    static_cast<QgsGeometryCollection *>( geom )->removeGeometry( partIdx );
    if ( static_cast<QgsGeometryCollection *>( geom )->numGeometries() == 0 )
    {
      mFeaturePools[layerId]->deleteFeature( feature );
      changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
    }
    else
    {
      feature.setGeometry( featureGeom );
      mFeaturePools[layerId]->updateFeature( feature );
      changes[layerId][feature.id()].append( Change( ChangePart, ChangeRemoved, QgsVertexId( partIdx ) ) );
    }
  }
  else
  {
    mFeaturePools[layerId]->deleteFeature( feature );
    changes[layerId][feature.id()].append( Change( ChangeFeature, ChangeRemoved ) );
  }
}

void QgsGeometryCheck::deleteFeatureGeometryRing( const QString &layerId, QgsFeature &feature, int partIdx, int ringIdx, Changes &changes ) const
{
  QgsGeometry featureGeom = feature.geometry();
  QgsAbstractGeometry *partGeom = QgsGeometryCheckerUtils::getGeomPart( featureGeom.geometry(), partIdx );
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
      mFeaturePools[layerId]->updateFeature( feature );
      changes[layerId][feature.id()].append( Change( ChangeRing, ChangeRemoved, QgsVertexId( partIdx, ringIdx ) ) );
    }
  }
  // Other geometry types do not have rings, remove the entire part
  else
  {
    deleteFeatureGeometryPart( layerId, feature, partIdx, changes );
  }
}
