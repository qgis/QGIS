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
#include "qgsgpsinformationwidget.h"
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

#include <QTimeZone>

QgsAppGpsDigitizing::QgsAppGpsDigitizing( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent )
  : QObject( parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsAppGpsDigitizing::gpsSettingsChanged );
  gpsSettingsChanged();

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
  const QgsPoint point = QgsPoint( mLastGpsPosition.x(), mLastGpsPosition.y(), mLastElevation );
  mCaptureList.push_back( point );


  // we store the rubber band points in map canvas CRS so transform to map crs
  // potential problem with transform errors and wrong coordinates if map CRS is changed after points are stored - SLM
  // should catch map CRS change and transform the points
  QgsPointXY myPoint;
  if ( mCanvas )
  {
    myPoint = mCanvasToWgs84Transform.transform( mLastGpsPosition, Qgis::TransformDirection::Reverse );
  }
  else
  {
    myPoint = mLastGpsPosition;
  }

  mRubberBand->addPoint( myPoint );
}

void QgsAppGpsDigitizing::resetFeature()
{
  mBlockGpsStateChanged++;
  createRubberBand(); //deletes existing rubberband
  mCaptureList.clear();
  mBlockGpsStateChanged--;
}

void QgsAppGpsDigitizing::closeFeature()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
    return;

  if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry && mCaptureList.size() < 2 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "Cannot close a line feature until it has at least two vertices." ) );
    return;
  }
  else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry && mCaptureList.size() < 3 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
        tr( "Cannot close a polygon feature until it has at least three vertices." ) );
    return;
  }

  // Handle timestamp
  QgsAttributeMap attrMap;
  const int idx { vlayer->fields().indexOf( mCboTimestampField->currentText() ) };
  if ( idx != -1 )
  {
    const QVariant ts = timestamp( vlayer, idx );
    if ( ts.isValid() )
    {
      attrMap[ idx ] = ts;
    }
  }

  const QgsCoordinateTransform t( mWgs84CRS, vlayer->crs(), QgsProject::instance() );
  const bool is3D = QgsWkbTypes::hasZ( vlayer->wkbType() );
  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsFeature f;
      try
      {
        const QgsPointXY pointXY = t.transform( mLastGpsPosition );

        QgsGeometry g;
        if ( is3D )
          g = QgsGeometry( new QgsPoint( pointXY.x(), pointXY.y(), mLastElevation ) );
        else
          g = QgsGeometry::fromPointXY( pointXY );

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
        if ( mCbxAutoCommit->isChecked() )
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

      std::unique_ptr<QgsLineString> ring( new QgsLineString( mCaptureList ) );
      if ( ! is3D )
        ring->dropZValue();

      if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry )
      {

        g = QgsGeometry( ring.release() );
        try
        {
          g.transform( t );
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
        ring->close();
        std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
        polygon->setExteriorRing( ring.release() );

        g = QgsGeometry( polygon.release() );
        try
        {
          g.transform( t );
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
        if ( mCbxAutoCommit->isChecked() )
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
        mCaptureList.clear();
      } // action.addFeature()

      mBlockGpsStateChanged--;
      break;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return;
  }
  vlayer->triggerRepaint();

  // force focus back to GPS window/ Add Feature button for ease of use by keyboard
  QgisApp::instance()->activateWindow();
  mBtnCloseFeature->setFocus( Qt::OtherFocusReason );
}

void QgsAppGpsDigitizing::gpsSettingsChanged()
{
  updateTrackAppearance();

  QgsSettings settings;
  int acquisitionInterval = 0;
  if ( QgsGpsConnection::settingsGpsConnectionType.exists() )
  {
    acquisitionInterval = static_cast< int >( QgsGpsConnection::settingGpsAcquisitionInterval.value() );
    mDistanceThreshold = QgsGpsConnection::settingGpsDistanceThreshold.value();
  }
  else
  {
    // legacy settings
    acquisitionInterval = settings.value( QStringLiteral( "acquisitionInterval" ), 0, QgsSettings::Gps ).toInt();
    mDistanceThreshold = settings.value( QStringLiteral( "distanceThreshold" ), 0, QgsSettings::Gps ).toDouble();
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
  const QString trackLineSymbolXml = QgsGpsInformationWidget::settingTrackLineSymbol.value();
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

void QgsAppGpsDigitizing::gpsStateChanged( const QgsGpsInformation &info )
{
  if ( mBlockGpsStateChanged )
    return;

  QVector<QPointF> data;

  const bool validFlag = info.isValid();
  QgsPointXY myNewCenter;
  nmeaPOS newNmeaPosition;
  nmeaTIME newNmeaTime;
  double newAlt = 0.0;
  if ( validFlag )
  {
    myNewCenter = QgsPointXY( info.longitude, info.latitude );
    newNmeaPosition.lat = nmea_degree2radian( info.latitude );
    newNmeaPosition.lon = nmea_degree2radian( info.longitude );
    newAlt = info.elevation;
    nmea_time_now( &newNmeaTime );
  }
  else
  {
    myNewCenter = mLastGpsPosition;
    newNmeaPosition = mLastNmeaPosition;
    newAlt = mLastElevation;
  }
  if ( !mAcquisitionEnabled || ( nmea_distance( &newNmeaPosition, &mLastNmeaPosition ) < mDistanceThreshold ) )
  {
    // do not update position if update is disabled by timer or distance is under threshold
    myNewCenter = mLastGpsPosition;

  }
  if ( validFlag && mAcquisitionEnabled )
  {
    // position updated by valid data, reset timer
    switchAcquisition();
  }

  // Avoid refreshing / panning if we haven't moved
  if ( mLastGpsPosition != myNewCenter )
  {
    mLastGpsPosition = myNewCenter;
    mLastNmeaPosition = newNmeaPosition;
    mLastNmeaTime = newNmeaTime;
    mLastElevation = newAlt;

    if ( mCbxAutoAddVertices->isChecked() )
    {
      addVertex();
    }
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
    if ( mCbxLeapSeconds->isChecked() && mLeapSeconds->value() != 0 )
    {
      time = time.addSecs( mLeapSeconds->value() );
    }
    // Desired format
    const Qt::TimeSpec timeSpec { static_cast<Qt::TimeSpec>( mCboTimestampFormat->currentData( ).toInt() ) };
    time = time.toTimeSpec( timeSpec );
    if ( timeSpec == Qt::TimeSpec::TimeZone )
    {
      // Get timezone from the combo
      const QTimeZone destTz( mCboTimeZones->currentText().toUtf8() );
      if ( destTz.isValid() )
      {
        time = time.toTimeZone( destTz );
      }
    }
    else if ( timeSpec == Qt::TimeSpec::LocalTime )
    {
      time = time.toLocalTime();
    }
    else if ( timeSpec == Qt::TimeSpec::UTC )
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
