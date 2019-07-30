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

#include <QCoreApplication>
#include <QTime>
#include <QIODevice>
#include <QStringList>
#include <QFileInfo>

#include "qgsnmeaconnection.h"
#include "qgslogger.h"
#include "info.h"


bool QgsGpsInformation::isValid() const
{
  bool valid = false;
  if ( status == 'V' || fixType == NMEA_FIX_BAD || quality == 0 ) // some sources say that 'V' indicates position fix, but is below acceptable quality
  {
    valid = false;
  }
  else if ( fixType == NMEA_FIX_2D )
  {
    valid = true;
  }
  else if ( status == 'A' || fixType == NMEA_FIX_3D || quality > 0 ) // good
  {
    valid = true;
  }

  return valid;
}

QgsGpsInformation::FixStatus QgsGpsInformation::fixStatus() const
{
  FixStatus fixStatus = NoData;

  // no fix if any of the three report bad; default values are invalid values and won't be changed if the corresponding NMEA msg is not received
  if ( status == 'V' || fixType == NMEA_FIX_BAD || quality == 0 ) // some sources say that 'V' indicates position fix, but is below acceptable quality
  {
    fixStatus = NoFix;
  }
  else if ( fixType == NMEA_FIX_2D ) // 2D indication (from GGA)
  {
    fixStatus = Fix2D;
  }
  else if ( status == 'A' || fixType == NMEA_FIX_3D || quality > 0 ) // good
  {
    fixStatus = Fix3D;
  }
  return fixStatus;
}


QgsGpsConnection::QgsGpsConnection( QIODevice *dev ): QObject( nullptr ), mSource( dev ), mStatus( NotConnected )
{
  clearLastGPSInformation();
  QObject::connect( dev, &QIODevice::readyRead, this, &QgsGpsConnection::parseData );
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

  bool connected = mSource->open( QIODevice::ReadWrite | QIODevice::Unbuffered );
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
  return true;
}

void QgsGpsConnection::cleanupSource()
{
  if ( mSource )
  {
    mSource->close();
  }
  delete mSource;
  mSource = nullptr;
}

void QgsGpsConnection::setSource( QIODevice *source )
{
  cleanupSource();
  mSource = source;
  clearLastGPSInformation();
}

void QgsGpsConnection::clearLastGPSInformation()
{
  mLastGPSInformation = QgsGpsInformation();
}
