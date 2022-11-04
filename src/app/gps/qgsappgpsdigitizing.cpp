/***************************************************************************
    qgsappgpsdigitizing.cpp
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

#include "qgsappgpsdigitizing.h"
#include "qgsrubberband.h"
#include "qgslinesymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsgui.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "gmath.h"
#include "info.h"
#include "qgspolygon.h"
#include "qgsmapcanvas.h"
#include "qgsfeatureaction.h"
#include "qgsgpsconnection.h"
#include "qgsappgpsconnection.h"
#include "qgsprojectgpssettings.h"

#include <QTimeZone>

QgsAppGpsDigitizing::QgsAppGpsDigitizing( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent )
  : QObject( parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

  mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ]
  {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [ = ]
  {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );

  mDistanceCalculator.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mDistanceCalculator.setSourceCrs( mWgs84CRS, QgsProject::instance()->transformContext() );
  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [ = ]
  {
    mDistanceCalculator.setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  mLastNmeaPosition.lat = nmea_degree2radian( 0.0 );
  mLastNmeaPosition.lon = nmea_degree2radian( 0.0 );

  mAcquisitionTimer = std::unique_ptr<QTimer>( new QTimer( this ) );
  mAcquisitionTimer->setSingleShot( true );

  connect( mAcquisitionTimer.get(), &QTimer::timeout,
           this, &QgsAppGpsDigitizing::switchAcquisition );

  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::destinationLayerChanged,
           this, &QgsAppGpsDigitizing::updateTimestampDestinationFields );

  updateTimestampDestinationFields( QgsProject::instance()->gpsSettings()->destinationLayer() );

  connect( mConnection, &QgsAppGpsConnection::stateChanged, this, &QgsAppGpsDigitizing::gpsStateChanged );
  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsAppGpsDigitizing::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsAppGpsDigitizing::gpsDisconnected );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsAppGpsDigitizing::gpsSettingsChanged );
  gpsSettingsChanged();
}

QgsAppGpsDigitizing::~QgsAppGpsDigitizing()
{
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsAppGpsDigitizing::addVertex()
{
  if ( !mRubberBand )
  {
    createRubberBand();
  }

  // we store the capture list in wgs84 and then transform to layer crs when
  // calling close feature
  const QgsPoint pointWgs84 = QgsPoint( mLastGpsPositionWgs84.x(), mLastGpsPositionWgs84.y(), mLastElevation );

  const bool trackWasEmpty = mCaptureListWgs84.empty();
  mCaptureListWgs84.push_back( pointWgs84 );

  // we store the rubber band points in map canvas CRS so transform to map crs
  // potential problem with transform errors and wrong coordinates if map CRS is changed after points are stored - SLM
  // should catch map CRS change and transform the points
  QgsPointXY mapPoint;
  if ( mCanvas )
  {
    try
    {
      mapPoint = mCanvasToWgs84Transform.transform( mLastGpsPositionWgs84, Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform GPS location (%1, %2) to map CRS" ).arg( mLastGpsPositionWgs84.x() ).arg( mLastGpsPositionWgs84.y() ) );
      return;
    }
  }
  else
  {
    mapPoint = mLastGpsPositionWgs84;
  }

  mRubberBand->addPoint( mapPoint );

  if ( trackWasEmpty )
    emit trackIsEmptyChanged( false );
}

void QgsAppGpsDigitizing::resetTrack()
{
  mBlockGpsStateChanged++;
  createRubberBand(); //deletes existing rubberband

  const bool trackWasEmpty = mCaptureListWgs84.isEmpty();
  mCaptureListWgs84.clear();
  mBlockGpsStateChanged--;

  if ( !trackWasEmpty )
    emit trackIsEmptyChanged( true );
}

void QgsAppGpsDigitizing::createFeature()
{
  QgsVectorLayer *vlayer = QgsProject::instance()->gpsSettings()->destinationLayer();
  if ( !vlayer )
    return;

  if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry && mCaptureListWgs84.size() < 2 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "Creating a line feature requires a track with at least two vertices." ) );
    return;
  }
  else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry && mCaptureListWgs84.size() < 3 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
        tr( "Creating a polygon feature requires a track with at least three vertices." ) );
    return;
  }

  // Handle timestamp
  QgsAttributeMap attrMap;
  const int idx { vlayer->fields().indexOf( mTimestampField ) };
  if ( idx != -1 )
  {
    const QVariant ts = timestamp( vlayer, idx );
    if ( ts.isValid() )
    {
      attrMap[ idx ] = ts;
    }
  }

  const QgsCoordinateTransform wgs84ToLayerTransform( mWgs84CRS, vlayer->crs(), QgsProject::instance() );
  const bool is3D = QgsWkbTypes::hasZ( vlayer->wkbType() );
  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsFeature f;
      try
      {
        const QgsPointXY pointXYLayerCrs = wgs84ToLayerTransform.transform( mLastGpsPositionWgs84 );

        QgsGeometry g;
        if ( is3D )
          g = QgsGeometry( new QgsPoint( pointXYLayerCrs.x(), pointXYLayerCrs.y(), mLastElevation ) );
        else
          g = QgsGeometry::fromPointXY( pointXYLayerCrs );

        if ( QgsWkbTypes::isMultiType( vlayer->wkbType() ) )
          g.convertToMultiType();

        f.setGeometry( g );
      }
      catch ( QgsCsException & )
      {
        QgisApp::instance()->messageBar()->pushCritical( tr( "Add Feature" ),
            tr( "Error reprojecting feature to layer CRS." ) );
        return;
      }

      QgsFeatureAction action( tr( "Feature Added" ), f, vlayer, QUuid(), -1, this );
      if ( action.addFeature( attrMap ) )
      {
        if ( QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
        {
          // should canvas->isDrawing() be checked?
          if ( !vlayer->commitChanges() ) //assumed to be vector layer and is editable and is in editing mode (preconditions have been tested)
          {
            QgisApp::instance()->messageBar()->pushCritical(
              tr( "Save Layer Edits" ),
              tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
              .arg( vlayer->name(),
                    vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );
          }

          vlayer->startEditing();
        }
      }

      break;
    }

    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
    {
      mBlockGpsStateChanged++;

      QgsFeature f;
      QgsGeometry g;

      std::unique_ptr<QgsLineString> ringWgs84( new QgsLineString( mCaptureListWgs84 ) );
      if ( ! is3D )
        ringWgs84->dropZValue();

      if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry )
      {

        g = QgsGeometry( ringWgs84.release() );
        try
        {
          g.transform( wgs84ToLayerTransform );
        }
        catch ( QgsCsException & )
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
              tr( "Error reprojecting feature to layer CRS." ) );
          return;
        }
        if ( QgsWkbTypes::isMultiType( vlayer->wkbType() ) )
          g.convertToMultiType();
      }
      else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry )
      {
        ringWgs84->close();
        std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
        polygon->setExteriorRing( ringWgs84.release() );

        g = QgsGeometry( polygon.release() );
        try
        {
          g.transform( wgs84ToLayerTransform );
        }
        catch ( QgsCsException & )
        {
          mBlockGpsStateChanged--;
          QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
              tr( "Error reprojecting feature to layer CRS." ) );
          return;
        }

        if ( QgsWkbTypes::isMultiType( vlayer->wkbType() ) )
          g.convertToMultiType();

        const int avoidIntersectionsReturn = g.avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers() );
        if ( avoidIntersectionsReturn == 1 )
        {
          //not a polygon type. Impossible to get there
        }
        else if ( avoidIntersectionsReturn == 2 )
        {
          //bail out...
          QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "The feature could not be added because removing the polygon intersections would change the geometry type." ) );
          mBlockGpsStateChanged--;
          return;
        }
        else if ( avoidIntersectionsReturn == 3 )
        {
          QgisApp::instance()->messageBar()->pushCritical( tr( "Add Feature" ), tr( "The feature has been added, but at least one geometry intersected is invalid. These geometries must be manually repaired." ) );
          mBlockGpsStateChanged--;
          return;
        }
      }

      f.setGeometry( g );
      QgsFeatureAction action( tr( "Feature added" ), f, vlayer, QUuid(), -1, this );

      if ( action.addFeature( attrMap ) )
      {
        if ( QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
        {
          if ( !vlayer->commitChanges() )
          {
            QgisApp::instance()->messageBar()->pushCritical( tr( "Save Layer Edits" ),
                tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                .arg( vlayer->name(),
                      vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );
          }

          vlayer->startEditing();
        }
        delete mRubberBand;
        mRubberBand = nullptr;

        // delete the elements of mCaptureList
        mCaptureListWgs84.clear();
      } // action.addFeature()

      mBlockGpsStateChanged--;

      if ( mCaptureListWgs84.empty() )
        emit trackIsEmptyChanged( true );

      break;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return;
  }
  vlayer->triggerRepaint();

  QgisApp::instance()->activateWindow();
}

void QgsAppGpsDigitizing::setTimeStampDestination( const QString &fieldName )
{
  if ( QgsVectorLayer *vlayer = QgsProject::instance()->gpsSettings()->destinationLayer() )
  {
    mPreferredTimestampFields[ vlayer->id() ] = fieldName;
  }
  mTimestampField = fieldName;
}

void QgsAppGpsDigitizing::setNmeaLogFile( const QString &filename )
{
  if ( mLogFile )
  {
    stopLogging();
  }

  mNmeaLogFile = filename;

  if ( mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startLogging();
  }
}

void QgsAppGpsDigitizing::setNmeaLoggingEnabled( bool enabled )
{
  if ( enabled == static_cast< bool >( mLogFile ) )
    return;

  if ( mLogFile && !enabled )
  {
    stopLogging();
  }

  mEnableNmeaLogging = enabled;

  if ( mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startLogging();
  }
}

void QgsAppGpsDigitizing::gpsSettingsChanged()
{
  updateTrackAppearance();

  int acquisitionInterval = 0;
  if ( QgsGpsConnection::settingsGpsConnectionType.exists() )
  {
    acquisitionInterval = static_cast< int >( QgsGpsConnection::settingGpsAcquisitionInterval.value() );
    mDistanceThreshold = QgsGpsConnection::settingGpsDistanceThreshold.value();
    mApplyLeapSettings = QgsGpsConnection::settingGpsApplyLeapSecondsCorrection.value();
    mLeapSeconds = static_cast< int >( QgsGpsConnection::settingGpsLeapSeconds.value() );
    mTimeStampSpec = QgsGpsConnection::settingsGpsTimeStampSpecification.value();
    mTimeZone = QgsGpsConnection::settingsGpsTimeStampTimeZone.value();
  }
  else
  {
    // legacy settings
    QgsSettings settings;

    acquisitionInterval = settings.value( QStringLiteral( "acquisitionInterval" ), 0, QgsSettings::Gps ).toInt();
    mDistanceThreshold = settings.value( QStringLiteral( "distanceThreshold" ), 0, QgsSettings::Gps ).toDouble();
    mApplyLeapSettings = settings.value( QStringLiteral( "applyLeapSeconds" ), true, QgsSettings::Gps ).toBool();
    mLeapSeconds = settings.value( QStringLiteral( "leapSecondsCorrection" ), 18, QgsSettings::Gps ).toInt();

    switch ( settings.value( QStringLiteral( "timeStampFormat" ), Qt::LocalTime, QgsSettings::Gps ).toInt() )
    {
      case 0:
        mTimeStampSpec = Qt::TimeSpec::LocalTime;
        break;

      case 1:
        mTimeStampSpec = Qt::TimeSpec::UTC;
        break;

      case 2:
        mTimeStampSpec = Qt::TimeSpec::TimeZone;
        break;
    }
    mTimeZone = settings.value( QStringLiteral( "timestampTimeZone" ), QVariant(), QgsSettings::Gps ).toString();
  }

  mAcquisitionInterval = acquisitionInterval * 1000;
  if ( mAcquisitionTimer->isActive() )
    mAcquisitionTimer->stop();
  mAcquisitionEnabled = true;

  switchAcquisition();
}

void QgsAppGpsDigitizing::updateTrackAppearance()
{
  if ( !mRubberBand )
    return;

  QDomDocument doc;
  QDomElement elem;
  const QString trackLineSymbolXml = QgsAppGpsDigitizing::settingTrackLineSymbol.value();
  if ( !trackLineSymbolXml.isEmpty() )
  {
    doc.setContent( trackLineSymbolXml );
    elem = doc.documentElement();
    std::unique_ptr< QgsLineSymbol > trackLineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, QgsReadWriteContext() ) );
    if ( trackLineSymbol )
    {
      mRubberBand->setSymbol( trackLineSymbol.release() );
    }
    mRubberBand->update();
  }
}

void QgsAppGpsDigitizing::switchAcquisition()
{
  if ( mAcquisitionInterval > 0 )
  {
    if ( mAcquisitionEnabled )
      mAcquisitionTimer->start( mAcquisitionInterval );
    else
      //wait only acquisitionInterval/10 for new valid data
      mAcquisitionTimer->start( mAcquisitionInterval / 10 );
    // anyway switch to enabled / disabled acquisition
    mAcquisitionEnabled = !mAcquisitionEnabled;
  }
}

void QgsAppGpsDigitizing::gpsConnected()
{
  if ( !mLogFile && mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startLogging();
  }
}

void QgsAppGpsDigitizing::gpsDisconnected()
{
  stopLogging();
}

void QgsAppGpsDigitizing::gpsStateChanged( const QgsGpsInformation &info )
{
  if ( mBlockGpsStateChanged )
    return;

  const bool validFlag = info.isValid();
  QgsPointXY newLocationWgs84;
  nmeaPOS newNmeaPosition;
  nmeaTIME newNmeaTime;
  double newAlt = 0.0;
  if ( validFlag )
  {
    newLocationWgs84 = QgsPointXY( info.longitude, info.latitude );
    newNmeaPosition.lat = nmea_degree2radian( info.latitude );
    newNmeaPosition.lon = nmea_degree2radian( info.longitude );
    newAlt = info.elevation;
    nmea_time_now( &newNmeaTime );
    mLastNmeaTime = newNmeaTime;
  }
  else
  {
    newLocationWgs84 = mLastGpsPositionWgs84;
    newNmeaPosition = mLastNmeaPosition;
    newAlt = mLastElevation;
  }
  if ( !mAcquisitionEnabled || ( nmea_distance( &newNmeaPosition, &mLastNmeaPosition ) < mDistanceThreshold ) )
  {
    // do not update position if update is disabled by timer or distance is under threshold
    newLocationWgs84 = mLastGpsPositionWgs84;

  }
  if ( validFlag && mAcquisitionEnabled )
  {
    // position updated by valid data, reset timer
    switchAcquisition();
  }

  // Avoid refreshing / panning if we haven't moved
  if ( mLastGpsPositionWgs84 != newLocationWgs84 )
  {
    mLastGpsPositionWgs84 = newLocationWgs84;
    mLastNmeaPosition = newNmeaPosition;
    mLastElevation = newAlt;

    if ( QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() )
    {
      addVertex();
    }
  }
}

void QgsAppGpsDigitizing::updateTimestampDestinationFields( QgsVectorLayer *vlayer )
{
  if ( vlayer )
  {
    // Restore preferred if stored
    if ( mPreferredTimestampFields.contains( vlayer->id( ) ) )
    {
      const QString previousTimeStampField = mPreferredTimestampFields[ vlayer->id( ) ];
      const int idx = vlayer->fields().indexOf( previousTimeStampField );
      if ( idx >= 0 )
        mTimestampField = previousTimeStampField;
    }
    // Cleanup preferred fields
    const QStringList layerIds { mPreferredTimestampFields.keys( ) };
    for ( const QString &layerId : layerIds )
    {
      if ( ! QgsProject::instance()->mapLayer( layerId ) )
      {
        mPreferredTimestampFields.remove( layerId );
      }
    }
  }

  emit timeStampDestinationChanged( mTimestampField );
}

void QgsAppGpsDigitizing::logNmeaSentence( const QString &nmeaString )
{
  if ( mEnableNmeaLogging && mLogFile && mLogFile->isOpen() )
  {
    mLogFileTextStream << nmeaString << "\r\n"; // specifically output CR + LF (NMEA requirement)
  }
}

void QgsAppGpsDigitizing::startLogging()
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

    connect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsDigitizing::logNmeaSentence ); // added to handle raw data
  }
  else  // error opening file
  {
    mLogFile.reset();

    // need to indicate why - this just reports that an error occurred
    QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Error opening log file." ) );
  }
}

void QgsAppGpsDigitizing::stopLogging()
{
  if ( mLogFile && mLogFile->isOpen() )
  {
    disconnect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsAppGpsDigitizing::logNmeaSentence );
    mLogFile->close();
    mLogFile.reset();
  }
}

void QgsAppGpsDigitizing::createRubberBand()
{
  delete mRubberBand;

  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry );
  updateTrackAppearance();
  mRubberBand->show();
}

QVariant QgsAppGpsDigitizing::timestamp( QgsVectorLayer *vlayer, int idx )
{
  QVariant value;
  if ( idx != -1 )
  {
    QDateTime time( QDate( 1900 + mLastNmeaTime.year, mLastNmeaTime.mon + 1, mLastNmeaTime.day ),
                    QTime( mLastNmeaTime.hour, mLastNmeaTime.min, mLastNmeaTime.sec, mLastNmeaTime.msec ) );
    // Time from GPS is UTC time
    time.setTimeSpec( Qt::UTC );
    // Apply leap seconds correction
    if ( mApplyLeapSettings && mLeapSeconds != 0 )
    {
      time = time.addSecs( mLeapSeconds );
    }
    // Desired format
    time = time.toTimeSpec( mTimeStampSpec );
    if ( mTimeStampSpec == Qt::TimeSpec::TimeZone )
    {
      // Get timezone from the combo
      const QTimeZone destTz( mTimeZone.toUtf8() );
      if ( destTz.isValid() )
      {
        time = time.toTimeZone( destTz );
      }
    }
    else if ( mTimeStampSpec == Qt::TimeSpec::LocalTime )
    {
      time = time.toLocalTime();
    }
    else if ( mTimeStampSpec == Qt::TimeSpec::UTC )
    {
      // Do nothing: we are already in UTC
    }

    // Only string and datetime fields are supported
    switch ( vlayer->fields().at( idx ).type() )
    {
      case QVariant::String:
        value = time.toString( Qt::DateFormat::ISODate );
        break;
      case QVariant::DateTime:
        value = time;
        break;
      default:
        break;
    }
  }
  return value;
}
