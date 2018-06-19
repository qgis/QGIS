/***************************************************************************
    qgslabelfeature.cpp
    ---------------------
    begin                : December 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslabelfeature.h"
#include "feature.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"

QgsLabelFeature::QgsLabelFeature( QgsFeatureId id, geos::unique_ptr geometry, QSizeF size )
  : mId( id )
  , mGeometry( std::move( geometry ) )
  , mSize( size )
  , mPriority( -1 )
  , mZIndex( 0 )
  , mHasFixedPosition( false )
  , mHasFixedAngle( false )
  , mFixedAngle( 0 )
  , mHasFixedQuadrant( false )
  , mDistLabel( 0 )
  , mOffsetType( QgsPalLayerSettings::FromPoint )
  , mRepeatDistance( 0 )
  , mAlwaysShow( false )
  , mIsObstacle( false )
  , mObstacleFactor( 1 )
{
}

QgsLabelFeature::~QgsLabelFeature()
{
  if ( mPermissibleZoneGeosPrepared )
  {
    mPermissibleZoneGeosPrepared.reset();
    mPermissibleZoneGeos.reset();
  }

  delete mInfo;
}

void QgsLabelFeature::setObstacleGeometry( geos::unique_ptr obstacleGeom )
{
  mObstacleGeometry = std::move( obstacleGeom );
}

void QgsLabelFeature::setPermissibleZone( const QgsGeometry &geometry )
{
  mPermissibleZone = geometry;

  if ( mPermissibleZoneGeosPrepared )
  {
    mPermissibleZoneGeosPrepared.reset();
    mPermissibleZoneGeos.reset();
    mPermissibleZoneGeosPrepared = nullptr;
  }

  if ( mPermissibleZone.isNull() )
    return;

  mPermissibleZoneGeos = QgsGeos::asGeos( mPermissibleZone );
  if ( !mPermissibleZoneGeos )
    return;

  mPermissibleZoneGeosPrepared.reset( GEOSPrepare_r( QgsGeos::getGEOSHandler(), mPermissibleZoneGeos.get() ) );
}
