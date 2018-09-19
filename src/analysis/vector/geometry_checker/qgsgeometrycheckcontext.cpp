/***************************************************************************
    qgsgeometrycheckcontext.h
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckcontext.h"
#include "qgsreadwritelocker.h"
#include "qgsthreadingutils.h"
#include "qgsvectorlayer.h"

QgsGeometryCheckContext::QgsGeometryCheckContext( int _precision, const QgsCoordinateReferenceSystem &_mapCrs, const QMap<QString, QgsFeaturePool *> &_featurePools, const QgsCoordinateTransformContext &transformContext )
  : tolerance( std::pow( 10, -_precision ) )
  , reducedTolerance( std::pow( 10, -_precision / 2 ) )
  , mapCrs( _mapCrs )
  , featurePools( _featurePools )
  , transformContext( transformContext )
{
}

const QgsCoordinateTransform &QgsGeometryCheckContext::layerTransform( const QPointer<QgsVectorLayer> &layer )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  if ( !mTransformCache.contains( layer ) )
  {
    QgsCoordinateTransform transform;
    QgsThreadingUtils::runOnMainThread( [this, &transform, layer]()
    {
      QgsVectorLayer *lyr = layer.data();
      if ( lyr )
        transform = QgsCoordinateTransform( lyr->crs(), mapCrs, transformContext );
    } );
    locker.changeMode( QgsReadWriteLocker::Write );
    mTransformCache[layer] = transform;
    locker.changeMode( QgsReadWriteLocker::Read );
  }

  return mTransformCache[layer];
}

double QgsGeometryCheckContext::layerScaleFactor( const QPointer<QgsVectorLayer> &layer )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  if ( !mScaleFactorCache.contains( layer ) )
  {
    double scaleFactor = 1.0;
    QgsThreadingUtils::runOnMainThread( [this, layer, &scaleFactor]()
    {
      QgsVectorLayer *lyr = layer.data();
      if ( lyr )
        scaleFactor = layerTransform( layer ).scaleFactor( lyr->extent() );
    } );

    locker.changeMode( QgsReadWriteLocker::Write );
    mScaleFactorCache[layer] = scaleFactor;
    locker.changeMode( QgsReadWriteLocker::Read );
  }

  return mScaleFactorCache.value( layer );
}
