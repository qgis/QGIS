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

#include "qextserialport.h"
#include "qextserialenumerator.h"

#include "qgsnmeaconnection.h"

QgsGPSConnection::QgsGPSConnection( QIODevice* dev, int pollInterval ): QObject( 0 ), mSource( dev ), mStatus( NotConnected )
{
  init( pollInterval );
}

QgsGPSConnection::QgsGPSConnection( QString port, int pollInterval ): QObject( 0 ), mStatus( NotConnected )
{
  QextSerialPort *s = new QextSerialPort( port );
  s->setBaudRate( BAUD4800 );
  s->setFlowControl( FLOW_OFF );
  s->setParity( PAR_NONE );
  s->setDataBits( DATA_8 );
  s->setStopBits( STOP_2 );
  mSource = s;
  init( pollInterval );
}

void QgsGPSConnection::init( int pollInterval )
{
  clearLastGPSInformation();
  mPollTimer = new QTimer();
  mPollTimer->setInterval( pollInterval );
  QObject::connect( mPollTimer, SIGNAL( timeout() ), this, SLOT( parseData() ) );
}

QgsGPSConnection::~QgsGPSConnection()
{
  cleanupSource();
  delete mPollTimer;
}

bool QgsGPSConnection::startPolling()
{
  if ( mPollTimer )
  {
    mPollTimer->start();
  }
  return true;
}

bool QgsGPSConnection::stopPolling()
{
  if ( mPollTimer )
  {
    mPollTimer->stop();
  }
  return true;
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

void QgsGPSConnection::setTimer( QTimer* t )
{
  delete mPollTimer;
  mPollTimer = t;
  QObject::connect( mPollTimer, SIGNAL( timeout() ), this, SLOT( parseData() ) );
}



QgsGPSConnection* QgsGPSConnection::detectGPSConnection()
{
  QList<BaudRateType> baudRatesToTry;
  baudRatesToTry << BAUD4800 << BAUD9600 << BAUD38400;

  QextSerialPort* port = 0;

  QList<BaudRateType>::const_iterator baudIt = baudRatesToTry.constBegin();
  for ( ; baudIt != baudRatesToTry.constEnd(); ++baudIt )
  {
    foreach( QString portname, availablePorts() )
    {
      port = new QextSerialPort( portname );
      port->setBaudRate( *baudIt );
      port->setFlowControl( FLOW_OFF );
      port->setParity( PAR_NONE );
      port->setDataBits( DATA_8 );
      port->setStopBits( STOP_1 );
      if ( !port->open( QIODevice::ReadOnly | QIODevice::Unbuffered ) )
      {
        delete port;
        continue;
      }

      //setup connection
      QgsNMEAConnection* c = new QgsNMEAConnection( port, 200 );

      //return connection if gps data has been received
      c->startPolling();

      QTime t = QTime::currentTime().addSecs( 4 );
      while ( QTime::currentTime() < t )
      {
        QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 1000 );
      }
      c->stopPolling();

      if ( c->status() != GPSDataReceived )
      {
        delete c;
        continue;
      }

      return c;
    }
  }

  return 0;
}

QStringList QgsGPSConnection::availablePorts()
{
  QStringList devs;

#ifdef linux
  // look for linux serial devices
  foreach( QString linuxDev, QStringList() << "/dev/ttyS%1" << "/dev/ttyUSB%1" << "/dev/rfcomm%1" )
  {
    for ( int i = 0; i < 10; ++i )
    {
      if ( QFileInfo( linuxDev.arg( i ) ).exists() )
      {
        devs << linuxDev.arg( i );
      }
    }
  }

  mCboDevices->addItem( "usb:" );
#endif

#ifdef __FreeBSD__ // freebsd
  // and freebsd devices (untested)
  foreach( QString freebsdDev, QStringList() << "/dev/cuaa%1" << "/dev/ucom%1" )
  {
    for ( int i = 0; i < 10; ++i )
    {
      if ( QFileInfo( freebsdDev.arg( i ) ).exists() )
      {
        devs << freebsdDev.arg( i );
      }
    }
  }
#endif

#ifdef sparc
  // and solaris devices (also untested)
  QString solarisDev( "/dev/cua/%1" );
  for ( char i = 'a'; i < 'k'; ++i )
  {
    if ( QFileInfo( solarisDev.arg( i ) ).exists() )
    {
      devs << solarisDev.arg( i );
    }
  }
#endif

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
  QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
  foreach( QextPortInfo port, ports )
  {
    devs << port.portName;
  }
#endif

  // OpenBSD, NetBSD etc? Anyone?

  return devs;
}
