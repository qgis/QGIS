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
#include "qgssettings.h"

#if defined(QT_POSITIONING_LIB)
#include "qgsqtlocationconnection.h"
#endif

#include <QStringList>
#include <QFileInfo>
#include <QTimer>

#if defined( HAVE_QT5SERIALPORT )
#include <QSerialPortInfo>
#include <QSerialPort>
#endif

QList< QPair<QString, QString> > QgsGpsDetector::availablePorts()
{
  QList< QPair<QString, QString> > devs;

  // try local QtLocation first
#if defined(QT_POSITIONING_LIB)
  devs << QPair<QString, QString>( QStringLiteral( "internalGPS" ), tr( "internal GPS" ) );
#endif

  // try local gpsd first
  devs << QPair<QString, QString>( QStringLiteral( "localhost:2947:" ), tr( "local gpsd" ) );

  // try serial ports
#if defined( HAVE_QT5SERIALPORT )
  for ( const QSerialPortInfo &p : QSerialPortInfo::availablePorts() )
  {
    devs << QPair<QString, QString>( p.portName(), tr( "%1: %2" ).arg( p.portName(), p.description() ) );
  }
#endif

  return devs;
}

QgsGpsDetector::QgsGpsDetector( const QString &portName )
{
#if defined( HAVE_QT5SERIALPORT )
  mBaudList << QSerialPort::Baud4800 << QSerialPort::Baud9600 << QSerialPort::Baud38400 << QSerialPort::Baud57600 << QSerialPort::Baud115200;  //add 57600 for SXBlueII GPS unit
#endif

  if ( portName.isEmpty() )
  {
    mPortList = availablePorts();
  }
  else
  {
    mPortList << QPair<QString, QString>( portName, portName );
  }
}

QgsGpsDetector::~QgsGpsDetector() = default;

void QgsGpsDetector::advance()
{
  mConn.reset();

  QgsSettings settings;

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

      mConn = std::make_unique< QgsGpsdConnection >( gpsParams[0], gpsParams[1].toShort(), gpsParams[2] );
    }
    else if ( mPortList.at( mPortIndex ).first.contains( QLatin1String( "internalGPS" ) ) )
    {
#if defined(QT_POSITIONING_LIB)
      mConn = std::make_unique< QgsQtLocationConnection >();
#else
      qWarning( "QT_POSITIONING_LIB not found and mPortList matches internalGPS, this should never happen" );
#endif
    }
    else
    {
#if defined( HAVE_QT5SERIALPORT )
      std::unique_ptr< QSerialPort > serial = std::make_unique< QSerialPort >( mPortList.at( mPortIndex ).first );

      serial->setBaudRate( mBaudList[ mBaudIndex ] );

      serial->setFlowControl( settings.enumValue( QStringLiteral( "gps/flow_control" ), QSerialPort::NoFlowControl, QgsSettings::Core ) );
      serial->setParity( settings.enumValue( QStringLiteral( "gps/parity" ), QSerialPort::NoParity, QgsSettings::Core ) );
      serial->setDataBits( settings.enumValue( QStringLiteral( "gps/data_bits" ), QSerialPort::Data8, QgsSettings::Core ) );
      serial->setStopBits( settings.enumValue( QStringLiteral( "gps/stop_bits" ), QSerialPort::OneStop, QgsSettings::Core ) );

      if ( serial->open( QIODevice::ReadOnly ) )
      {
        mConn = std::make_unique< QgsNmeaConnection >( serial.release() );
      }
#else
      qWarning( "QT5SERIALPORT not found and mPortList matches serial port, this should never happen" );
#endif
    }
  }

  connect( mConn.get(), &QgsGpsConnection::stateChanged, this, static_cast < void ( QgsGpsDetector::* )( const QgsGpsInformation & ) >( &QgsGpsDetector::detected ) );
  connect( mConn.get(), &QObject::destroyed, this, &QgsGpsDetector::connDestroyed );

  // leave 2s to pickup a valid string
  QTimer::singleShot( 2000, this, &QgsGpsDetector::advance );
}

void QgsGpsDetector::detected( const QgsGpsInformation &info )
{
  Q_UNUSED( info )

  if ( !mConn )
  {
    // advance if connection was destroyed
    advance();
  }
  else if ( mConn->status() == QgsGpsConnection::GPSDataReceived )
  {
    // signal detected

    // let's hope there's a single, unique connection to this signal... otherwise... boom
    emit detected( mConn.release() );

    deleteLater();
  }
}

void QgsGpsDetector::connDestroyed( QObject *obj )
{
  // WTF? This whole class needs re-writing...
  if ( obj == mConn.get() )
  {
    mConn.release();
  }
}
