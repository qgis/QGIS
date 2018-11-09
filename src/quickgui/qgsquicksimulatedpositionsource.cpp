/***************************************************************************
 qgsquicksimulatedpositionsource.cpp
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

#include "qgsquicksimulatedpositionsource.h"

#include <QTimer>

/// @cond PRIVATE

QgsQuickSimulatedPositionSource::QgsQuickSimulatedPositionSource( QObject *parent, double longitude, double latitude, double flightRadius )
  : QGeoPositionInfoSource( parent )
  , mTimer( qgis::make_unique< QTimer >() )
  , mFlightRadius( flightRadius )
  , mLongitude( longitude )
  , mLatitude( latitude )
{
  connect( mTimer.get(), &QTimer::timeout, this, &QgsQuickSimulatedPositionSource::readNextPosition );
}

void QgsQuickSimulatedPositionSource::startUpdates()
{
  int interval = updateInterval();
  if ( interval < minimumUpdateInterval() )
    interval = minimumUpdateInterval();

  mTimer->start( interval );
  readNextPosition();
}

void QgsQuickSimulatedPositionSource::stopUpdates()
{
  mTimer->stop();
}

void QgsQuickSimulatedPositionSource::requestUpdate( int /*timeout*/ )
{
  readNextPosition();
}



void QgsQuickSimulatedPositionSource::readNextPosition()
{
  if ( mFlightRadius <= 0 )
    readConstantPosition();
  else
    readRandomPosition();
}

void QgsQuickSimulatedPositionSource::readRandomPosition()
{
  double latitude = mLatitude, longitude = mLongitude;
  latitude += sin( mAngle * M_PI / 180 ) * mFlightRadius;
  longitude += cos( mAngle * M_PI / 180 ) * mFlightRadius;
  mAngle += 1;

  QGeoCoordinate coordinate( latitude, longitude );
  double altitude = std::rand() % 40 + 20; // rand altitude <20,55>m and lost (0)
  if ( altitude <= 55 )
  {
    coordinate.setAltitude( altitude ); // 3D
  }

  QDateTime timestamp = QDateTime::currentDateTime();

  QGeoPositionInfo info( coordinate, timestamp );
  if ( info.isValid() )
  {
    mLastPosition = info;
    info.setAttribute( QGeoPositionInfo::Direction, 360 - int( mAngle ) % 360 );
    int accuracy = std::rand() % 40 + 20; // rand accuracy <20,55>m and lost (-1)
    if ( accuracy > 55 )
    {
      accuracy = -1;
    }
    info.setAttribute( QGeoPositionInfo::HorizontalAccuracy, accuracy );
    emit positionUpdated( info );
  }
}

void QgsQuickSimulatedPositionSource::readConstantPosition()
{
  QGeoCoordinate coordinate( mLatitude, mLongitude );
  coordinate.setAltitude( 20 );
  QDateTime timestamp = QDateTime::currentDateTime();
  QGeoPositionInfo info( coordinate, timestamp );
  info.setAttribute( QGeoPositionInfo::Direction, 0 );
  info.setAttribute( QGeoPositionInfo::HorizontalAccuracy, 20 );
  emit positionUpdated( info );
}

/// @endcond
