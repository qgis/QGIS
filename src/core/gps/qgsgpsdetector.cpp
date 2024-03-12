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
#include "qgsgpsconnection.h"
#include "qgsnmeaconnection.h"
#include "qgsgpsdconnection.h"
#include "qgssettingstree.h"
#include "qgssettingsentryenumflag.h"
#include "qgslogger.h"

#if defined(QT_POSITIONING_LIB)
#include "qgsqtlocationconnection.h"
#endif

#include <QStringList>
#include <QFileInfo>
#include <QTimer>

#if defined( HAVE_QTSERIALPORT )
#include <QSerialPortInfo>
#include <QSerialPort>

const QgsSettingsEntryEnumFlag<QSerialPort::FlowControl> *QgsGpsDetector::settingsGpsFlowControl = new QgsSettingsEntryEnumFlag<QSerialPort::FlowControl>( QStringLiteral( "flow-control" ), QgsSettingsTree::sTreeGps, QSerialPort::NoFlowControl );
const QgsSettingsEntryEnumFlag<QSerialPort::StopBits> *QgsGpsDetector::settingsGpsStopBits = new QgsSettingsEntryEnumFlag<QSerialPort::StopBits>( QStringLiteral( "stop-bits" ), QgsSettingsTree::sTreeGps, QSerialPort::OneStop );
const QgsSettingsEntryEnumFlag<QSerialPort::DataBits> *QgsGpsDetector::settingsGpsDataBits = new QgsSettingsEntryEnumFlag<QSerialPort::DataBits>( QStringLiteral( "data-bits" ), QgsSettingsTree::sTreeGps, QSerialPort::Data8 );
const QgsSettingsEntryEnumFlag<QSerialPort::Parity> *QgsGpsDetector::settingsGpsParity = new QgsSettingsEntryEnumFlag<QSerialPort::Parity>( QStringLiteral( "parity" ), QgsSettingsTree::sTreeGps, QSerialPort::NoParity );
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
#if defined( HAVE_QTSERIALPORT )
  for ( const QSerialPortInfo &p : QSerialPortInfo::availablePorts() )
  {
    devs << QPair<QString, QString>( p.portName(), tr( "%1: %2" ).arg( p.portName(), p.description() ) );
  }
#endif

  return devs;
}

QgsGpsDetector::QgsGpsDetector( const QString &portName, bool useUnsafeSignals )
  : mUseUnsafeSignals( useUnsafeSignals )
{
#if defined( HAVE_QTSERIALPORT )
  mBaudList << QSerialPort::Baud4800 << QSerialPort::Baud9600 << QSerialPort::Baud38400 << QSerialPort::Baud57600 << QSerialPort::Baud115200;  //add 57600 for SXBlueII GPS unit
#endif

  if ( portName.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Attempting to autodetect GPS connection" ), 2 );
    mPortList = availablePorts();
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Attempting GPS connection for %1" ).arg( portName ), 2 );
    mPortList << QPair<QString, QString>( portName, portName );
  }

  mTimeoutTimer = new QTimer( this );
  mTimeoutTimer->setSingleShot( true );
  connect( mTimeoutTimer, &QTimer::timeout, this, &QgsGpsDetector::connectionTimeout );
}

QgsGpsDetector::~QgsGpsDetector()
{
  QgsDebugMsgLevel( QStringLiteral( "Destroying GPS detector" ), 2 );
}

QgsGpsConnection *QgsGpsDetector::takeConnection()
{
  if ( mUseUnsafeSignals )
  {
    QgsDebugError( QStringLiteral( "QgsGpsDetector::takeConnection() incorrectly called when useUnsafeSignals option is in effect" ) );
    return nullptr;
  }

  if ( mConn )
  {
    // this is NOT the detectors connection anymore, so disconnect all signals from the connection
    // to the detector so that there's no unwanted interaction with the detector
    mConn->disconnect( this );
  }

#ifdef QGISDEBUG
  if ( mConn )
  {
    QgsDebugMsgLevel( QStringLiteral( "Detected GPS connection is being taken by caller" ), 2 );
  }
  else
  {
    QgsDebugError( QStringLiteral( "Something is trying to take the GPS connection, but it doesn't exist!" ) );
  }
#endif

  return mConn.release();
}

void QgsGpsDetector::advance()
{
  if ( mConn )
  {
    QgsDebugMsgLevel( QStringLiteral( "Destroying existing connection to attempt next configuration combination" ), 2 );
    mConn.reset();
  }

  QgsDebugMsgLevel( QStringLiteral( "Trying to find a connection..." ), 2 );

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
      QgsDebugError( QStringLiteral( "No more devices to try!" ) );
      emit detectionFailed();
      deleteLater();
      return;
    }

    QgsDebugMsgLevel( QStringLiteral( "Attempting connection to device %1 @ %2" ).arg( mPortIndex ).arg( mBaudIndex ), 2 );

    if ( mPortList.at( mPortIndex ).first.contains( ':' ) )
    {
      mBaudIndex = mBaudList.size() - 1;

      QStringList gpsParams = mPortList.at( mPortIndex ).first.split( ':' );

      Q_ASSERT( gpsParams.size() >= 3 );
      QgsDebugMsgLevel( QStringLiteral( "Connecting to GPSD device %1" ).arg( gpsParams.join( ',' ) ), 2 );

      mConn = std::make_unique< QgsGpsdConnection >( gpsParams[0], gpsParams[1].toShort(), gpsParams[2] );
    }
    else if ( mPortList.at( mPortIndex ).first.contains( QLatin1String( "internalGPS" ) ) )
    {
#if defined(QT_POSITIONING_LIB)
      QgsDebugMsgLevel( QStringLiteral( "Connecting to QtLocation service device" ), 2 );
      mConn = std::make_unique< QgsQtLocationConnection >();
#else
      QgsDebugError( QStringLiteral( "QT_POSITIONING_LIB not found and mPortList matches internalGPS, this should never happen" ) );
      qWarning( "QT_POSITIONING_LIB not found and mPortList matches internalGPS, this should never happen" );
#endif
    }
    else
    {
#if defined( HAVE_QTSERIALPORT )
      std::unique_ptr< QSerialPort > serial = std::make_unique< QSerialPort >( mPortList.at( mPortIndex ).first );

      serial->setBaudRate( mBaudList[ mBaudIndex ] );

      serial->setFlowControl( QgsGpsDetector::settingsGpsFlowControl->value() );
      serial->setParity( QgsGpsDetector::settingsGpsParity->value() );
      serial->setDataBits( QgsGpsDetector::settingsGpsDataBits->value() );
      serial->setStopBits( QgsGpsDetector::settingsGpsStopBits->value() );

      QgsDebugMsgLevel( QStringLiteral( "Connecting to serial GPS device %1 (@ %2)" ).arg( mPortList.at( mPortIndex ).first ).arg( mBaudList[ mBaudIndex ] ), 2 );

      if ( serial->open( QIODevice::ReadOnly ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "Successfully opened, have a port connection ready" ), 2 );
        mConn = std::make_unique< QgsNmeaConnection >( serial.release() );
      }
      else
      {
        QgsDebugError( QStringLiteral( "Serial port could NOT be opened" ) );
      }
#else
      QgsDebugError( QStringLiteral( "QT5SERIALPORT not found and mPortList matches serial port, this should never happen" ) );
      qWarning( "QT5SERIALPORT not found and mPortList matches serial port, this should never happen" );
#endif
    }

    if ( !mConn )
    {
      QgsDebugError( QStringLiteral( "Got to end of connection handling loop, but have no connection!" ) );
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Have a connection, now listening for messages" ), 2 );

  connect( mConn.get(), &QgsGpsConnection::stateChanged, this, qOverload< const QgsGpsInformation & >( &QgsGpsDetector::detected ) );
  if ( mUseUnsafeSignals )
  {
    connect( mConn.get(), &QObject::destroyed, this, &QgsGpsDetector::connDestroyed );
  }

  // leave 2s to pickup a valid string
  mTimeoutTimer->start( 2000 );
}

void QgsGpsDetector::detected( const QgsGpsInformation & )
{
  QgsDebugMsgLevel( QStringLiteral( "Detected information" ), 2 );

  if ( !mConn )
  {
    mTimeoutTimer->stop();

    // advance if connection was destroyed
    QgsDebugError( QStringLiteral( "Got information, but CONNECTION WAS DESTROYED EXTERNALLY!" ) );
    advance();
  }
  else if ( mConn->status() == QgsGpsConnection::GPSDataReceived )
  {
    mTimeoutTimer->stop();
    // stop listening for state changed signals, we've already validated this connection and don't want subsequent calls
    // to QgsGpsDetector::detected being made
    disconnect( mConn.get(), &QgsGpsConnection::stateChanged, this, qOverload< const QgsGpsInformation & >( &QgsGpsDetector::detected ) );

    // signal detected
    QgsDebugMsgLevel( QStringLiteral( "Connection status IS GPSDataReceived" ), 2 );

    if ( mUseUnsafeSignals )
    {
      // let's hope there's a single, unique connection to this signal... otherwise... boom!
      Q_NOWARN_DEPRECATED_PUSH
      emit detected( mConn.release() );
      Q_NOWARN_DEPRECATED_POP
    }
    else
    {
      emit connectionDetected();
    }

    deleteLater();
  }
  else
  {
    // don't stop timeout, we keep waiting to see if later we get the desired connection status...
    QgsDebugMsgLevel( QStringLiteral( "Connection status is NOT GPSDataReceived. It is %1" ).arg( mConn->status() ), 2 );
  }
}

void QgsGpsDetector::connectionTimeout()
{
  QgsDebugMsgLevel( QStringLiteral( "No data received within max listening time" ), 2 );
  advance();
}

void QgsGpsDetector::connDestroyed( QObject *obj )
{
  QgsDebugError( QStringLiteral( "CONNECTION WAS DESTROYED EXTERNALLY!" ) );

  // WTF? This whole class needs re-writing...
  if ( obj == mConn.get() )
  {
    mConn.release();
  }
}
