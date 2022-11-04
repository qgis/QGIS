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
#include "qgsprojectgpssettings.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"

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

  mDestinationLayerModel = new QgsMapLayerProxyModel( this );
  mDestinationLayerModel->setProject( QgsProject::instance() );
  mDestinationLayerModel->setFilters( QgsMapLayerProxyModel::Filter::HasGeometry | QgsMapLayerProxyModel::Filter::WritableLayer );

  mDestinationLayerMenu = new QMenu( this );
  connect( mDestinationLayerMenu, &QMenu::aboutToShow, this, &QgsGpsToolBar::destinationMenuAboutToShow );

  QToolButton *destinationLayerButton = new QToolButton();
  destinationLayerButton->setAutoRaise( true );
  destinationLayerButton->setToolTip( tr( "Set destination layer for GPS digitized features" ) );
  destinationLayerButton->setMenu( mDestinationLayerMenu );
  destinationLayerButton->setPopupMode( QToolButton::InstantPopup );
  destinationLayerButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  addWidget( destinationLayerButton );

  mAddTrackVertexAction = new QAction( tr( "Add Track Vertex" ), this );
  mAddTrackVertexAction->setToolTip( tr( "Add vertex to GPS track using current GPS location" ) );
  mAddTrackVertexAction->setEnabled( false );
  mAddTrackVertexAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mActionAddTrackPoint.svg" ) ) );
  connect( mAddTrackVertexAction, &QAction::triggered, this, &QgsGpsToolBar::addVertexClicked );
  addAction( mAddTrackVertexAction );

  mAddFeatureAction = new QAction( tr( "Create Feature from Track" ), this );
  mAddFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureLine.svg" ) ) );
  connect( mAddFeatureAction, &QAction::triggered, this, &QgsGpsToolBar::addFeatureClicked );
  addAction( mAddFeatureAction );

  mResetFeatureAction = new QAction( tr( "Reset Track" ), this );
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
  mAddTrackVertexAction->setEnabled( false );
  mResetFeatureAction->setEnabled( false );
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
        mAddTrackVertexAction->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connecting:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setToolTip( tr( "Connecting to GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsConnect.svg" ) ) );
        mConnectAction->setEnabled( false );
        mRecenterAction->setEnabled( false );
        mAddFeatureAction->setEnabled( false );
        mAddTrackVertexAction->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connected:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setText( tr( "Disconnect GPS" ) );
        mConnectAction->setToolTip( tr( "Disconnect from GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsDisconnect.svg" ) ) );
        mConnectAction->setEnabled( true );
        mRecenterAction->setEnabled( true );
        mAddFeatureAction->setEnabled( static_cast< bool >( QgsProject::instance()->gpsSettings()->destinationLayer() ) );
        mAddTrackVertexAction->setEnabled( mEnableAddVertexButton );
        break;
    }
  } );

  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::destinationLayerChanged,
           this, &QgsGpsToolBar::destinationLayerChanged );

  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::automaticallyAddTrackVerticesChanged, this, [ = ]( bool enabled ) { setAddVertexButtonEnabled( !enabled ); } );
  setAddVertexButtonEnabled( !QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() );
}

void QgsGpsToolBar::setAddVertexButtonEnabled( bool enabled )
{
  mEnableAddVertexButton = enabled;
  mAddTrackVertexAction->setEnabled( mEnableAddVertexButton && mConnection->isConnected() );
}

void QgsGpsToolBar::setResetTrackButtonEnabled( bool enabled )
{
  mResetFeatureAction->setEnabled( enabled );
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

void QgsGpsToolBar::destinationLayerChanged( QgsVectorLayer *vlayer )
{
  if ( vlayer )
  {

  }

  if ( !( vlayer && vlayer->isValid() ) )
  {
    mAddFeatureAction->setEnabled( false );
    return;
  }

  QString buttonLabel = tr( "Create Feature" );
  QString buttonToolTip = tr( "Create Feature" );
  QString icon = QStringLiteral( "mActionCaptureLine.svg" );;
  if ( vlayer )
  {
    const QgsWkbTypes::GeometryType layerGeometryType = vlayer->geometryType();
    bool enable = true;

    switch ( layerGeometryType )
    {
      case QgsWkbTypes::PointGeometry:
        buttonLabel = tr( "Create Point Feature at Location" );
        buttonToolTip = tr( "Create a new point feature at the current GPS location" );
        icon = QStringLiteral( "mActionCapturePoint.svg" );
        break;

      case QgsWkbTypes::LineGeometry:
        buttonLabel = tr( "Create Line Feature from Track" );
        buttonToolTip = tr( "Create a new line feature using the current GPS track" );
        icon = QStringLiteral( "mActionCaptureLine.svg" );
        break;

      case QgsWkbTypes::PolygonGeometry:
        buttonLabel = tr( "Create Polygon Feature from Track" );
        buttonToolTip = tr( "Create a new polygon feature using the current GPS track" );
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
  mAddFeatureAction->setToolTip( buttonToolTip );
}

void QgsGpsToolBar::destinationMenuAboutToShow()
{
  mDestinationLayerMenu->clear();

  const QString currentLayerId = QgsProject::instance()->gpsSettings()->destinationLayer() ?
                                 QgsProject::instance()->gpsSettings()->destinationLayer()->id() : QString();

  for ( int row = 0; row < mDestinationLayerModel->rowCount(); ++row )
  {
    const QModelIndex index = mDestinationLayerModel->index( row, 0 );

    QAction *layerAction = new QAction( index.data( Qt::DisplayRole ).toString(), mDestinationLayerMenu );
    layerAction->setToolTip( index.data( Qt::ToolTipRole ).toString() );
    layerAction->setIcon( index.data( Qt::DecorationRole ).value< QIcon >() );
    layerAction->setCheckable( true );

    const QString actionLayerId = index.data( QgsMapLayerModel::ItemDataRole::LayerIdRole ).toString();

    if ( actionLayerId == currentLayerId )
      layerAction->setChecked( true );

    connect( layerAction, &QAction::toggled, this, [ = ]( bool checked )
    {
      if ( checked )
      {
        QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProject::instance()->mapLayer( actionLayerId ) );
        if ( layer != QgsProject::instance()->gpsSettings()->destinationLayer() )
        {
          QgsProject::instance()->gpsSettings()->setDestinationLayer( layer );
          QgsProject::instance()->setDirty();
        }
      }
    } );

    mDestinationLayerMenu->addAction( layerAction );
  }
}

