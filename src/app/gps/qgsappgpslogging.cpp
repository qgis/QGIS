/***************************************************************************
    qgsappgpslogging.cpp
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappgpslogging.h"
#include "qgsgui.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsgpsconnection.h"
#include "qgsappgpsconnection.h"

QgsAppGpsLogging::QgsAppGpsLogging( QgsAppGpsConnection *connection, QObject *parent )
  : QgsGpsLogger( nullptr, parent )
  , mConnection( connection )
{
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [ = ]
  {
    setTransformContext( QgsProject::instance()->transformContext() );
  } );
  setTransformContext( QgsProject::instance()->transformContext() );

  setEllipsoid( QgsProject::instance()->ellipsoid() );
  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [ = ]
  {
    setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsAppGpsLogging::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsAppGpsLogging::gpsDisconnected );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsAppGpsLogging::updateGpsSettings );
  updateGpsSettings();
}

QgsAppGpsLogging::~QgsAppGpsLogging()
{
}

void QgsAppGpsLogging::setNmeaLogFile( const QString &filename )
{
  if ( mLogFile )
  {
    stopNmeaLogging();
  }

  mNmeaLogFile = filename;

  if ( mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startNmeaLogging();
  }
}

void QgsAppGpsLogging::setNmeaLoggingEnabled( bool enabled )
{
  if ( enabled == static_cast< bool >( mLogFile ) )
    return;

  if ( mLogFile && !enabled )
  {
    stopNmeaLogging();
  }

  mEnableNmeaLogging = enabled;

  if ( mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startNmeaLogging();
  }
}

void QgsAppGpsLogging::setGpkgLogFile( const QString &filename )
{

}


void QgsAppGpsLogging::gpsConnected()
{
  if ( !mLogFile && mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startNmeaLogging();
  }
  setConnection( mConnection->connection() );
}

void QgsAppGpsLogging::gpsDisconnected()
{
  stopNmeaLogging();
  setConnection( nullptr );
}

void QgsAppGpsLogging::logNmeaSentence( const QString &nmeaString )
{
  if ( mEnableNmeaLogging && mLogFile && mLogFile->isOpen() )
  {
    mLogFileTextStream << nmeaString << "\r\n"; // specifically output CR + LF (NMEA requirement)
  }
}

void QgsAppGpsLogging::startNmeaLogging()
{
  if ( !mLogFile )
  {
    mLogFile = std::make_unique< QFile >( mNmeaLogFile );
  }

  if ( mLogFile->open( QIODevice::Append ) )  // open in binary and explicitly output CR + LF per NMEA
  {
    mLogFileTextStream.setDevice( mLogFile.get() );

    // crude way to separate chunks - use when manually editing file - NMEA parsers should discard
    mLogFileTextStream << "====" << "\r\n";

    connect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsLogging::logNmeaSentence ); // added to handle raw data
  }
  else  // error opening file
  {
    mLogFile.reset();

    // need to indicate why - this just reports that an error occurred
    QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Error opening log file." ) );
  }
}

void QgsAppGpsLogging::stopNmeaLogging()
{
  if ( mLogFile && mLogFile->isOpen() )
  {
    disconnect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsLogging::logNmeaSentence );
    mLogFile->close();
    mLogFile.reset();
  }
}

