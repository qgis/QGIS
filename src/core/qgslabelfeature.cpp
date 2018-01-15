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

QgsLabelFeature::QgsLabelFeature( QgsFeatureId id, GEOSGeometry *geometry, QSizeF size )
  : mId( id )
  , mGeometry( geometry )
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
  if ( mGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mGeometry );

  if ( mObstacleGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mObstacleGeometry );

  if ( mPermissibleZoneGeosPrepared )
  {
    GEOSPreparedGeom_destroy_r( QgsGeometry::getGEOSHandler(), mPermissibleZoneGeosPrepared );
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mPermissibleZoneGeos );
  }

  delete mInfo;
}

void QgsLabelFeature::setObstacleGeometry( GEOSGeometry *obstacleGeom )
{
  if ( mObstacleGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mObstacleGeometry );

  mObstacleGeometry = obstacleGeom;
}

void QgsLabelFeature::setPermissibleZone( const QgsGeometry &geometry )
{
  mPermissibleZone = geometry;

  if ( mPermissibleZoneGeosPrepared )
  {
    GEOSPreparedGeom_destroy_r( QgsGeometry::getGEOSHandler(), mPermissibleZoneGeosPrepared );
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mPermissibleZoneGeos );
    mPermissibleZoneGeosPrepared = nullptr;
    mPermissibleZoneGeos = nullptr;
  }

  if ( mPermissibleZone.isNull() )
    return;

  mPermissibleZoneGeos = mPermissibleZone.exportToGeos();
  if ( !mPermissibleZoneGeos )
    return;

  mPermissibleZoneGeosPrepared = GEOSPrepare_r( QgsGeometry::getGEOSHandler(), mPermissibleZoneGeos );
}
