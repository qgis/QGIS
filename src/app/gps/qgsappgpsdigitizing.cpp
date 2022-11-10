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
  : QgsGpsLogger( nullptr, parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ]
  {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [ = ]
  {
    setTransformContext( QgsProject::instance()->transformContext() );
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, transformContext() );
  } );
  setTransformContext( QgsProject::instance()->transformContext() );

  setEllipsoid( QgsProject::instance()->ellipsoid() );

  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [ = ]
  {
    setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsAppGpsDigitizing::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsAppGpsDigitizing::gpsDisconnected );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsAppGpsDigitizing::gpsSettingsChanged );
  gpsSettingsChanged();

  connect( QgisApp::instance(), &QgisApp::activeLayerChanged, this, [ = ]( QgsMapLayer * layer )
  {
    if ( QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() )
    {
      QgsProject::instance()->gpsSettings()->setDestinationLayer( qobject_cast< QgsVectorLayer *> ( layer ) );
    }
  } );
  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::destinationFollowsActiveLayerChanged, this, [ = ]( bool enabled )
  {
    if ( enabled )
    {
      QgsProject::instance()->gpsSettings()->setDestinationLayer( qobject_cast< QgsVectorLayer *> ( QgisApp::instance()->activeLayer() ) );
    }
  } );
  if ( QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() )
  {
    QgsProject::instance()->gpsSettings()->setDestinationLayer( qobject_cast< QgsVectorLayer *> ( QgisApp::instance()->activeLayer() ) );
  }

  setAutomaticallyAddTrackVertices( QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() );
  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::automaticallyAddTrackVerticesChanged, this, [ = ]( bool enabled )
  {
    setAutomaticallyAddTrackVertices( enabled );
  } );

  connect( this, &QgsGpsLogger::trackVertexAdded, this, &QgsAppGpsDigitizing::addVertex );
  connect( this, &QgsGpsLogger::trackReset, this, &QgsAppGpsDigitizing::onTrackReset );
}

QgsAppGpsDigitizing::~QgsAppGpsDigitizing()
{
  delete mRubberBand;
  mRubberBand = nullptr;
}

double QgsAppGpsDigitizing::totalTrackLength() const
{
  QVector<QgsPointXY> points;
  QgsGeometry::convertPointList( mCaptureListWgs84, points );
  return mDa.measureLine( points );
}

double QgsAppGpsDigitizing::trackDistanceFromStart() const
{
  if ( mCaptureListWgs84.empty() )
    return 0;

  return mDa.measureLine( { QgsPointXY( mCaptureListWgs84.constFirst() ), QgsPointXY( mCaptureListWgs84.constLast() )} );
}

const QgsDistanceArea &QgsAppGpsDigitizing::distanceArea() const
{
  return mDa;
}

void QgsAppGpsDigitizing::addVertex( const QgsPoint &wgs84Point )
{
  if ( !mRubberBand )
  {
    createRubberBand();
  }

  // we store the rubber band points in map canvas CRS so transform to map crs
  // potential problem with transform errors and wrong coordinates if map CRS is changed after points are stored - SLM
  // should catch map CRS change and transform the points
  QgsPointXY mapPoint;
  if ( mCanvas )
  {
    try
    {
      mapPoint = mCanvasToWgs84Transform.transform( wgs84Point, Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform GPS location (%1, %2) to map CRS" ).arg( wgs84Point.x() ).arg( wgs84Point.y() ) );
      return;
    }
  }
  else
  {
    mapPoint = wgs84Point;
  }

  mRubberBand->addPoint( mapPoint );
}

void QgsAppGpsDigitizing::onTrackReset()
{
  createRubberBand(); //deletes existing rubberband
}

void QgsAppGpsDigitizing::createFeature()
{
  QgsVectorLayer *vlayer = QgsProject::instance()->gpsSettings()->destinationLayer();
  if ( !vlayer )
    return;

  const QVector< QgsPoint > captureListWgs84 = currentTrack();
  if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry && captureListWgs84.size() < 2 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "Creating a line feature requires a track with at least two vertices." ) );
    return;
  }
  else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry && captureListWgs84.size() < 3 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
        tr( "Creating a polygon feature requires a track with at least three vertices." ) );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    if ( vlayer->startEditing() )
    {
      if ( !QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
      {
        QgisApp::instance()->messageBar()->pushInfo( tr( "Add Feature" ), tr( "Layer “%1” was made editable" ).arg( vlayer->name() ) );
      }
    }
    else
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "Cannot create feature — the layer “%2” could not be made editable" ).arg( vlayer->name() ) );
      return;
    }
  }

  // Handle timestamp
  QgsAttributeMap attrMap;
  const int idx = vlayer->fields().indexOf( QgsProject::instance()->gpsSettings()->destinationTimeStampField() );
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
        const QgsPointXY pointXYLayerCrs = wgs84ToLayerTransform.transform( lastPosition() );

        QgsGeometry g;
        if ( is3D )
          g = QgsGeometry( new QgsPoint( pointXYLayerCrs.x(), pointXYLayerCrs.y(), lastElevation() ) );
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
      else
      {
        QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Could not create new feature in layer %1" ).arg( vlayer->name() ) );
      }

      break;
    }

    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
    {
      mBlockGpsStateChanged++;

      QgsFeature f;
      QgsGeometry g;

      std::unique_ptr<QgsLineString> ringWgs84( new QgsLineString( captureListWgs84 ) );
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
        resetTrack();
      }
      else
      {
        QgisApp::instance()->messageBar()->pushCritical( QString(), tr( "Could not create new feature in layer %1" ).arg( vlayer->name() ) );
      }

      mBlockGpsStateChanged--;

      break;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return;
  }
  vlayer->triggerRepaint();

  QgisApp::instance()->activateWindow();
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

void QgsAppGpsDigitizing::createVertexAtCurrentLocation()
{
  addTrackVertex();
}

void QgsAppGpsDigitizing::gpsSettingsChanged()
{
  updateTrackAppearance();

  updateGpsSettings();
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

void QgsAppGpsDigitizing::gpsConnected()
{
  if ( !mLogFile && mEnableNmeaLogging && !mNmeaLogFile.isEmpty() )
  {
    startLogging();
  }
  setConnection( mConnection->connection() );
}

void QgsAppGpsDigitizing::gpsDisconnected()
{
  stopLogging();
  setConnection( nullptr );
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
  const QDateTime timestamp = lastTimestamp();
  QVariant value;
  if ( idx != -1 && timestamp.isValid() )
  {
    // Only string and datetime fields are supported
    switch ( vlayer->fields().at( idx ).type() )
    {
      case QVariant::String:
        value = timestamp.toString( Qt::DateFormat::ISODate );
        break;
      case QVariant::DateTime:
        value = timestamp;
        break;
      default:
        break;
    }
  }
  return value;
}
