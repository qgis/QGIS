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


QgsVectorLayerGpsLogger::QgsVectorLayerGpsLogger( QgsGpsConnection *connection, QObject *parent )
  : QgsGpsLogger( connection, parent )
{

}

QgsVectorLayerGpsLogger::~QgsVectorLayerGpsLogger()
{

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

