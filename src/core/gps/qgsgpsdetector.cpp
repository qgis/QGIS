/***************************************************************************
                          qgsgpsdetector.cpp  -  description
                          --------------------
    begin                : January 13th, 2009
    copyright            : (C) 2009 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpsdetector.h"
#include "qgslogger.h"
#include "qgsgpsconnection.h"
#include "qgsnmeaconnection.h"
#include "qgsgpsdconnection.h"


#if defined(HAVE_QT_MOBILITY_LOCATION ) || defined(QT_POSITIONING_LIB)
#include "qgsqtlocationconnection.h"
#endif

#include <QStringList>
#include <QFileInfo>
#include <QTimer>
#include <QSerialPortInfo>

QList< QPair<QString, QString> > QgsGpsDetector::availablePorts()
{
  QList< QPair<QString, QString> > devs;

  // try local QtLocation first
#if defined(HAVE_QT_MOBILITY_LOCATION ) || defined(QT_POSITIONING_LIB)
  devs << QPair<QString, QString>( QStringLiteral( "internalGPS" ), tr( "internal GPS" ) );
#endif

  // try local gpsd first
  devs << QPair<QString, QString>( QStringLiteral( "localhost:2947:" ), tr( "local gpsd" ) );

  for ( auto p : QSerialPortInfo::availablePorts() )
  {
    devs << QPair<QString, QString>( p.portName(), tr( "%1: %2" ).arg( p.portName(), p.description() ) );
  }

  return devs;
}

QgsGpsDetector::QgsGpsDetector( const QString &portName )
{
  mConn = nullptr;
  mBaudList << QSerialPort::Baud4800 << QSerialPort::Baud9600 << QSerialPort::Baud38400 << QSerialPort::Baud57600 << QSerialPort::Baud115200;  //add 57600 for SXBlueII GPS unit

  if ( portName.isEmpty() )
  {
    mPortList = availablePorts();
  }
  else
  {
    mPortList << QPair<QString, QString>( portName, portName );
  }

  mPortIndex = 0;
  mBaudIndex = -1;
}

QgsGpsDetector::~QgsGpsDetector()
{
  delete mConn;
}

void QgsGpsDetector::advance()
{
  delete mConn;

  mConn = nullptr;

  while ( !mConn )
  {
    mBaudIndex++;
    if ( mBaudIndex == mBaudList.size() )
    {
      mBaudIndex = 0;
      mPortIndex++;
    }

    if ( mPortIndex == mPortList.size() )
    {
      emit detectionFailed();
      deleteLater();
      return;
    }

    if ( mPortList.at( mPortIndex ).first.contains( ':' ) )
    {
      mBaudIndex = mBaudList.size() - 1;

      QStringList gpsParams = mPortList.at( mPortIndex ).first.split( ':' );

      Q_ASSERT( gpsParams.size() >= 3 );

      mConn = new QgsGpsdConnection( gpsParams[0], gpsParams[1].toShort(), gpsParams[2] );
    }
    else if ( mPortList.at( mPortIndex ).first.contains( QLatin1String( "internalGPS" ) ) )
    {
#if defined(HAVE_QT_MOBILITY_LOCATION ) || defined(QT_POSITIONING_LIB)
      mConn = new QgsQtLocationConnection();
#else
      qWarning( "QT_MOBILITY_LOCATION not found and mPortList matches internalGPS, this should never happen" );
#endif
    }
    else
    {
      QSerialPort *serial = new QSerialPort( mPortList.at( mPortIndex ).first );

      serial->setBaudRate( mBaudList[ mBaudIndex ] );
      serial->setFlowControl( QSerialPort::NoFlowControl );
      serial->setParity( QSerialPort::NoParity );
      serial->setDataBits( QSerialPort::Data8 );
      serial->setStopBits( QSerialPort::OneStop );

      if ( serial->open( QIODevice::ReadOnly ) )
      {
        mConn = new QgsNmeaConnection( serial );
      }
      else
      {
        delete serial;
      }
    }
  }

  connect( mConn, &QgsGpsConnection::stateChanged, this, static_cast < void ( QgsGpsDetector::* )( const QgsGpsInformation & ) >( &QgsGpsDetector::detected ) );
  connect( mConn, &QObject::destroyed, this, &QgsGpsDetector::connDestroyed );

  // leave 2s to pickup a valid string
  QTimer::singleShot( 2000, this, &QgsGpsDetector::advance );
}

void QgsGpsDetector::detected( const QgsGpsInformation &info )
{
  Q_UNUSED( info );

  if ( !mConn )
  {
    // advance if connection was destroyed
    advance();
  }
  else if ( mConn->status() == QgsGpsConnection::GPSDataReceived )
  {
    // signal detection
    QgsGpsConnection *conn = mConn;
    mConn = nullptr;
    emit detected( conn );
    deleteLater();
  }
}

void QgsGpsDetector::connDestroyed( QObject *obj )
{
  if ( obj == mConn )
  {
    mConn = nullptr;
  }
}
