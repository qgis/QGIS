/***************************************************************************
    qgsappgpsconnection.cpp
    -------------------------
    begin                : October 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappgpsconnection.h"
#include "qgsapplication.h"
#include "qgsgpsconnection.h"
#include "qgsgpsconnectionregistry.h"
#include "qgsgpsdetector.h"
#include "qgisapp.h"
#include "qgsstatusbar.h"
#include "qgsmessagebar.h"

QgsAppGpsConnection::QgsAppGpsConnection( QObject *parent )
  : QObject( parent )
{

}

QgsAppGpsConnection::~QgsAppGpsConnection()
{
  if ( mConnection )
  {
    disconnectGps();
  }
}

QgsGpsConnection *QgsAppGpsConnection::connection()
{
  return mConnection;
}

bool QgsAppGpsConnection::isConnected() const
{
  return static_cast< bool >( mConnection );
}

void QgsAppGpsConnection::setConnection( QgsGpsConnection *connection )
{
  if ( mConnection )
  {
    disconnectGps();
  }

  onConnected( connection );
}

QgsPoint QgsAppGpsConnection::lastValidLocation() const
{
  return mConnection->lastValidLocation();
}

void QgsAppGpsConnection::connectGps()
{
  QString port;

  Qgis::GpsConnectionType connectionType = Qgis::GpsConnectionType::Automatic;
  QString gpsdHost;
  int gpsdPort = 0;
  QString gpsdDevice;
  QString serialDevice;
  if ( QgsGpsConnection::settingsGpsConnectionType.exists() )
  {
    connectionType = QgsGpsConnection::settingsGpsConnectionType.value();
    gpsdHost = QgsGpsConnection::settingsGpsdHostName.value();
    gpsdPort = static_cast< int >( QgsGpsConnection::settingsGpsdPortNumber.value() );
    gpsdDevice = QgsGpsConnection::settingsGpsdDeviceName.value();
    serialDevice = QgsGpsConnection::settingsGpsSerialDevice.value();
  }
  else
  {
    // legacy settings
    QgsSettings settings;
    const QString portMode = settings.value( QStringLiteral( "portMode" ), "scanPorts", QgsSettings::Gps ).toString();

    if ( portMode == QLatin1String( "scanPorts" ) )
    {
      connectionType = Qgis::GpsConnectionType::Automatic;
    }
    else if ( portMode == QLatin1String( "internalGPS" ) )
    {
      connectionType = Qgis::GpsConnectionType::Internal;
    }
    else if ( portMode == QLatin1String( "explicitPort" ) )
    {
      connectionType = Qgis::GpsConnectionType::Serial;
    }
    else if ( portMode == QLatin1String( "gpsd" ) )
    {
      connectionType = Qgis::GpsConnectionType::Gpsd;
    }

    gpsdHost = settings.value( QStringLiteral( "gpsdHost" ), "localhost", QgsSettings::Gps ).toString();
    gpsdPort = settings.value( QStringLiteral( "gpsdPort" ), 2947, QgsSettings::Gps ).toInt();
    gpsdDevice = settings.value( QStringLiteral( "gpsdDevice" ), QVariant(), QgsSettings::Gps ).toString();
    serialDevice = settings.value( QStringLiteral( "lastPort" ), "", QgsSettings::Gps ).toString();
  }

  switch ( connectionType )
  {
    case Qgis::GpsConnectionType::Automatic:
      break;
    case Qgis::GpsConnectionType::Internal:
      port = QStringLiteral( "internalGPS" );
      break;
    case Qgis::GpsConnectionType::Serial:
      port = QgsGpsConnection::settingsGpsSerialDevice.value();
      if ( port.isEmpty() )
      {
        QgisApp::instance()->statusBarIface()->clearMessage();
        QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "No path to the GPS port is specified. Please set a path then try again." ) );
        emit connectionError( tr( "No path to the GPS port is specified. Please set a path then try again." ) );
        return;
      }
      break;
    case Qgis::GpsConnectionType::Gpsd:
    {
      port = QStringLiteral( "%1:%2:%3" ).arg( gpsdHost ).arg( gpsdPort ).arg( gpsdDevice );
      break;
    }
  }

  emit connecting();

  emit fixStatusChanged( Qgis::GpsFixStatus::NoData );

  showStatusBarMessage( tr( "Connecting to GPS device %1…" ).arg( port ) );

  QgsGpsDetector *detector = new QgsGpsDetector( port );
  connect( detector, static_cast < void ( QgsGpsDetector::* )( QgsGpsConnection * ) > ( &QgsGpsDetector::detected ), this, &QgsAppGpsConnection::onConnected );
  connect( detector, &QgsGpsDetector::detectionFailed, this, &QgsAppGpsConnection::onTimeOut );
  detector->advance();   // start the detection process
}

void QgsAppGpsConnection::disconnectGps()
{
  if ( mConnection )
  {
    QgsApplication::gpsConnectionRegistry()->unregisterConnection( mConnection );
    delete mConnection;
    mConnection = nullptr;

    emit disconnected();
    emit fixStatusChanged( Qgis::GpsFixStatus::NoData );

    showStatusBarMessage( tr( "Disconnected from GPS device." ) );
  }
}

void QgsAppGpsConnection::onTimeOut()
{
  mConnection = nullptr;
  emit connectionTimedOut();

  QgisApp::instance()->statusBarIface()->clearMessage();
  QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Failed to connect to GPS device." ) );
}

void QgsAppGpsConnection::onConnected( QgsGpsConnection *conn )
{
  mConnection = conn;
  connect( mConnection, &QgsGpsConnection::stateChanged, this, &QgsAppGpsConnection::stateChanged );
  connect( mConnection, &QgsGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsConnection::nmeaSentenceReceived );
  connect( mConnection, &QgsGpsConnection::fixStatusChanged, this, &QgsAppGpsConnection::fixStatusChanged );
  connect( mConnection, &QgsGpsConnection::positionChanged, this, &QgsAppGpsConnection::positionChanged );

  Qgis::GnssConstellation constellation = Qgis::GnssConstellation::Unknown;
  // emit signals so initial fix status is correctly advertised
  emit fixStatusChanged( mConnection->currentGPSInformation().bestFixStatus( constellation ) );

  //insert connection into registry such that it can also be used by other dialogs or plugins
  QgsApplication::gpsConnectionRegistry()->registerConnection( mConnection );

  emit connected();
  showStatusBarMessage( tr( "Connected to GPS device." ) );
}

void QgsAppGpsConnection::showStatusBarMessage( const QString &msg )
{
  QgisApp::instance()->statusBarIface()->showMessage( msg );
}
