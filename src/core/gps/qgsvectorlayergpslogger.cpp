/***************************************************************************
  qgsvectorlayergpslogger.cpp
   -------------------
  begin                : November 2022
  copyright            : (C) 2022 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayergpslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsgpsconnection.h"
#include "qgslinestring.h"

QgsVectorLayerGpsLogger::QgsVectorLayerGpsLogger( QgsGpsConnection *connection, QObject *parent )
  : QgsGpsLogger( connection, parent )
{
  connect( this, &QgsGpsLogger::stateChanged, this, &QgsVectorLayerGpsLogger::gpsStateChanged );
}

QgsVectorLayerGpsLogger::~QgsVectorLayerGpsLogger()
{
  endCurrentTrack();
}

void QgsVectorLayerGpsLogger::setPointsLayer( QgsVectorLayer *layer )
{
  mPointsLayer = layer;

  if ( mPointsLayer )
  {
    mWgs84toPointLayerTransform = QgsCoordinateTransform( mWgs84CRS, mPointsLayer->crs(), transformContext() );
  }
}

void QgsVectorLayerGpsLogger::setTracksLayer( QgsVectorLayer *layer )
{
  mTracksLayer = layer;

  if ( mTracksLayer )
  {
    mWgs84toTrackLayerTransform = QgsCoordinateTransform( mWgs84CRS, mTracksLayer->crs(), transformContext() );
  }
}

QgsVectorLayer *QgsVectorLayerGpsLogger::pointsLayer()
{
  return mPointsLayer;
}

QgsVectorLayer *QgsVectorLayerGpsLogger::tracksLayer()
{
  return mTracksLayer;
}

QString QgsVectorLayerGpsLogger::pointTimeField() const
{
  return mPointTimeField;
}

void QgsVectorLayerGpsLogger::setPointTimeField( const QString &field )
{
  mPointTimeField = field;
}

QString QgsVectorLayerGpsLogger::pointDistanceFromPreviousField() const
{
  return mPointDistanceFromPreviousField;
}

void QgsVectorLayerGpsLogger::setPointDistanceFromPreviousField( const QString &field )
{
  mPointDistanceFromPreviousField = field;
}

QString QgsVectorLayerGpsLogger::pointTimeDeltaFromPreviousField() const
{
  return mPointTimeDeltaFromPreviousField;
}

void QgsVectorLayerGpsLogger::setPointTimeDeltaFromPreviousField( const QString &field )
{
  mPointTimeDeltaFromPreviousField = field;
}

QString QgsVectorLayerGpsLogger::trackStartTimeField() const
{
  return mTrackStartTimeField;
}

void QgsVectorLayerGpsLogger::setTrackStartTimeField( const QString &field )
{
  mTrackStartTimeField = field;
}

QString QgsVectorLayerGpsLogger::trackEndTimeField() const
{
  return mTrackEndTimeField;
}

void QgsVectorLayerGpsLogger::setTrackEndTimeField( const QString &field )
{
  mTrackEndTimeField = field;
}

QString QgsVectorLayerGpsLogger::trackLengthField() const
{
  return mTrackLengthField;
}

void QgsVectorLayerGpsLogger::setTrackLengthField( const QString &field )
{
  mTrackLengthField = field;
}

void QgsVectorLayerGpsLogger::setTransformContext( const QgsCoordinateTransformContext &context )
{
  QgsGpsLogger::setTransformContext( context );

  if ( mPointsLayer )
  {
    mWgs84toPointLayerTransform = QgsCoordinateTransform( mWgs84CRS, mPointsLayer->crs(), transformContext() );
  }
  if ( mTracksLayer )
  {
    mWgs84toTrackLayerTransform = QgsCoordinateTransform( mWgs84CRS, mTracksLayer->crs(), transformContext() );
  }
}

void QgsVectorLayerGpsLogger::endCurrentTrack()
{
  const QVector< QgsPoint > track = currentTrack();
  if ( track.isEmpty() )
    return;

  if ( mTracksLayer )
  {
    // record track
    const QgsGeometry geometryWgs84 = QgsGeometry( new QgsLineString( track ) );
    QgsGeometry geometry = geometryWgs84;

    try
    {
      geometry.transform( mWgs84toTrackLayerTransform );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Error transforming GPS track" ) );
    }

    QgsAttributeMap attributes;

    const int trackStartFieldIdx = mTracksLayer->fields().lookupField( mTrackStartTimeField );
    if ( trackStartFieldIdx >= 0 )
    {
      attributes.insert( trackStartFieldIdx, timestamp( mTracksLayer, trackStartFieldIdx, trackStartTime() ) );
    }

    const int trackEndFieldIdx = mTracksLayer->fields().lookupField( mTrackEndTimeField );
    if ( trackEndFieldIdx >= 0 )
    {
      attributes.insert( trackEndFieldIdx, timestamp( mTracksLayer, trackStartFieldIdx, lastTimestamp() ) );
    }

    const int trackLengthFieldIdx = mTracksLayer->fields().lookupField( mTrackLengthField );
    if ( trackLengthFieldIdx >= 0 )
    {
      const double length = distanceArea().measureLength( geometryWgs84 );
      attributes.insert( trackLengthFieldIdx, length );
    }

    QgsExpressionContext context = mTracksLayer->createExpressionContext();

    QgsFeature feature = QgsVectorLayerUtils::createFeature( mTracksLayer, geometry, attributes, &context );

    mTracksLayer->addFeature( feature, QgsFeatureSink::Flag::FastInsert );
  }
  resetTrack();
}

void QgsVectorLayerGpsLogger::gpsStateChanged( const QgsGpsInformation &info )
{
  if ( mPointsLayer && info.isValid() )
  {
    // record point
    const QgsPointXY newPosition = lastPosition();
    const QDateTime newTime = lastTimestamp();
    QgsGeometry geometry( new QgsPoint( newPosition.x(), newPosition.y(), lastElevation() ) );

    try
    {
      geometry.transform( mWgs84toPointLayerTransform );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Error transforming GPS point" ) );
    }

    QgsAttributeMap attributes;

    const int pointTimeFieldIdx = mPointsLayer->fields().lookupField( mPointTimeField );
    if ( pointTimeFieldIdx >= 0 )
    {
      attributes.insert( pointTimeFieldIdx, timestamp( mPointsLayer, pointTimeFieldIdx, lastTimestamp() ) );
    }

    const int pointDistanceFromPreviousIdx = mPointsLayer->fields().lookupField( mPointDistanceFromPreviousField );
    if ( pointDistanceFromPreviousIdx >= 0 )
    {
      if ( !mLastPoint.isEmpty() )
      {
        const double distanceSinceLast = distanceArea().measureLine( mLastPoint, newPosition );
        attributes.insert( pointDistanceFromPreviousIdx, distanceSinceLast );
      }
    }
    mLastPoint = newPosition;

    const int pointTimeSincePreviousFieldIdx = mPointsLayer->fields().lookupField( mPointTimeDeltaFromPreviousField );
    if ( pointTimeFieldIdx >= 0 )
    {
      if ( mLastTime.isValid() )
      {
        attributes.insert( pointTimeSincePreviousFieldIdx, static_cast< double >( mLastTime.msecsTo( newTime ) ) / 1000 );
      }
    }
    mLastTime = newTime;

    QgsExpressionContext context = mPointsLayer->createExpressionContext();

    QgsFeature feature = QgsVectorLayerUtils::createFeature( mPointsLayer, geometry, attributes, &context );

    mPointsLayer->addFeature( feature, QgsFeatureSink::Flag::FastInsert );
  }
}

QVariant QgsVectorLayerGpsLogger::timestamp( QgsVectorLayer *vlayer, int idx, const QDateTime &time )
{
  QVariant value;
  if ( idx != -1 && time.isValid() )
  {
    // Only string and datetime fields are supported
    switch ( vlayer->fields().at( idx ).type() )
    {
      case QVariant::String:
        value = time.toString( Qt::DateFormat::ISODate );
        break;
      case QVariant::DateTime:
        value = time;
        break;
      default:
        break;
    }
  }
  return value;
}

