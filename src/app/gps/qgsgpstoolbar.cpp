/***************************************************************************
    qgsgpstoolbar.cpp
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

#include "qgsgpstoolbar.h"
#include "qgsappgpsconnection.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgis.h"
#include "qgscoordinateutils.h"
#include "qgisapp.h"
#include "qgsappgpssettingsmenu.h"
#include "qgsapplication.h"

#include <QLabel>
#include <QToolButton>

QgsGpsToolBar::QgsGpsToolBar( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QWidget *parent )
  : QToolBar( parent )
  , mConnection( connection )
  , mCanvas( canvas )
{
  setObjectName( QStringLiteral( "mGpsToolBar" ) );
  setWindowTitle( tr( "GPS Toolbar" ) );
  setToolTip( tr( "GPS Toolbar" ) );

  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

  mConnectAction = new QAction( tr( "Connect GPS" ), this );
  mConnectAction->setToolTip( tr( "Connect to GPS" ) );
  mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsConnect.svg" ) ) );
  mConnectAction->setCheckable( true );
  addAction( mConnectAction );

  connect( mConnectAction, &QAction::toggled, this, [ = ]( bool connect )
  {
    if ( connect )
      mConnection->connectGps();
    else
      mConnection->disconnectGps();
  } );

  mRecenterAction = new QAction( tr( "Recenter" ) );
  mRecenterAction->setToolTip( tr( "Recenter map on GPS location" ) );
  mRecenterAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mActionRecenter.svg" ) ) );
  mRecenterAction->setEnabled( false );

  connect( mRecenterAction, &QAction::triggered, this, [ = ]
  {
    if ( mConnection->lastValidLocation().isEmpty() )
      return;

    const QgsCoordinateTransform canvasToWgs84Transform = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
    try
    {
      const QgsPointXY center = canvasToWgs84Transform.transform( mConnection->lastValidLocation(), Qgis::TransformDirection::Reverse );
      mCanvas->setCenter( center );
      mCanvas->refresh();
    }
    catch ( QgsCsException & )
    {

    }
  } );
  addAction( mRecenterAction );

  addSeparator();

  mAddTrackPointAction = new QAction( tr( "Add Track Point" ), this );
  mAddTrackPointAction->setEnabled( false );
  mAddTrackPointAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mActionAddTrackPoint.svg" ) ) );
  connect( mAddTrackPointAction, &QAction::triggered, this, &QgsGpsToolBar::addVertexClicked );
  addAction( mAddTrackPointAction );

  mAddFeatureAction = new QAction( tr( "Create Feature" ), this );
  mAddFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureLine.svg" ) ) );
  connect( mAddFeatureAction, &QAction::triggered, this, &QgsGpsToolBar::addFeatureClicked );
  addAction( mAddFeatureAction );

  mResetFeatureAction = new QAction( tr( "Reset Feature" ), this );
  mResetFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mActionReset.svg" ) ) );
  connect( mResetFeatureAction, &QAction::triggered, this, &QgsGpsToolBar::resetFeatureClicked );
  addAction( mResetFeatureAction );

  addSeparator();

  mShowInfoAction = new QAction( tr( "Information" ) );
  mShowInfoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionPropertiesWidget.svg" ) ) );
  mShowInfoAction->setToolTip( tr( "Show GPS Information Panel" ) );
  mShowInfoAction->setCheckable( true );
  addAction( mShowInfoAction );

  mLocationLabel = new QLabel();
  addWidget( mLocationLabel );

  connect( mConnection, &QgsAppGpsConnection::positionChanged, this, &QgsGpsToolBar::updateLocationLabel );
  updateLocationLabel( mConnection->lastValidLocation() );

  QToolButton *settingsButton = new QToolButton();
  settingsButton->setAutoRaise( true );
  settingsButton->setToolTip( tr( "Settings" ) );
  settingsButton->setMenu( QgisApp::instance()->gpsSettingsMenu() );
  settingsButton->setPopupMode( QToolButton::InstantPopup );
  settingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  addWidget( settingsButton );

  mRecenterAction->setEnabled( false );
  mAddFeatureAction->setEnabled( false );
  mAddTrackPointAction->setEnabled( false );
  connect( mConnection, &QgsAppGpsConnection::statusChanged, this, [ = ]( Qgis::GpsConnectionStatus status )
  {
    switch ( status )
    {
      case Qgis::GpsConnectionStatus::Disconnected:
        whileBlocking( mConnectAction )->setChecked( false );
        mConnectAction->setText( tr( "Connect GPS" ) );
        mConnectAction->setToolTip( tr( "Connect to GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsConnect.svg" ) ) );
        mConnectAction->setEnabled( true );
        mRecenterAction->setEnabled( false );
        mAddFeatureAction->setEnabled( false );
        mAddTrackPointAction->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connecting:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setToolTip( tr( "Connecting to GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsConnect.svg" ) ) );
        mConnectAction->setEnabled( false );
        mRecenterAction->setEnabled( false );
        mAddFeatureAction->setEnabled( false );
        mAddTrackPointAction->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connected:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setText( tr( "Disconnect GPS" ) );
        mConnectAction->setToolTip( tr( "Disconnect from GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsDisconnect.svg" ) ) );
        mConnectAction->setEnabled( true );
        mRecenterAction->setEnabled( true );
        mAddFeatureAction->setEnabled( true );
        mAddTrackPointAction->setEnabled( mEnableAddVertexButton );
        break;
    }
  } );

  connect( QgisApp::instance(), &QgisApp::activeLayerChanged,
           this, &QgsGpsToolBar::updateCloseFeatureButton );
}

void QgsGpsToolBar::setAddVertexButtonEnabled( bool enabled )
{
  mEnableAddVertexButton = enabled;
  mAddTrackPointAction->setEnabled( mEnableAddVertexButton && mConnection->isConnected() );
}

void QgsGpsToolBar::updateLocationLabel( const QgsPoint &point )
{
  if ( point.isEmpty() )
  {
    mLocationLabel->clear();
  }
  else
  {
    const QString pos = QgsCoordinateUtils::formatCoordinateForProject( QgsProject::instance(), point, QgsCoordinateReferenceSystem(), 8 );
    mLocationLabel->setText( pos );
  }
}

void QgsGpsToolBar::updateCloseFeatureButton( QgsMapLayer *lyr )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( lyr );

  if ( !( vlayer && vlayer->isValid() ) )
    return;

  // Add feature button tracks edit state of layer
  if ( vlayer != mLastLayer )
  {
    if ( mLastLayer )  // disconnect previous layer
    {
      disconnect( mLastLayer, &QgsVectorLayer::editingStarted,
                  this, &QgsGpsToolBar::layerEditStateChanged );
      disconnect( mLastLayer, &QgsVectorLayer::editingStopped,
                  this, &QgsGpsToolBar::layerEditStateChanged );
    }
    if ( vlayer ) // connect new layer
    {
      connect( vlayer, &QgsVectorLayer::editingStarted,
               this, &QgsGpsToolBar::layerEditStateChanged );
      connect( vlayer, &QgsVectorLayer::editingStopped,
               this, &QgsGpsToolBar::layerEditStateChanged );
    }
    mLastLayer = vlayer;
  }

  QString buttonLabel = tr( "Create Feature" );
  QString icon = QStringLiteral( "mActionCaptureLine.svg" );;
  if ( vlayer )
  {
    QgsVectorDataProvider *provider = vlayer->dataProvider();
    const QgsWkbTypes::GeometryType layerGeometryType = vlayer->geometryType();

    bool enable = provider->capabilities() & QgsVectorDataProvider::AddFeatures &&  // layer can add features
                  vlayer->isEditable() && vlayer->isSpatial();

    switch ( layerGeometryType )
    {
      case QgsWkbTypes::PointGeometry:
        buttonLabel = tr( "Create Point Feature" );
        icon = QStringLiteral( "mActionCapturePoint.svg" );
        break;

      case QgsWkbTypes::LineGeometry:
        buttonLabel = tr( "Create Line Feature" );
        icon = QStringLiteral( "mActionCaptureLine.svg" );
        break;

      case QgsWkbTypes::PolygonGeometry:
        buttonLabel = tr( "Create Polygon Feature" );
        icon = QStringLiteral( "mActionCapturePolygon.svg" );
        break;

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
        enable = false;
        break;
    }

    mAddFeatureAction->setEnabled( enable );
  }
  else
  {
    mAddFeatureAction->setEnabled( false );
  }
  mAddFeatureAction->setText( buttonLabel );
  mAddFeatureAction->setIcon( QgsApplication::getThemeIcon( icon ) );
  mAddFeatureAction->setToolTip( buttonLabel );
}

void QgsGpsToolBar::layerEditStateChanged()
{
  updateCloseFeatureButton( mLastLayer );
}
