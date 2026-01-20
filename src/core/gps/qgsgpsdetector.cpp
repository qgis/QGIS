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
#include "qgsgpsdconnection.h"
#include "qgslogger.h"
#include "qgsnmeaconnection.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingstree.h"

#include "moc_qgsgpsdetector.cpp"

#if defined(QT_POSITIONING_LIB)
#include "qgsqtlocationconnection.h"
#endif

#include <QStringList>
#include <QFileInfo>
#include <QTimer>

#if defined( HAVE_QTSERIALPORT )
#include <QSerialPortInfo>
#include <QSerialPort>

const QgsSettingsEntryEnumFlag<QSerialPort::FlowControl> *QgsGpsDetector::settingsGpsFlowControl = new QgsSettingsEntryEnumFlag<QSerialPort::FlowControl>( u"flow-control"_s, QgsSettingsTree::sTreeGps, QSerialPort::NoFlowControl );
const QgsSettingsEntryEnumFlag<QSerialPort::StopBits> *QgsGpsDetector::settingsGpsStopBits = new QgsSettingsEntryEnumFlag<QSerialPort::StopBits>( u"stop-bits"_s, QgsSettingsTree::sTreeGps, QSerialPort::OneStop );
const QgsSettingsEntryEnumFlag<QSerialPort::DataBits> *QgsGpsDetector::settingsGpsDataBits = new QgsSettingsEntryEnumFlag<QSerialPort::DataBits>( u"data-bits"_s, QgsSettingsTree::sTreeGps, QSerialPort::Data8 );
const QgsSettingsEntryEnumFlag<QSerialPort::Parity> *QgsGpsDetector::settingsGpsParity = new QgsSettingsEntryEnumFlag<QSerialPort::Parity>( u"parity"_s, QgsSettingsTree::sTreeGps, QSerialPort::NoParity );
#endif

QList< QPair<QString, QString> > QgsGpsDetector::availablePorts()
{
  QList< QPair<QString, QString> > devs;

  // try local QtLocation first
#if defined(QT_POSITIONING_LIB)
  devs << QPair<QString, QString>( u"internalGPS"_s, tr( "internal GPS" ) );
#endif

  // try local gpsd first
  devs << QPair<QString, QString>( u"localhost:2947:"_s, tr( "local gpsd" ) );

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
    QgsDebugMsgLevel( u"Attempting to autodetect GPS connection"_s, 2 );
    mPortList = availablePorts();
  }
  else
  {
    QgsDebugMsgLevel( u"Attempting GPS connection for %1"_s.arg( portName ), 2 );
    mPortList << QPair<QString, QString>( portName, portName );
  }

  mTimeoutTimer = new QTimer( this );
  mTimeoutTimer->setSingleShot( true );
  connect( mTimeoutTimer, &QTimer::timeout, this, &QgsGpsDetector::connectionTimeout );
}

QgsGpsDetector::~QgsGpsDetector()
{
  QgsDebugMsgLevel( u"Destroying GPS detector"_s, 2 );
}

QgsGpsConnection *QgsGpsDetector::takeConnection()
{
  if ( mUseUnsafeSignals )
  {
    QgsDebugError( u"QgsGpsDetector::takeConnection() incorrectly called when useUnsafeSignals option is in effect"_s );
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
    QgsDebugMsgLevel( u"Detected GPS connection is being taken by caller"_s, 2 );
  }
  else
  {
    QgsDebugError( u"Something is trying to take the GPS connection, but it doesn't exist!"_s );
  }
#endif

  return mConn.release();
}

void QgsGpsDetector::advance()
{
  if ( mConn )
  {
    QgsDebugMsgLevel( u"Destroying existing connection to attempt next configuration combination"_s, 2 );
    mConn.reset();
  }

  QgsDebugMsgLevel( u"Trying to find a connection..."_s, 2 );

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
      QgsDebugError( u"No more devices to try!"_s );
      emit detectionFailed();
      deleteLater();
      return;
    }

    QgsDebugMsgLevel( u"Attempting connection to device %1 @ %2"_s.arg( mPortIndex ).arg( mBaudIndex ), 2 );

    if ( mPortList.at( mPortIndex ).first.contains( ':' ) )
    {
      mBaudIndex = mBaudList.size() - 1;

      QStringList gpsParams = mPortList.at( mPortIndex ).first.split( ':' );
      if ( gpsParams.size() < 3 )
      {
        QgsDebugError( u"If the port name contains a colon, then it should have more than one colon (e.g., host:port:device). Port name: %1"_s.arg( mPortList.at( mPortIndex ).first ) );
        emit detectionFailed();
        deleteLater();
        return;
      }

      QgsDebugMsgLevel( u"Connecting to GPSD device %1"_s.arg( gpsParams.join( ',' ) ), 2 );

      mConn = std::make_unique< QgsGpsdConnection >( gpsParams[0], gpsParams[1].toShort(), gpsParams[2] );
    }
    else if ( mPortList.at( mPortIndex ).first.contains( "internalGPS"_L1 ) )
    {
#if defined(QT_POSITIONING_LIB)
      QgsDebugMsgLevel( u"Connecting to QtLocation service device"_s, 2 );
      mConn = std::make_unique< QgsQtLocationConnection >();
#else
      QgsDebugError( u"QT_POSITIONING_LIB not found and mPortList matches internalGPS, this should never happen"_s );
      qWarning( "QT_POSITIONING_LIB not found and mPortList matches internalGPS, this should never happen" );
#endif
    }
    else
    {
#if defined(HAVE_QTSERIALPORT)
      auto serial = std::make_unique< QSerialPort >( mPortList.at( mPortIndex ).first );

      serial->setBaudRate( mBaudList[ mBaudIndex ] );

      serial->setFlowControl( QgsGpsDetector::settingsGpsFlowControl->value() );
      serial->setParity( QgsGpsDetector::settingsGpsParity->value() );
      serial->setDataBits( QgsGpsDetector::settingsGpsDataBits->value() );
      serial->setStopBits( QgsGpsDetector::settingsGpsStopBits->value() );

      QgsDebugMsgLevel( u"Connecting to serial GPS device %1 (@ %2)"_s.arg( mPortList.at( mPortIndex ).first ).arg( mBaudList[ mBaudIndex ] ), 2 );

      if ( serial->open( QIODevice::ReadOnly ) )
      {
        QgsDebugMsgLevel( u"Successfully opened, have a port connection ready"_s, 2 );
        mConn = std::make_unique< QgsNmeaConnection >( serial.release() );
      }
      else
      {
        QgsDebugError( u"Serial port could NOT be opened"_s );
      }
#else
      QgsDebugError( u"QTSERIALPORT not found and mPortList matches serial port, this should never happen"_s );
      qWarning( "QTSERIALPORT not found and mPortList matches serial port, this should never happen" );
#endif
    }

    if ( !mConn )
    {
      QgsDebugError( u"Got to end of connection handling loop, but have no connection!"_s );
    }
  }

  QgsDebugMsgLevel( u"Have a connection, now listening for messages"_s, 2 );

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
  QgsDebugMsgLevel( u"Detected information"_s, 2 );

  if ( !mConn )
  {
    mTimeoutTimer->stop();

    // advance if connection was destroyed
    QgsDebugError( u"Got information, but CONNECTION WAS DESTROYED EXTERNALLY!"_s );
    advance();
  }
  else if ( mConn->status() == QgsGpsConnection::GPSDataReceived )
  {
    mTimeoutTimer->stop();
    // stop listening for state changed signals, we've already validated this connection and don't want subsequent calls
    // to QgsGpsDetector::detected being made
    disconnect( mConn.get(), &QgsGpsConnection::stateChanged, this, qOverload< const QgsGpsInformation & >( &QgsGpsDetector::detected ) );

    // signal detected
    QgsDebugMsgLevel( u"Connection status IS GPSDataReceived"_s, 2 );

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
    QgsDebugMsgLevel( u"Connection status is NOT GPSDataReceived. It is %1"_s.arg( mConn->status() ), 2 );
  }
}

void QgsGpsDetector::connectionTimeout()
{
  QgsDebugMsgLevel( u"No data received within max listening time"_s, 2 );
  advance();
}

void QgsGpsDetector::connDestroyed( QObject *obj )
{
  QgsDebugError( u"CONNECTION WAS DESTROYED EXTERNALLY!"_s );

  // WTF? This whole class needs re-writing...
  if ( obj == mConn.get() )
  {
    mConn.release(); // cppcheck-suppress ignoredReturnValue
  }
}
