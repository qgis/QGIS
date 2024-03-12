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
#include "qgsmessagebaritem.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

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

  setConnectionPrivate( connection );
}

QgsPoint QgsAppGpsConnection::lastValidLocation() const
{
  if ( mConnection )
    return mConnection->lastValidLocation();
  else
    return QgsPoint();
}

QgsGpsInformation QgsAppGpsConnection::lastInformation() const
{
  if ( mConnection )
    return mConnection->currentGPSInformation();
  else
    return QgsGpsInformation();
}

void QgsAppGpsConnection::connectGps()
{
  QString port;

  Qgis::GpsConnectionType connectionType = Qgis::GpsConnectionType::Automatic;
  QString gpsdHost;
  int gpsdPort = 0;
  QString gpsdDevice;
  QString serialDevice;
  if ( QgsGpsConnection::settingsGpsConnectionType->exists() )
  {
    connectionType = QgsGpsConnection::settingsGpsConnectionType->value();
    gpsdHost = QgsGpsConnection::settingsGpsdHostName->value();
    gpsdPort = static_cast< int >( QgsGpsConnection::settingsGpsdPortNumber->value() );
    gpsdDevice = QgsGpsConnection::settingsGpsdDeviceName->value();
    serialDevice = QgsGpsConnection::settingsGpsSerialDevice->value();
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
      port = QgsGpsConnection::settingsGpsSerialDevice->value();
      if ( port.isEmpty() )
      {
        QgisApp::instance()->statusBarIface()->clearMessage();
        showGpsConnectFailureWarning( tr( "No path to the GPS port is specified. Please set a path then try again." ) );
        emit connectionError( tr( "No path to the GPS port is specified. Please set a path then try again." ) );
        emit statusChanged( Qgis::DeviceConnectionStatus::Disconnected );
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
  emit statusChanged( Qgis::DeviceConnectionStatus::Connecting );
  emit fixStatusChanged( Qgis::GpsFixStatus::NoData );

  QgisApp::instance()->statusBarIface()->clearMessage();
  showStatusBarMessage( tr( "Connecting to GPS device %1…" ).arg( port ) );

  QgsDebugMsgLevel( QStringLiteral( "Firing up GPS detector" ), 2 );

  // note -- QgsGpsDetector internally uses deleteLater to clean itself up!
  mDetector = new QgsGpsDetector( port, false );
  connect( mDetector, &QgsGpsDetector::connectionDetected, this, &QgsAppGpsConnection::onConnectionDetected );
  connect( mDetector, &QgsGpsDetector::detectionFailed, this, &QgsAppGpsConnection::onTimeOut );
  mDetector->advance();   // start the detection process
}

void QgsAppGpsConnection::disconnectGps()
{
  // we don't actually delete the connection until everything has had time to respond to the cleanup signals
  std::unique_ptr< QgsGpsConnection > oldConnection( mConnection );
  mConnection = nullptr;

  emit disconnected();
  emit statusChanged( Qgis::DeviceConnectionStatus::Disconnected );
  emit fixStatusChanged( Qgis::GpsFixStatus::NoData );

  QgisApp::instance()->statusBarIface()->clearMessage();
  showStatusBarMessage( tr( "Disconnected from GPS device." ) );

  if ( oldConnection )
    QgsApplication::gpsConnectionRegistry()->unregisterConnection( oldConnection.get() );
}

void QgsAppGpsConnection::onTimeOut()
{
  if ( sender() != mDetector )
    return;

  QgsDebugMsgLevel( QStringLiteral( "GPS detector reported timeout" ), 2 );
  disconnectGps();
  emit connectionTimedOut();

  QgisApp::instance()->statusBarIface()->clearMessage();
  showGpsConnectFailureWarning( tr( "TIMEOUT - Failed to connect to GPS device." ) );
}

void QgsAppGpsConnection::onConnectionDetected()
{
  if ( sender() != mDetector )
    return;

  QgsDebugMsgLevel( QStringLiteral( "GPS detector GOT a connection" ), 2 );
  setConnectionPrivate( mDetector->takeConnection() );
}

void QgsAppGpsConnection::setConnectionPrivate( QgsGpsConnection *connection )
{
  mConnection = connection;
  connect( mConnection, &QgsGpsConnection::stateChanged, this, &QgsAppGpsConnection::stateChanged );
  connect( mConnection, &QgsGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsConnection::nmeaSentenceReceived );
  connect( mConnection, &QgsGpsConnection::fixStatusChanged, this, &QgsAppGpsConnection::fixStatusChanged );
  connect( mConnection, &QgsGpsConnection::positionChanged, this, &QgsAppGpsConnection::positionChanged );

  Qgis::GnssConstellation constellation = Qgis::GnssConstellation::Unknown;
  // emit signals so initial fix status is correctly advertised
  emit stateChanged( mConnection->currentGPSInformation() );
  emit fixStatusChanged( mConnection->currentGPSInformation().bestFixStatus( constellation ) );
  emit positionChanged( mConnection->lastValidLocation() );

  //insert connection into registry such that it can also be used by other dialogs or plugins
  QgsApplication::gpsConnectionRegistry()->registerConnection( mConnection );

  emit connected();
  emit statusChanged( Qgis::DeviceConnectionStatus::Connected );
  showMessage( Qgis::MessageLevel::Success, tr( "Connected to GPS device." ) );
}

void QgsAppGpsConnection::showStatusBarMessage( const QString &msg )
{
  QgisApp::instance()->statusBarIface()->showMessage( msg );
}

void QgsAppGpsConnection::showGpsConnectFailureWarning( const QString &message )
{
  if ( mConnectionMessageItem )
  {
    // delete old connection message item, so that we don't stack up multiple outdated connection
    // related messages
    QgisApp::instance()->messageBar()->popWidget( mConnectionMessageItem );
    mConnectionMessageItem = nullptr;
  }

  QgisApp::instance()->statusBarIface()->clearMessage();
  mConnectionMessageItem = QgisApp::instance()->messageBar()->createMessage( QString(), message );
  QPushButton *configureButton = new QPushButton( tr( "Configure Device…" ) );
  connect( configureButton, &QPushButton::clicked, configureButton, [ = ]
  {
    QgisApp::instance()->showOptionsDialog( QgisApp::instance(), QStringLiteral( "mGpsOptions" ) );
  } );
  mConnectionMessageItem->layout()->addWidget( configureButton );
  QgisApp::instance()->messageBar()->pushWidget( mConnectionMessageItem, Qgis::MessageLevel::Critical );
}

void QgsAppGpsConnection::showMessage( Qgis::MessageLevel level, const QString &message )
{
  if ( mConnectionMessageItem )
  {
    // delete old connection message item, so that we don't stack up multiple outdated connection
    // related messages
    QgisApp::instance()->messageBar()->popWidget( mConnectionMessageItem );
    mConnectionMessageItem = nullptr;
  }

  QgisApp::instance()->statusBarIface()->clearMessage();

  mConnectionMessageItem = QgisApp::instance()->messageBar()->createMessage( QString(), message );
  QgisApp::instance()->messageBar()->pushWidget( mConnectionMessageItem, level, QgsMessageBar::defaultMessageTimeout( level ) );
}

