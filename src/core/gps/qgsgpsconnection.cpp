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

QgsGPSConnection::QgsGPSConnection( QIODevice* dev, int pollInterval ): QObject( 0 ), mSource( dev ), mStatus( NotConnected )
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


//for device autodetection
#include "qextserialport.h"
#include "qgsnmeaconnection.h"
#include <QCoreApplication>
#include <QStringList>
#include <QTime>

QgsGPSConnection* QgsGPSConnection::detectGPSConnection()
{
  QStringList devicesToTry;
#ifdef WIN32
  devicesToTry << "COM1" << "COM2" << "COM3" << "COM4" << "COM5" << "COM6" << "COM7" << "COM8";
#else
  devicesToTry << "/dev/ttyS0" << "/dev/ttyS1" << "/dev/ttyS2" << "/dev/ttyS3" << "/dev/ttyUSB0" << "/dev/ttyUSB1";
#endif
  QList<BaudRateType> baudRatesToTry;
  baudRatesToTry << BAUD4800 << BAUD9600;


  QextSerialPort* port = 0;

  QList<BaudRateType>::const_iterator baudIt = baudRatesToTry.constBegin();
  for ( ; baudIt != baudRatesToTry.constEnd(); ++baudIt )
  {
    QStringList::const_iterator deviceIt = devicesToTry.constBegin();
    for ( ; deviceIt != devicesToTry.constEnd(); ++deviceIt )
    {
      port = new QextSerialPort( *deviceIt );
      port->setBaudRate( *baudIt );
      port->setFlowControl( FLOW_OFF );
      port->setParity( PAR_NONE );
      port->setDataBits( DATA_8 );
      port->setStopBits( STOP_1 );
      if ( port->open( QIODevice::ReadOnly | QIODevice::Unbuffered ) )
      {
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

        if ( c->status() == GPSDataReceived )
        {
          return c;
        }
        else
        {
          delete c;
        }
      }
      else
      {
        delete port;
      }
    }
  }

  //todo: check usb connections

  return 0;
}
