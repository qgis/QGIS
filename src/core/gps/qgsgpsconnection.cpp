/***************************************************************************
                          qgsgpsconnection.cpp  -  description
                          --------------------
    begin                : November 30th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpsconnection.h"
#include <QIODevice>


QgsGpsConnection::QgsGpsConnection( QIODevice *dev )
  : QObject( nullptr )
  , mSource( dev )
{
  if ( mSource )
    QObject::connect( mSource.get(), &QIODevice::readyRead, this, &QgsGpsConnection::parseData );

  QObject::connect( this, &QgsGpsConnection::stateChanged, this, &QgsGpsConnection::onStateChanged );
}

QgsGpsConnection::~QgsGpsConnection()
{
  cleanupSource();
}

bool QgsGpsConnection::connect()
{
  if ( !mSource )
  {
    return false;
  }

  const bool connected = mSource->open( QIODevice::ReadWrite | QIODevice::Unbuffered );
  if ( connected )
  {
    mStatus = Connected;
  }
  return connected;
}

bool QgsGpsConnection::close()
{
  if ( !mSource )
  {
    return false;
  }

  mSource->close();

  if ( mLastFixStatus != Qgis::GpsFixStatus::NoData )
  {
    mLastFixStatus = Qgis::GpsFixStatus::NoData;
    emit fixStatusChanged( mLastFixStatus );
  }

  return true;
}

void QgsGpsConnection::cleanupSource()
{
  if ( mSource )
  {
    mSource->close();
  }
  mSource.reset();

  if ( mLastFixStatus != Qgis::GpsFixStatus::NoData )
  {
    mLastFixStatus = Qgis::GpsFixStatus::NoData;
    emit fixStatusChanged( mLastFixStatus );
  }
}

void QgsGpsConnection::setSource( QIODevice *source )
{
  cleanupSource();
  mSource.reset( source );
  QObject::connect( mSource.get(), &QIODevice::readyRead, this, &QgsGpsConnection::parseData );

  clearLastGPSInformation();
}

void QgsGpsConnection::onStateChanged( const QgsGpsInformation &info )
{
  if ( info.isValid() )
  {
    const QgsPoint oldPosition = mLastLocation;
    mLastLocation = QgsPoint( info.longitude, info.latitude, info.elevation );
    if ( mLastLocation != oldPosition )
    {
      emit positionChanged( mLastLocation );
    }
  }

  Qgis::GnssConstellation bestFixConstellation = Qgis::GnssConstellation::Unknown;
  Qgis::GpsFixStatus bestFix = info.bestFixStatus( bestFixConstellation );
  if ( bestFix != mLastFixStatus )
  {
    mLastFixStatus = bestFix;
    emit fixStatusChanged( mLastFixStatus );
  }
}

void QgsGpsConnection::clearLastGPSInformation()
{
  mLastGPSInformation = QgsGpsInformation();
}
