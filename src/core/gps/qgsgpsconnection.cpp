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
#include <QSerialPort>
#include <QSerialPortInfo>

#include "qgsnmeaconnection.h"
#include "qgslogger.h"

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
  mLastGPSInformation.direction = 0;
  mLastGPSInformation.elevation = 0;
  mLastGPSInformation.hdop = 0;
  mLastGPSInformation.latitude = 0;
  mLastGPSInformation.longitude = 0;
  mLastGPSInformation.pdop = 0;
  mLastGPSInformation.satellitesInView.clear();
  mLastGPSInformation.speed = 0;
  mLastGPSInformation.vdop = 0;
  mLastGPSInformation.hacc = -1;
  mLastGPSInformation.vacc = -1;
  mLastGPSInformation.quality = -1;  // valid values: 0,1,2, maybe others
  mLastGPSInformation.satellitesUsed = 0;
  mLastGPSInformation.fixMode = ' ';
  mLastGPSInformation.fixType = 0; // valid values: 1,2,3
  mLastGPSInformation.status = ' '; // valid values: A,V
  mLastGPSInformation.utcDateTime.setDate( QDate() );
  mLastGPSInformation.satPrn.clear();
  mLastGPSInformation.utcDateTime.setTime( QTime() );
  mLastGPSInformation.satInfoComplete = false;
}
