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


QgsLabelFeature::QgsLabelFeature( QgsFeatureId id, GEOSGeometry* geometry, QSizeF size )
    : mLayer( nullptr )
    , mId( id )
    , mGeometry( geometry )
    , mObstacleGeometry( nullptr )
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
    , mInfo( nullptr )
{
}

QgsLabelFeature::~QgsLabelFeature()
{
  if ( mGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mGeometry );

  if ( mObstacleGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mObstacleGeometry );

  delete mInfo;
}

void QgsLabelFeature::setObstacleGeometry( GEOSGeometry* obstacleGeom )
{
  if ( mObstacleGeometry )
    GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), mObstacleGeometry );

  mObstacleGeometry = obstacleGeom;
}
