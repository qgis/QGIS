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
#include "qgsvectorlayergpslogger.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

const std::vector< std::tuple< Qgis::GpsInformationComponent, std::tuple< QVariant::Type, QString >>> QgsAppGpsLogging::sPointFields
{
  { Qgis::GpsInformationComponent::Timestamp, { QVariant::DateTime, QStringLiteral( "timestamp" )}},
  { Qgis::GpsInformationComponent::Altitude, { QVariant::Double, QStringLiteral( "altitude" )}},
  { Qgis::GpsInformationComponent::EllipsoidAltitude, { QVariant::Double, QStringLiteral( "altitude_wgs84" )}},
  { Qgis::GpsInformationComponent::GroundSpeed, { QVariant::Double, QStringLiteral( "ground_speed" )}},
  { Qgis::GpsInformationComponent::Bearing, { QVariant::Double, QStringLiteral( "bearing" )}},
  { Qgis::GpsInformationComponent::Pdop, { QVariant::Double, QStringLiteral( "pdop" )}},
  { Qgis::GpsInformationComponent::Hdop, { QVariant::Double, QStringLiteral( "hdop" )}},
  { Qgis::GpsInformationComponent::Vdop, { QVariant::Double, QStringLiteral( "vdop" )}},
  { Qgis::GpsInformationComponent::HorizontalAccuracy, { QVariant::Double, QStringLiteral( "horizontal_accuracy" )}},
  { Qgis::GpsInformationComponent::VerticalAccuracy, { QVariant::Double, QStringLiteral( "vertical_accuracy" )}},
  { Qgis::GpsInformationComponent::HvAccuracy, { QVariant::Double, QStringLiteral( "hv_accuracy" )}},
  { Qgis::GpsInformationComponent::SatellitesUsed, { QVariant::Double, QStringLiteral( "satellites_used" )}},
  { Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint, { QVariant::Double, QStringLiteral( "distance_since_previous" )}},
  { Qgis::GpsInformationComponent::TrackTimeSinceLastPoint, { QVariant::Double, QStringLiteral( "time_since_previous" )}},
};

const std::vector< std::tuple< Qgis::GpsInformationComponent, std::tuple< QVariant::Type, QString >>> QgsAppGpsLogging::sTrackFields
{
  { Qgis::GpsInformationComponent::TrackStartTime, { QVariant::DateTime, QStringLiteral( "start_time" )}},
  { Qgis::GpsInformationComponent::TrackEndTime, { QVariant::DateTime, QStringLiteral( "end_time" )}},
  { Qgis::GpsInformationComponent::TotalTrackLength, { QVariant::Double, QStringLiteral( "track_length" )}},
  { Qgis::GpsInformationComponent::TrackDistanceFromStart, { QVariant::Double, QStringLiteral( "distance_from_start" )}},
};


QgsAppGpsLogging::QgsAppGpsLogging( QgsAppGpsConnection *connection, QObject *parent )
  : QObject( parent )
  , mConnection( connection )
{
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [ = ]
  {
    if ( mGpkgLogger )
      mGpkgLogger->setTransformContext( QgsProject::instance()->transformContext() );
  } );
  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [ = ]
  {
    if ( mGpkgLogger )
      mGpkgLogger->setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsAppGpsLogging::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsAppGpsLogging::gpsDisconnected );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [ = ]
  {
    if ( mGpkgLogger )
      mGpkgLogger->updateGpsSettings();
  } );
}

QgsAppGpsLogging::~QgsAppGpsLogging()
{
  if ( mGpkgLogger )
  {
    mGpkgLogger->endCurrentTrack();
  }
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
  mGpkgLogFile = filename;
  if ( filename.isEmpty() )
  {
    // stop logging
    if ( mGpkgLogger )
    {
      mGpkgLogger->endCurrentTrack();
      mGpkgLogger.reset();

      QgisApp::instance()->messageBar()->pushInfo( QString(), tr( "GPS logging stopped" ) );
    }
  }
  else
  {
    if ( !createOrUpdateLogDatabase() )
      return;
    createGpkgLogger();

    QgisApp::instance()->messageBar()->pushInfo( QString(), tr( "Saving GPS log to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( mGpkgLogFile ).toString(), QDir::toNativeSeparators( mGpkgLogFile ) ) );
  }
}

void QgsAppGpsLogging::gpsConnected()
{
  if ( !mLogFile && mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startNmeaLogging();
  }
  if ( mGpkgLogger )
  {
    mGpkgLogger->setConnection( mConnection->connection() );
  }
}

void QgsAppGpsLogging::gpsDisconnected()
{
  stopNmeaLogging();
  if ( mGpkgLogger )
  {
    mGpkgLogger->endCurrentTrack();
    mGpkgLogger->setConnection( nullptr );
  }
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
    if ( mLogFile->size() > 0 )
    {
      mLogFileTextStream << "====" << "\r\n";
    }

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

void QgsAppGpsLogging::createGpkgLogger()
{
  mGpkgLogger = std::make_unique< QgsVectorLayerGpsLogger >( mConnection->connection() );
  mGpkgLogger->setTransformContext( QgsProject::instance()->transformContext() );
  mGpkgLogger->setEllipsoid( QgsProject::instance()->ellipsoid() );
  mGpkgLogger->updateGpsSettings();
  // write direct to data provider, just in case the QGIS session is closed unexpectedly (because the laptop
  // battery ran out that is, not because we want to protect against QGIS crashes ;)
  mGpkgLogger->setWriteToEditBuffer( false );

  QVariantMap uriParts;
  uriParts.insert( QStringLiteral( "path" ), mGpkgLogFile );
  uriParts.insert( QStringLiteral( "layerName" ), QStringLiteral( "gps_points" ) );

  mGpkgPointsLayer = std::make_unique< QgsVectorLayer >( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), uriParts ) );
  if ( mGpkgPointsLayer->isValid() )
  {
    for ( const auto &it : sPointFields )
    {
      Qgis::GpsInformationComponent component;
      std::tuple< QVariant::Type, QString > fieldTypeToName;
      QVariant::Type fieldType;
      QString fieldName;
      std::tie( component, fieldTypeToName ) = it;
      std::tie( fieldType, fieldName ) = fieldTypeToName;

      const int fieldIndex = mGpkgPointsLayer->fields().lookupField( fieldName );
      if ( fieldIndex >= 0 )
      {
        mGpkgLogger->setDestinationField( component, fieldName );
      }
    }
    mGpkgLogger->setPointsLayer( mGpkgPointsLayer.get() );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushCritical( tr( "Log GPS Tracks" ), tr( "Could not load gps_points layer" ) );
    emit gpkgLoggingFailed();
    mGpkgPointsLayer.reset();
    return;
  }

  uriParts.insert( QStringLiteral( "layerName" ), QStringLiteral( "gps_tracks" ) );
  mGpkgTracksLayer = std::make_unique< QgsVectorLayer >( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), uriParts ) );
  if ( mGpkgTracksLayer->isValid() )
  {
    for ( const auto &it : sTrackFields )
    {
      Qgis::GpsInformationComponent component;
      std::tuple< QVariant::Type, QString > fieldTypeToName;
      QVariant::Type fieldType;
      QString fieldName;
      std::tie( component, fieldTypeToName ) = it;
      std::tie( fieldType, fieldName ) = fieldTypeToName;

      const int fieldIndex = mGpkgTracksLayer->fields().lookupField( fieldName );
      if ( fieldIndex >= 0 )
      {
        mGpkgLogger->setDestinationField( component, fieldName );
      }
    }
    mGpkgLogger->setTracksLayer( mGpkgTracksLayer.get() );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushCritical( tr( "Log GPS Tracks" ), tr( "Could not load gps_tracks layer" ) );
    emit gpkgLoggingFailed();
    mGpkgTracksLayer.reset();
    return;
  }

}

bool QgsAppGpsLogging::createOrUpdateLogDatabase()
{
  const QFileInfo fi( mGpkgLogFile );

  if ( QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) )
  {
    QString error;

    // if database doesn't already exist, create it
    bool newFile = false;
    if ( !QFile::exists( mGpkgLogFile ) )
    {
      if ( ! ogrMetadata->createDatabase( mGpkgLogFile, error ) )
      {
        QgisApp::instance()->messageBar()->pushCritical( tr( "Create GPS Log" ), tr( "Database creation failed: %1" ).arg( error ) );
        emit gpkgLoggingFailed();
        return false;
      }
      newFile = true;
    }

    // does gps_points layer already exist?
    bool createPointLayer = true;
    if ( !newFile )
    {
      std::unique_ptr< QgsVectorLayer > testLayer = std::make_unique< QgsVectorLayer>( ogrMetadata->encodeUri( {{QStringLiteral( "path" ), mGpkgLogFile }, {QStringLiteral( "layerName" ), QStringLiteral( "gps_points" )}} ), QString(), QStringLiteral( "ogr" ) );
      if ( testLayer->isValid() )
      {
        createPointLayer = false;
      }
    }

    QMap< int, int > unusedMap;
    QVariantMap options;
    options.insert( QStringLiteral( "driverName" ), QgsVectorFileWriter::driverForExtension( fi.suffix() ) );
    options.insert( QStringLiteral( "update" ), true );
    options.insert( QStringLiteral( "layerName" ), QStringLiteral( "gps_points" ) );
    if ( createPointLayer )
    {
      QgsFields pointFields;
      for ( const auto &it : sPointFields )
      {
        Qgis::GpsInformationComponent component;
        std::tuple< QVariant::Type, QString > fieldTypeToName;
        QVariant::Type fieldType;
        QString fieldName;
        std::tie( component, fieldTypeToName ) = it;
        std::tie( fieldType, fieldName ) = fieldTypeToName;
        pointFields.append( QgsField( fieldName, fieldType ) );
      }

      const Qgis::VectorExportResult result = ogrMetadata->createEmptyLayer( mGpkgLogFile,
                                              pointFields,
                                              QgsGpsLogger::settingsGpsStoreAttributeInMValues.value() ? QgsWkbTypes::PointZM : QgsWkbTypes::PointZ,
                                              QgsCoordinateReferenceSystem( "EPSG:4326" ),
                                              false, unusedMap, error, &options );
      if ( result != Qgis::VectorExportResult::Success )
      {
        QgisApp::instance()->messageBar()->pushCritical( tr( "Create GPS Log" ), tr( "Database creation failed: %1" ).arg( error ) );
        emit gpkgLoggingFailed();
        return false;
      }
    }

    options.insert( QStringLiteral( "layerName" ), QStringLiteral( "gps_tracks" ) );

    // does gps_tracks layer already exist?
    bool createTracksLayer = true;
    if ( !newFile )
    {
      std::unique_ptr< QgsVectorLayer > testLayer = std::make_unique< QgsVectorLayer>( ogrMetadata->encodeUri( {{QStringLiteral( "path" ), mGpkgLogFile }, {QStringLiteral( "layerName" ), QStringLiteral( "gps_tracks" )}} ), QString(), QStringLiteral( "ogr" ) );
      if ( testLayer->isValid() )
      {
        createTracksLayer = false;
      }
    }

    if ( createTracksLayer )
    {
      QgsFields tracksFields;
      for ( const auto &it : sTrackFields )
      {
        Qgis::GpsInformationComponent component;
        std::tuple< QVariant::Type, QString > fieldTypeToName;
        QVariant::Type fieldType;
        QString fieldName;
        std::tie( component, fieldTypeToName ) = it;
        std::tie( fieldType, fieldName ) = fieldTypeToName;
        tracksFields.append( QgsField( fieldName, fieldType ) );
      }

      const Qgis::VectorExportResult result = ogrMetadata->createEmptyLayer( mGpkgLogFile,
                                              tracksFields,
                                              QgsGpsLogger::settingsGpsStoreAttributeInMValues.value() ? QgsWkbTypes::LineStringZM : QgsWkbTypes::LineStringZ,
                                              QgsCoordinateReferenceSystem( "EPSG:4326" ),
                                              false, unusedMap, error, &options );
      if ( result != Qgis::VectorExportResult::Success )
      {
        QgisApp::instance()->messageBar()->pushCritical( tr( "Create GPS Log" ), tr( "Database creation failed: %1" ).arg( error ) );
        emit gpkgLoggingFailed();
        return false;
      }
    }
    return true;
  }
  return false;
}

