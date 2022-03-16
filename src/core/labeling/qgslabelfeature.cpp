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
{
}

QgsLabelFeature::~QgsLabelFeature()
{
  if ( mPermissibleZoneGeosPrepared )
  {
    mPermissibleZoneGeosPrepared.reset();
    mPermissibleZoneGeos.reset();
  }
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

QgsFeature QgsLabelFeature::feature() const
{
  return mFeature;
}

QSizeF QgsLabelFeature::size( double angle ) const
{
  if ( mRotatedSize.isEmpty() )
    return mSize;

  // Between 45 to 135 and 235 to 315 degrees, return the rotated size
  return ( angle >= 0.785398 && angle <= 2.35619 ) || ( angle >= 3.92699 && angle <= 5.49779 ) ? mRotatedSize : mSize;
}

QgsPointXY QgsLabelFeature::anchorPosition() const
{
  return mAnchorPosition;
}

void QgsLabelFeature::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

double QgsLabelFeature::overrunDistance() const
{
  return mOverrunDistance;
}

void QgsLabelFeature::setOverrunDistance( double overrunDistance )
{
  mOverrunDistance = overrunDistance;
}

double QgsLabelFeature::overrunSmoothDistance() const
{
  return mOverrunSmoothDistance;
}

void QgsLabelFeature::setOverrunSmoothDistance( double overrunSmoothDistance )
{
  mOverrunSmoothDistance = overrunSmoothDistance;
}

QgsLabelLineSettings::AnchorTextPoint QgsLabelFeature::lineAnchorTextPoint() const
{
  if ( mAnchorTextPoint == QgsLabelLineSettings::AnchorTextPoint::FollowPlacement )
  {
    if ( mLineAnchorPercent < 0.25 )
      return QgsLabelLineSettings::AnchorTextPoint::StartOfText;
    else if ( mLineAnchorPercent > 0.75 )
      return QgsLabelLineSettings::AnchorTextPoint::EndOfText;
    else
      return QgsLabelLineSettings::AnchorTextPoint::CenterOfText;
  }
  else
  {
    return mAnchorTextPoint;
  }
}

const QgsLabelObstacleSettings &QgsLabelFeature::obstacleSettings() const
{
  return mObstacleSettings;
}

void QgsLabelFeature::setObstacleSettings( const QgsLabelObstacleSettings &settings )
{
  mObstacleSettings = settings;
}

QgsCoordinateReferenceSystem QgsLabelFeature::originalFeatureCrs() const
{
  return mOriginalFeatureCrs;
}

void QgsLabelFeature::setOriginalFeatureCrs( const QgsCoordinateReferenceSystem &originalFeatureCrs )
{
  mOriginalFeatureCrs = originalFeatureCrs;
}

void QgsLabelFeature::setAnchorPosition( const QgsPointXY &anchorPosition )
{
  mAnchorPosition = anchorPosition;
}
