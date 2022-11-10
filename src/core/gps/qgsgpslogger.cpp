/***************************************************************************
  qgsgpslogger.cpp
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

#include "qgsgpslogger.h"
#include "qgsgpsconnection.h"
#include "qgsvectorlayer.h"

QgsGpsLogger::QgsGpsLogger( QgsGpsConnection *connection, QObject *parent )
  : QObject( parent )
  , mConnection( connection )
{

}

QgsGpsLogger::~QgsGpsLogger()
{

}

QgsGpsConnection *QgsGpsLogger::connection()
{
  return mConnection;
}

void QgsGpsLogger::setPointsLayer( QgsVectorLayer *layer )
{
  mPointsLayer = layer;
}

void QgsGpsLogger::setTracksLayer( QgsVectorLayer *layer )
{
  mTracksLayer = layer;
}

QgsVectorLayer *QgsGpsLogger::pointsLayer()
{
  return mPointsLayer;
}

QgsVectorLayer *QgsGpsLogger::tracksLayer()
{
  return mTracksLayer;
}

QString QgsGpsLogger::pointTimeField() const
{
  return mPointTimeField;
}

void QgsGpsLogger::setPointTimeField( const QString &field )
{
  mPointTimeField = field;
}

QString QgsGpsLogger::pointDistanceFromPreviousField() const
{
  return mPointDistanceFromPreviousField;
}

void QgsGpsLogger::setPointDistanceFromPreviousField( const QString &field )
{
  mPointDistanceFromPreviousField = field;
}

QString QgsGpsLogger::pointTimeDeltaFromPreviousField() const
{
  return mPointTimeDeltaFromPreviousField;
}

void QgsGpsLogger::setPointTimeDeltaFromPreviousField( const QString &field )
{
  mPointTimeDeltaFromPreviousField = field;
}

QString QgsGpsLogger::trackStartTimeField() const
{
  return mTrackStartTimeField;
}

void QgsGpsLogger::setTrackStartTimeField( const QString &field )
{
  mTrackStartTimeField = field;
}

QString QgsGpsLogger::trackEndTimeField() const
{
  return mTrackEndTimeField;
}

void QgsGpsLogger::setTrackEndTimeField( const QString &field )
{
  mTrackEndTimeField = field;
}

QString QgsGpsLogger::trackLengthField() const
{
  return mTrackLengthField;
}

void QgsGpsLogger::setTrackLengthField( const QString &field )
{
  mTrackLengthField = field;
}
