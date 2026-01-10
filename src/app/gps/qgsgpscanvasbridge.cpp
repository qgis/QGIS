/***************************************************************************
    qgsgpscanvasbridge.cpp
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

#include "qgsgpscanvasbridge.h"

#include "qgisapp.h"
#include "qgsappgpsconnection.h"
#include "qgsbearingnumericformat.h"
#include "qgsbearingutils.h"
#include "qgsgpsbearingitem.h"
#include "qgsgpsconnection.h"
#include "qgsgpsmarker.h"
#include "qgsgui.h"
#include "qgslinesymbol.h"
#include "qgslocaldefaultsettings.h"
#include "qgsmapcanvas.h"
#include "qgsprojectdisplaysettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsstatusbar.h"
#include "qgssymbollayerutils.h"

#include "moc_qgsgpscanvasbridge.cpp"

const QgsSettingsEntryBool *QgsGpsCanvasBridge::settingShowBearingLine = new QgsSettingsEntryBool( u"show-bearing-line"_s, QgsSettingsTree::sTreeGps, false, u"Whether the GPS bearing line symbol should be shown"_s );

const QgsSettingsEntryString *QgsGpsCanvasBridge::settingBearingLineSymbol = new QgsSettingsEntryString( u"bearing-line-symbol"_s, QgsSettingsTree::sTreeGps, QString(), u"Line symbol to use for GPS bearing line"_s, Qgis::SettingsOptions(), 0 );

const QgsSettingsEntryInteger *QgsGpsCanvasBridge::settingMapExtentRecenteringThreshold = new QgsSettingsEntryInteger( u"map-recentering-threshold"_s, QgsSettingsTree::sTreeGps, 50, u"Threshold for GPS automatic map centering"_s );

const QgsSettingsEntryEnumFlag<Qgis::MapRecenteringMode> *QgsGpsCanvasBridge::settingMapCenteringMode = new QgsSettingsEntryEnumFlag<Qgis::MapRecenteringMode>( u"map-recentering"_s, QgsSettingsTree::sTreeGps, Qgis::MapRecenteringMode::WhenOutsideVisibleExtent, u"Automatic GPS based map recentering mode"_s );

const QgsSettingsEntryBool *QgsGpsCanvasBridge::settingRotateMap = new QgsSettingsEntryBool( u"auto-map-rotate"_s, QgsSettingsTree::sTreeGps, false, u"Whether to automatically rotate the map to match GPS bearing"_s );

const QgsSettingsEntryInteger *QgsGpsCanvasBridge::settingMapRotateInterval = new QgsSettingsEntryInteger( u"map-rotate-interval"_s, QgsSettingsTree::sTreeGps, 0, u"Interval for GPS automatic map rotation"_s );

QgsGpsCanvasBridge::QgsGpsCanvasBridge( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QObject *parent )
  : QObject( parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"EPSG:4326"_s );

  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsGpsCanvasBridge::gpsDisconnected );
  connect( mConnection, &QgsAppGpsConnection::stateChanged, this, &QgsGpsCanvasBridge::gpsStateChanged );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsGpsCanvasBridge::gpsSettingsChanged );

  mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, [this] {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [this] {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );

  mDistanceCalculator.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mDistanceCalculator.setSourceCrs( mWgs84CRS, QgsProject::instance()->transformContext() );
  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [this] {
    mDistanceCalculator.setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  connect( mCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsGpsCanvasBridge::cursorCoordinateChanged );
  connect( mCanvas, &QgsMapCanvas::tapAndHoldGestureOccurred, this, &QgsGpsCanvasBridge::tapAndHold );

  mBearingNumericFormat.reset( QgsLocalDefaultSettings::bearingFormat() );
  connect( QgsProject::instance()->displaySettings(), &QgsProjectDisplaySettings::bearingFormatChanged, this, [this] {
    mBearingNumericFormat.reset( QgsProject::instance()->displaySettings()->bearingFormat()->clone() );
    updateGpsDistanceStatusMessage( false );
  } );

  gpsSettingsChanged();

  mCanvas->installInteractionBlocker( this );
}

QgsGpsCanvasBridge::~QgsGpsCanvasBridge()
{
  delete mMapMarker;
  mMapMarker = nullptr;
  delete mMapBearingItem;
  mMapBearingItem = nullptr;

  if ( mCanvas )
    mCanvas->removeInteractionBlocker( this );
}

bool QgsGpsCanvasBridge::blockCanvasInteraction( Interaction interaction ) const
{
  switch ( interaction )
  {
    case QgsMapCanvasInteractionBlocker::Interaction::MapPanOnSingleClick:
      // if we're connected and set to follow the GPS location, block the single click navigation mode
      // to avoid accidental map canvas pans away from the GPS location.
      // (for now, we don't block click-and-drag pans, as they are less likely to be accidentally triggered)
      if ( mConnection->isConnected() && ( mCenteringMode != Qgis::MapRecenteringMode::Never ) )
        return true;

      break;
  }

  return false;
}

void QgsGpsCanvasBridge::setLocationMarkerVisible( bool show )
{
  mShowMarker = show;
}

void QgsGpsCanvasBridge::setBearingLineVisible( bool show )
{
  if ( !show )
  {
    if ( mMapBearingItem )
    {
      delete mMapBearingItem;
      mMapBearingItem = nullptr;
    }
  }
  mShowBearingLine = show;
}

void QgsGpsCanvasBridge::setRotateMap( bool enabled )
{
  mRotateMap = enabled;
}

void QgsGpsCanvasBridge::setMapCenteringMode( Qgis::MapRecenteringMode mode )
{
  mCenteringMode = mode;
}

void QgsGpsCanvasBridge::tapAndHold( const QgsPointXY &mapPoint, QTapAndHoldGesture * )
{
  if ( !mConnection->isConnected() )
    return;

  try
  {
    mLastCursorPosWgs84 = mCanvasToWgs84Transform.transform( mapPoint );
    updateGpsDistanceStatusMessage( true );
  }
  catch ( QgsCsException & )
  {
  }
}

void QgsGpsCanvasBridge::updateBearingAppearance()
{
  if ( !mMapBearingItem )
    return;

  QDomDocument doc;
  QDomElement elem;
  QString bearingLineSymbolXml = QgsGpsCanvasBridge::settingBearingLineSymbol->value();
  if ( bearingLineSymbolXml.isEmpty() )
  {
    QgsSettings settings;
    bearingLineSymbolXml = settings.value( u"bearingLineSymbol"_s, QVariant(), QgsSettings::Gps ).toString();
  }

  if ( !bearingLineSymbolXml.isEmpty() )
  {
    doc.setContent( bearingLineSymbolXml );
    elem = doc.documentElement();
    std::unique_ptr<QgsLineSymbol> bearingSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, QgsReadWriteContext() ) );
    if ( bearingSymbol )
    {
      mMapBearingItem->setSymbol( std::move( bearingSymbol ) );
    }
  }
}

void QgsGpsCanvasBridge::gpsSettingsChanged()
{
  updateBearingAppearance();

  QgsSettings settings;
  if ( QgsGpsConnection::settingsGpsConnectionType->exists() )
  {
    mBearingFromTravelDirection = QgsGpsConnection::settingGpsBearingFromTravelDirection->value();
    mMapExtentMultiplier = static_cast<int>( QgsGpsCanvasBridge::settingMapExtentRecenteringThreshold->value() );
    mMapRotateInterval = static_cast<int>( QgsGpsCanvasBridge::settingMapRotateInterval->value() );
  }
  else
  {
    // legacy settings
    mBearingFromTravelDirection = settings.value( u"calculateBearingFromTravel"_s, "false", QgsSettings::Gps ).toBool();

    mMapExtentMultiplier = settings.value( u"mapExtentMultiplier"_s, "50", QgsSettings::Gps ).toInt();
    mMapRotateInterval = settings.value( u"rotateMapInterval"_s, 0, QgsSettings::Gps ).toInt();
  }
}

void QgsGpsCanvasBridge::gpsDisconnected()
{
  if ( mMapMarker ) // marker should not be shown on GPS disconnected - not current position
  {
    delete mMapMarker;
    mMapMarker = nullptr;
  }
  if ( mMapBearingItem )
  {
    delete mMapBearingItem;
    mMapBearingItem = nullptr;
  }
}

void QgsGpsCanvasBridge::gpsStateChanged( const QgsGpsInformation &info )
{
  const bool validFlag = info.isValid();
  QgsPointXY myNewCenter;
  if ( validFlag )
  {
    myNewCenter = QgsPointXY( info.longitude, info.latitude );
  }
  else
  {
    myNewCenter = mLastGpsPosition;
  }

  // Avoid refreshing / panning if we haven't moved
  if ( mLastGpsPosition != myNewCenter )
  {
    mSecondLastGpsPosition = mLastGpsPosition;
    mLastGpsPosition = myNewCenter;
    // Pan based on user specified behavior
    switch ( mCenteringMode )
    {
      case Qgis::MapRecenteringMode::Always:
      case Qgis::MapRecenteringMode::WhenOutsideVisibleExtent:
        try
        {
          const QgsPointXY point = mCanvasToWgs84Transform.transform( myNewCenter, Qgis::TransformDirection::Reverse );
          //keep the extent the same just center the map canvas in the display so our feature is in the middle
          const QgsRectangle rect( point, point ); // empty rect can be used to set new extent that is centered on the point used to construct the rect

          // testing if position is outside some proportion of the map extent
          // this is a user setting - useful range: 5% to 100% (0.05 to 1.0)
          QgsRectangle extentLimit( mCanvas->extent() );
          extentLimit.scale( mMapExtentMultiplier * 0.01 );

          // only change the extents if the point is beyond the current extents to minimize repaints
          if ( mCenteringMode == Qgis::MapRecenteringMode::Always || ( mCenteringMode == Qgis::MapRecenteringMode::WhenOutsideVisibleExtent && !extentLimit.contains( point ) ) )
          {
            mCanvas->setExtent( rect, true );
            mCanvas->refresh();
          }
        }
        catch ( QgsCsException & )
        {
        }
        break;

      case Qgis::MapRecenteringMode::Never:
        break;
    }

    updateGpsDistanceStatusMessage( false );
  }

  double bearing = 0;
  double trueNorth = 0;
  const QgsSettings settings;
  const double adjustment = settings.value( u"gps/bearingAdjustment"_s, 0.0, QgsSettings::App ).toDouble();

  if ( !std::isnan( info.direction ) || ( mBearingFromTravelDirection && !mSecondLastGpsPosition.isEmpty() ) )
  {
    if ( !mBearingFromTravelDirection )
    {
      bearing = info.direction;
      if ( settings.value( u"gps/correctForTrueNorth"_s, false, QgsSettings::App ).toBool() )
      {
        try
        {
          trueNorth = QgsBearingUtils::bearingTrueNorth( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext(), mCanvas->mapSettings().visibleExtent().center() );
        }
        catch ( QgsException & )
        {
        }
      }
    }
    else
    {
      try
      {
        bearing = 180 * mDistanceCalculator.bearing( mSecondLastGpsPosition, mLastGpsPosition ) / M_PI;
      }
      catch ( QgsCsException & )
      {
      }
    }

    if ( mRotateMap && ( !mLastRotateTimer.isValid() || mLastRotateTimer.hasExpired( static_cast<long long>( mMapRotateInterval ) * 1000 ) ) )
    {
      const QgsCoordinateTransform wgs84ToCanvas( mWgs84CRS, mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );

      try
      {
        QLineF bearingLine;
        bearingLine.setP1( wgs84ToCanvas.transform( myNewCenter ).toQPointF() );

        // project out the bearing line by roughly the size of the canvas
        QgsDistanceArea da1;
        da1.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
        da1.setEllipsoid( QgsProject::instance()->ellipsoid() );
        double totalLength = 0;
        try
        {
          totalLength = da1.measureLine( mCanvas->mapSettings().extent().center(), QgsPointXY( mCanvas->mapSettings().extent().xMaximum(), mCanvas->mapSettings().extent().yMaximum() ) );
        }
        catch ( QgsCsException & )
        {
          // TODO report errors to user
          QgsDebugError( u"An error occurred while calculating length"_s );
        }

        QgsDistanceArea da;
        da.setSourceCrs( mWgs84CRS, QgsProject::instance()->transformContext() );
        da.setEllipsoid( QgsProject::instance()->ellipsoid() );
        const QgsPointXY res = da.computeSpheroidProject( myNewCenter, totalLength, ( bearing - trueNorth + adjustment ) * M_PI / 180.0 );
        bearingLine.setP2( wgs84ToCanvas.transform( res ).toQPointF() );

        mCanvas->setRotation( 270 - bearingLine.angle() );
        mCanvas->refresh();
      }
      catch ( QgsCsException & )
      {
        QgsDebugError( u"Coordinate exception encountered while calculating GPS bearing rotation"_s );
        mCanvas->setRotation( trueNorth - bearing - adjustment );
        mCanvas->refresh();
      }
      mLastRotateTimer.restart();
    }

    if ( mShowBearingLine )
    {
      if ( !mMapBearingItem )
      {
        mMapBearingItem = new QgsGpsBearingItem( mCanvas );
        updateBearingAppearance();
      }

      mMapBearingItem->setGpsPosition( myNewCenter );
      mMapBearingItem->setGpsBearing( bearing - trueNorth + adjustment );
    }
    else if ( mMapBearingItem )
    {
      delete mMapBearingItem;
      mMapBearingItem = nullptr;
    }
  }
  else if ( mMapBearingItem )
  {
    delete mMapBearingItem;
    mMapBearingItem = nullptr;
  }

  // new marker position after recentering
  if ( mShowMarker ) // show marker
  {
    if ( validFlag ) // update cursor position if valid position
    {
      // initially, cursor isn't drawn until first valid fix; remains visible until GPS disconnect
      if ( !mMapMarker )
      {
        mMapMarker = new QgsGpsMarker( mCanvas );
      }
      mMapMarker->setGpsPosition( myNewCenter );

      mMapMarker->setMarkerRotation( bearing - trueNorth + adjustment );
    }
  }
  else
  {
    if ( mMapMarker )
    {
      delete mMapMarker;
      mMapMarker = nullptr;
    }
  }
}

void QgsGpsCanvasBridge::cursorCoordinateChanged( const QgsPointXY &point )
{
  if ( !mConnection->isConnected() )
    return;

  try
  {
    mLastCursorPosWgs84 = mCanvasToWgs84Transform.transform( point );
    updateGpsDistanceStatusMessage( true );
  }
  catch ( QgsCsException & )
  {
  }
}

void QgsGpsCanvasBridge::updateGpsDistanceStatusMessage( bool forceDisplay )
{
  if ( !mConnection->isConnected() )
    return;

  static constexpr int GPS_DISTANCE_MESSAGE_TIMEOUT_MS = 2000;

  if ( !forceDisplay )
  {
    // if we aren't forcing the display of the message (i.e. in direct response to a mouse cursor movement),
    // then only show an updated message when the GPS position changes if the previous forced message occurred < 2 seconds ago.
    // otherwise we end up showing infinite messages as the GPS position constantly changes...
    if ( mLastForcedStatusUpdate.hasExpired( GPS_DISTANCE_MESSAGE_TIMEOUT_MS ) )
      return;
  }
  else
  {
    mLastForcedStatusUpdate.restart();
  }

  try
  {
    const double distance = mDistanceCalculator.convertLengthMeasurement( mDistanceCalculator.measureLine( QVector<QgsPointXY>() << mLastCursorPosWgs84 << mLastGpsPosition ), QgsProject::instance()->distanceUnits() );
    const double bearing = 180 * mDistanceCalculator.bearing( mLastGpsPosition, mLastCursorPosWgs84 ) / M_PI;
    const int distanceDecimalPlaces = QgsSettings().value( u"qgis/measure/decimalplaces"_s, "3" ).toInt();
    const QString distanceString = QgsDistanceArea::formatDistance( distance, distanceDecimalPlaces, QgsProject::instance()->distanceUnits() );
    const QString bearingString = mBearingNumericFormat->formatDouble( bearing, QgsNumericFormatContext() );

    QgisApp::instance()->statusBarIface()->showMessage( tr( "%1 (%2) from GPS location" ).arg( distanceString, bearingString ), forceDisplay ? GPS_DISTANCE_MESSAGE_TIMEOUT_MS : GPS_DISTANCE_MESSAGE_TIMEOUT_MS - static_cast<int>( mLastForcedStatusUpdate.elapsed() ) );
  }
  catch ( QgsCsException & )
  {
  }
}
