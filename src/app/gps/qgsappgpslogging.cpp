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

#include "qgisapp.h"
#include "qgsappgpsconnection.h"
#include "qgsgpsconnection.h"
#include "qgsgui.h"
#include "qgsmessagebar.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsvectorlayergpslogger.h"

#include "moc_qgsappgpslogging.cpp"

const QgsSettingsEntryString *QgsAppGpsLogging::settingLastLogFolder = new QgsSettingsEntryString( u"last-log-folder"_s, QgsSettingsTree::sTreeGps, QString(), u"Last used folder for GPS log files"_s );

const QgsSettingsEntryString *QgsAppGpsLogging::settingLastGpkgLog = new QgsSettingsEntryString( u"last-gpkg-log"_s, QgsSettingsTree::sTreeGps, QString(), u"Last used Geopackage/Spatialite file for logging GPS locations"_s );

const std::vector<std::tuple<Qgis::GpsInformationComponent, std::tuple<QMetaType::Type, QString>>> QgsAppGpsLogging::sPointFields {
  { Qgis::GpsInformationComponent::Timestamp, { QMetaType::Type::QDateTime, u"timestamp"_s } },
  { Qgis::GpsInformationComponent::Altitude, { QMetaType::Type::Double, u"altitude"_s } },
  { Qgis::GpsInformationComponent::EllipsoidAltitude, { QMetaType::Type::Double, u"altitude_wgs84"_s } },
  { Qgis::GpsInformationComponent::GroundSpeed, { QMetaType::Type::Double, u"ground_speed"_s } },
  { Qgis::GpsInformationComponent::Bearing, { QMetaType::Type::Double, u"bearing"_s } },
  { Qgis::GpsInformationComponent::Pdop, { QMetaType::Type::Double, u"pdop"_s } },
  { Qgis::GpsInformationComponent::Hdop, { QMetaType::Type::Double, u"hdop"_s } },
  { Qgis::GpsInformationComponent::Vdop, { QMetaType::Type::Double, u"vdop"_s } },
  { Qgis::GpsInformationComponent::HorizontalAccuracy, { QMetaType::Type::Double, u"horizontal_accuracy"_s } },
  { Qgis::GpsInformationComponent::VerticalAccuracy, { QMetaType::Type::Double, u"vertical_accuracy"_s } },
  { Qgis::GpsInformationComponent::HvAccuracy, { QMetaType::Type::Double, u"hv_accuracy"_s } },
  { Qgis::GpsInformationComponent::SatellitesUsed, { QMetaType::Type::Double, u"satellites_used"_s } },
  { Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint, { QMetaType::Type::Double, u"distance_since_previous"_s } },
  { Qgis::GpsInformationComponent::TrackTimeSinceLastPoint, { QMetaType::Type::Double, u"time_since_previous"_s } },
};

const std::vector<std::tuple<Qgis::GpsInformationComponent, std::tuple<QMetaType::Type, QString>>> QgsAppGpsLogging::sTrackFields {
  { Qgis::GpsInformationComponent::TrackStartTime, { QMetaType::Type::QDateTime, u"start_time"_s } },
  { Qgis::GpsInformationComponent::TrackEndTime, { QMetaType::Type::QDateTime, u"end_time"_s } },
  { Qgis::GpsInformationComponent::TotalTrackLength, { QMetaType::Type::Double, u"track_length"_s } },
  { Qgis::GpsInformationComponent::TrackDistanceFromStart, { QMetaType::Type::Double, u"distance_from_start"_s } },
};


QgsAppGpsLogging::QgsAppGpsLogging( QgsAppGpsConnection *connection, QObject *parent )
  : QObject( parent )
  , mConnection( connection )
{
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [this] {
    if ( mGpkgLogger )
      mGpkgLogger->setTransformContext( QgsProject::instance()->transformContext() );
  } );
  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [this] {
    if ( mGpkgLogger )
      mGpkgLogger->setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsAppGpsLogging::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsAppGpsLogging::gpsDisconnected );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, [this] {
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
  if ( enabled == static_cast<bool>( mLogFile ) )
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
      mGpkgTracksLayer.reset();
      mGpkgPointsLayer.reset();

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
  if ( !mGpkgLogFile.isEmpty() )
  {
    setGpkgLogFile( mGpkgLogFile );
  }
}

void QgsAppGpsLogging::gpsDisconnected()
{
  stopNmeaLogging();
  if ( mGpkgLogger )
  {
    mGpkgLogger->endCurrentTrack();
    mGpkgLogger.reset();
    mGpkgTracksLayer.reset();
    mGpkgPointsLayer.reset();
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
    mLogFile = std::make_unique<QFile>( mNmeaLogFile );
  }

  if ( mLogFile->open( QIODevice::Append ) ) // open in binary and explicitly output CR + LF per NMEA
  {
    mLogFileTextStream.setDevice( mLogFile.get() );

    // crude way to separate chunks - use when manually editing file - NMEA parsers should discard
    if ( mLogFile->size() > 0 )
    {
      mLogFileTextStream << "====" << "\r\n";
    }

    connect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsLogging::logNmeaSentence ); // added to handle raw data
  }
  else // error opening file
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
  mGpkgLogger = std::make_unique<QgsVectorLayerGpsLogger>( mConnection->connection() );
  mGpkgLogger->setTransformContext( QgsProject::instance()->transformContext() );
  mGpkgLogger->setEllipsoid( QgsProject::instance()->ellipsoid() );
  mGpkgLogger->updateGpsSettings();
  // write direct to data provider, just in case the QGIS session is closed unexpectedly (because the laptop
  // battery ran out that is, not because we want to protect against QGIS crashes ;)
  mGpkgLogger->setWriteToEditBuffer( false );

  QVariantMap uriParts;
  uriParts.insert( u"path"_s, mGpkgLogFile );
  uriParts.insert( u"layerName"_s, u"gps_points"_s );

  mGpkgPointsLayer = std::make_unique<QgsVectorLayer>( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, uriParts ) );
  if ( mGpkgPointsLayer->isValid() )
  {
    for ( const auto &it : sPointFields )
    {
      Qgis::GpsInformationComponent component;
      std::tuple<QMetaType::Type, QString> fieldTypeToName;
      QMetaType::Type fieldType;
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

  uriParts.insert( u"layerName"_s, u"gps_tracks"_s );
  mGpkgTracksLayer = std::make_unique<QgsVectorLayer>( QgsProviderRegistry::instance()->encodeUri( u"ogr"_s, uriParts ) );
  if ( mGpkgTracksLayer->isValid() )
  {
    for ( const auto &it : sTrackFields )
    {
      Qgis::GpsInformationComponent component;
      std::tuple<QMetaType::Type, QString> fieldTypeToName;
      QMetaType::Type fieldType;
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

  if ( QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) )
  {
    QString error;

    // if database doesn't already exist, create it
    bool newFile = false;
    if ( !QFile::exists( mGpkgLogFile ) )
    {
      if ( !ogrMetadata->createDatabase( mGpkgLogFile, error ) )
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
      auto testLayer = std::make_unique<QgsVectorLayer>( ogrMetadata->encodeUri( { { u"path"_s, mGpkgLogFile }, { u"layerName"_s, u"gps_points"_s } } ), QString(), u"ogr"_s );
      if ( testLayer->isValid() )
      {
        createPointLayer = false;
      }
    }

    QMap<int, int> unusedMap;
    QVariantMap options;
    options.insert( u"driverName"_s, QgsVectorFileWriter::driverForExtension( fi.suffix() ) );
    options.insert( u"update"_s, true );
    options.insert( u"layerName"_s, u"gps_points"_s );
    if ( createPointLayer )
    {
      QgsFields pointFields;
      for ( const auto &it : sPointFields )
      {
        Qgis::GpsInformationComponent component;
        std::tuple<QMetaType::Type, QString> fieldTypeToName;
        QMetaType::Type fieldType;
        QString fieldName;
        std::tie( component, fieldTypeToName ) = it;
        std::tie( fieldType, fieldName ) = fieldTypeToName;
        pointFields.append( QgsField( fieldName, fieldType ) );
      }

      QString createdLayerUri;
      const Qgis::VectorExportResult result = ogrMetadata->createEmptyLayer( mGpkgLogFile, pointFields, QgsGpsLogger::settingsGpsStoreAttributeInMValues->value() ? Qgis::WkbType::PointZM : Qgis::WkbType::PointZ, QgsCoordinateReferenceSystem( "EPSG:4326" ), false, unusedMap, error, &options, createdLayerUri );
      if ( result != Qgis::VectorExportResult::Success )
      {
        QgisApp::instance()->messageBar()->pushCritical( tr( "Create GPS Log" ), tr( "Database creation failed: %1" ).arg( error ) );
        emit gpkgLoggingFailed();
        return false;
      }
    }

    options.insert( u"layerName"_s, u"gps_tracks"_s );

    // does gps_tracks layer already exist?
    bool createTracksLayer = true;
    if ( !newFile )
    {
      auto testLayer = std::make_unique<QgsVectorLayer>( ogrMetadata->encodeUri( { { u"path"_s, mGpkgLogFile }, { u"layerName"_s, u"gps_tracks"_s } } ), QString(), u"ogr"_s );
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
        std::tuple<QMetaType::Type, QString> fieldTypeToName;
        QMetaType::Type fieldType;
        QString fieldName;
        std::tie( component, fieldTypeToName ) = it;
        std::tie( fieldType, fieldName ) = fieldTypeToName;
        tracksFields.append( QgsField( fieldName, fieldType ) );
      }

      QString createdLayerUri;
      const Qgis::VectorExportResult result = ogrMetadata->createEmptyLayer( mGpkgLogFile, tracksFields, QgsGpsLogger::settingsGpsStoreAttributeInMValues->value() ? Qgis::WkbType::LineStringZM : Qgis::WkbType::LineStringZ, QgsCoordinateReferenceSystem( "EPSG:4326" ), false, unusedMap, error, &options, createdLayerUri );
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
