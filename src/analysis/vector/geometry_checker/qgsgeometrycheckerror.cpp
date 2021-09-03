/***************************************************************************
                         qgsgeometrycheckerror.cpp
                         --------
    begin                : September 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckerror.h"
#include "qgsapplication.h"

QgsGeometryCheckError::QgsGeometryCheckError( const QgsGeometryCheck *check,
    const QString &layerId,
    QgsFeatureId featureId,
    const QgsGeometry &geometry,
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
    const QgsPointXY &errorLocation,
    QgsVertexId vidx,
    const QVariant &value,
    ValueType valueType )
  : mCheck( check )
  , mLayerId( layerFeature.layerId() )
  , mFeatureId( layerFeature.feature().id() )
  , mErrorLocation( errorLocation )
  , mVidx( vidx )
  , mValue( value )
  , mValueType( valueType )
  , mStatus( StatusPending )
{
  if ( vidx.part != -1 )
  {
    const QgsGeometry geom = layerFeature.geometry();
    mGeometry = QgsGeometry( QgsGeometryCheckerUtils::getGeomPart( geom.constGet(), vidx.part )->clone() );
  }
  else
  {
    mGeometry = layerFeature.geometry();
  }
  if ( !layerFeature.useMapCrs() )
  {
    QgsVectorLayer *vl = layerFeature.layer().data();
    if ( vl )
    {
      const QgsCoordinateTransform ct( vl->crs(), check->context()->mapCrs, check->context()->transformContext );
      try
      {
        mGeometry.transform( ct );
        mErrorLocation = ct.transform( mErrorLocation );
      }
      catch ( const QgsCsException & )
      {
        QgsDebugMsg( QStringLiteral( "Can not show error in current map coordinate reference system" ) );
      }
    }
  }
}

QgsGeometry QgsGeometryCheckError::geometry() const
{
  return mGeometry;
}

QgsRectangle QgsGeometryCheckError::contextBoundingBox() const
{
  return QgsRectangle();
}

QgsRectangle QgsGeometryCheckError::affectedAreaBBox() const
{
  return mGeometry.boundingBox();
}

void QgsGeometryCheckError::setFixed( int method )
{
  mStatus = StatusFixed;
  const QList<QgsGeometryCheckResolutionMethod> methods = mCheck->availableResolutionMethods();
  for ( const QgsGeometryCheckResolutionMethod &fix : methods )
  {
    if ( fix.id() == method )
      mResolutionMessage = fix.name();
  }
}

void QgsGeometryCheckError::setFixFailed( const QString &reason )
{
  mStatus = StatusFixFailed;
  mResolutionMessage = reason;
}

bool QgsGeometryCheckError::isEqual( QgsGeometryCheckError *other ) const
{
  return other->check() == check() &&
         other->layerId() == layerId() &&
         other->featureId() == featureId() &&
         other->vidx() == vidx();
}

bool QgsGeometryCheckError::closeMatch( QgsGeometryCheckError * ) const
{
  return false;
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

QMap<QString, QgsFeatureIds> QgsGeometryCheckError::involvedFeatures() const
{
  return QMap<QString, QSet<QgsFeatureId> >();
}

QIcon QgsGeometryCheckError::icon() const
{
  if ( status() == QgsGeometryCheckError::StatusFixed )
    return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCheckGeometry.svg" ) );
  else
    return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmLineIntersections.svg" ) );
}

void QgsGeometryCheckError::update( const QgsGeometryCheckError *other )
{
  Q_ASSERT( mCheck == other->mCheck );
  Q_ASSERT( mLayerId == other->mLayerId );
  Q_ASSERT( mFeatureId == other->mFeatureId );
  mErrorLocation = other->mErrorLocation;
  mVidx = other->mVidx;
  mValue = other->mValue;
  mGeometry = other->mGeometry;
}

QgsGeometryCheck::LayerFeatureIds::LayerFeatureIds( const QMap<QString, QgsFeatureIds> &idsIn )
  : ids( idsIn )
{
}
