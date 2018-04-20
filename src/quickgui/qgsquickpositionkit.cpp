/***************************************************************************
  qgsquickpositionkit.cpp
  --------------------------------------
  Date                 : Dec. 2017
  Copyright            : (C) 2017 Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include "qgsquickpositionkit.h"
#include "qgsquicksimulatedpositionsource.h"

QgsQuickPositionKit::QgsQuickPositionKit( QObject *parent )
  : QObject( parent )
  , mAccuracy( -1 )
  , mDirection( -1 )
  , mHasPosition( false )
  , mIsSimulated( false )
{
  use_gps_location();
}

QGeoPositionInfoSource  *QgsQuickPositionKit::gpsSource()
{
  // this should give us "true" position source
  // on Linux it comes from Geoclue library
  QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource( this );
  if ( source->error() != QGeoPositionInfoSource::NoError )
  {
    QgsMessageLog::logMessage( tr( "Unable to create default GPS Position Source" )
                               + "(" + QString::number( ( long )source->error() ) + ")"
                               , "QgsQuick"
                               , Qgis::Warning );
    delete source;
    return 0;
  }
  else
  {
    return source;
  }
}

QGeoPositionInfoSource  *QgsQuickPositionKit::simulatedSource( double longitude, double latitude, double radius )
{
  return new QgsQuickSimulatedPositionSource( this, longitude, latitude, radius );
}

void QgsQuickPositionKit::use_simulated_location( double longitude, double latitude, double radius )
{
  QGeoPositionInfoSource *source = simulatedSource( longitude, latitude, radius );
  mIsSimulated = true;
  replacePositionSource( source );
}

void QgsQuickPositionKit::use_gps_location()
{
  QGeoPositionInfoSource *source = gpsSource();
  mIsSimulated = false;
  replacePositionSource( source );
}

void QgsQuickPositionKit::replacePositionSource( QGeoPositionInfoSource *source )
{
  if ( mSource == source )
    return;

  if ( mSource )
  {
    disconnect( mSource, 0, this, 0 );
    delete mSource;
    mSource = 0;
  }

  mSource = source;

  if ( mSource )
  {
    connect( mSource, SIGNAL( positionUpdated( QGeoPositionInfo ) ),
             this, SLOT( positionUpdated( QGeoPositionInfo ) ) );
    connect( mSource, SIGNAL( updateTimeout() ), this, SLOT( onUpdateTimeout() ) );
    mSource->startUpdates();

    QgsDebugMsg( QStringLiteral( "Position source changed: %1" ).arg( mSource->sourceName() ) );
  }
}

void QgsQuickPositionKit::positionUpdated( const QGeoPositionInfo &info )
{
  if ( !info.coordinate().isValid() )
  {
    // keep last valid position
    mHasPosition = false;
    emit hasPositionChanged();
  }


  mPosition = QgsPoint( info.coordinate().longitude(),
                        info.coordinate().latitude(),
                        info.coordinate().altitude() ); // can be NaN

  if ( info.hasAttribute( QGeoPositionInfo::HorizontalAccuracy ) )
    mAccuracy = info.attribute( QGeoPositionInfo::HorizontalAccuracy );
  else
    mAccuracy = -1;
  if ( info.hasAttribute( QGeoPositionInfo::Direction ) )
    mDirection = info.attribute( QGeoPositionInfo::Direction );
  else
    mDirection = -1;

  emit positionChanged();

  if ( !mHasPosition )
  {
    mHasPosition = true;
    emit hasPositionChanged();
  }
}

void QgsQuickPositionKit::onUpdateTimeout()
{
  if ( mHasPosition )
  {
    mHasPosition = false;
    emit hasPositionChanged();
  }
}

bool QgsQuickPositionKit::hasPosition() const
{
  return mHasPosition;
}

QgsPoint QgsQuickPositionKit::position() const
{
  return mPosition;
}

qreal QgsQuickPositionKit::accuracy() const
{
  return mAccuracy;
}

qreal QgsQuickPositionKit::direction() const
{
  return mDirection;
}

bool QgsQuickPositionKit::simulated() const
{
  return mIsSimulated;
}
