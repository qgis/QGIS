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

#include "qextserialport.h"
#include "qextserialenumerator.h"

#include "qgsnmeaconnection.h"
#include "qgslogger.h"

QgsGPSConnection::QgsGPSConnection( QIODevice* dev ): QObject( 0 ), mSource( dev ), mStatus( NotConnected )
{
  clearLastGPSInformation();
  QObject::connect( dev, SIGNAL( readyRead() ), this, SLOT( parseData() ) );
}

QgsGPSConnection::~QgsGPSConnection()
{
  cleanupSource();
}

bool QgsGPSConnection::connect()
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

bool QgsGPSConnection::close()
{
  if ( !mSource )
  {
    return false;
  }

  mSource->close();
  return true;
}

void QgsGPSConnection::cleanupSource()
{
  if ( mSource )
  {
    mSource->close();
  }
  delete mSource;
  mSource = 0;
}

void QgsGPSConnection::setSource( QIODevice* source )
{
  cleanupSource();
  mSource = source;
  clearLastGPSInformation();
}

void QgsGPSConnection::clearLastGPSInformation()
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
}
