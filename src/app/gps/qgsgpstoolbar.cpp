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
#include "qgsappgpsdigitizing.h"
#include "qgsunittypes.h"
#include "qgsgpsinformation.h"

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

  mDestinationLayerButton = new QToolButton();
  mDestinationLayerButton->setAutoRaise( true );
  mDestinationLayerButton->setToolTip( tr( "Set destination layer for GPS digitized features" ) );
  mDestinationLayerButton->setMenu( mDestinationLayerMenu );
  mDestinationLayerButton->setPopupMode( QToolButton::InstantPopup );
  mDestinationLayerButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsDestinationLayer.svg" ) ) );
  addWidget( mDestinationLayerButton );

  mAddTrackVertexAction = new QAction( tr( "Add Track Vertex" ), this );
  mAddTrackVertexAction->setToolTip( tr( "Add vertex to GPS track using current GPS location" ) );
  mAddTrackVertexAction->setEnabled( false );
  mAddTrackVertexAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mActionAddTrackPoint.svg" ) ) );
  connect( mAddTrackVertexAction, &QAction::triggered, this, &QgsGpsToolBar::addVertexClicked );
  addAction( mAddTrackVertexAction );

  mCreateFeatureAction = new QAction( tr( "Create Feature from Track" ), this );
  mCreateFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureLine.svg" ) ) );
  connect( mCreateFeatureAction, &QAction::triggered, this, &QgsGpsToolBar::addFeatureClicked );
  addAction( mCreateFeatureAction );

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

  connect( mConnection, &QgsAppGpsConnection::positionChanged, this, &QgsGpsToolBar::updateLocationLabel );
  connect( mConnection, &QgsAppGpsConnection::stateChanged, this, &QgsGpsToolBar::updateLocationLabel );
  updateLocationLabel();

  QToolButton *settingsButton = new QToolButton();
  settingsButton->setAutoRaise( true );
  settingsButton->setToolTip( tr( "Settings" ) );
  settingsButton->setMenu( QgisApp::instance()->gpsSettingsMenu() );
  settingsButton->setPopupMode( QToolButton::InstantPopup );
  settingsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  mSettingsMenuAction = addWidget( settingsButton );

  mRecenterAction->setEnabled( false );
  mCreateFeatureAction->setEnabled( false );
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
        mCreateFeatureAction->setEnabled( false );
        mAddTrackVertexAction->setEnabled( false );
        delete mInformationButton;
        mInformationButton = nullptr;
        break;
      case Qgis::GpsConnectionStatus::Connecting:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setToolTip( tr( "Connecting to GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsConnect.svg" ) ) );
        mConnectAction->setEnabled( false );
        mRecenterAction->setEnabled( false );
        mCreateFeatureAction->setEnabled( false );
        mAddTrackVertexAction->setEnabled( false );
        delete mInformationButton;
        mInformationButton = nullptr;
        break;
      case Qgis::GpsConnectionStatus::Connected:
        whileBlocking( mConnectAction )->setChecked( true );
        mConnectAction->setText( tr( "Disconnect GPS" ) );
        mConnectAction->setToolTip( tr( "Disconnect from GPS" ) );
        mConnectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/gpsicons/mIconGpsDisconnect.svg" ) ) );
        mConnectAction->setEnabled( true );
        mRecenterAction->setEnabled( true );
        mCreateFeatureAction->setEnabled( static_cast< bool >( QgsProject::instance()->gpsSettings()->destinationLayer() ) );
        mAddTrackVertexAction->setEnabled( mEnableAddVertexButton );
        break;
    }
    adjustSize();
  } );

  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::destinationLayerChanged,
           this, &QgsGpsToolBar::destinationLayerChanged );

  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::automaticallyAddTrackVerticesChanged, this, [ = ]( bool enabled ) { setAddVertexButtonEnabled( !enabled ); } );
  setAddVertexButtonEnabled( !QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() );

  adjustSize();
}

void QgsGpsToolBar::setGpsDigitizing( QgsAppGpsDigitizing *digitizing )
{
  mDigitizing = digitizing;
  connect( mDigitizing, &QgsAppGpsDigitizing::distanceAreaChanged, this, &QgsGpsToolBar::updateLocationLabel );
  connect( mDigitizing, &QgsAppGpsDigitizing::trackVertexAdded, this, &QgsGpsToolBar::updateLocationLabel );
  connect( mDigitizing, &QgsAppGpsDigitizing::trackReset, this, &QgsGpsToolBar::updateLocationLabel );
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

void QgsGpsToolBar::updateLocationLabel()
{
  const QgsPoint point = mConnection->lastValidLocation();
  if ( point.isEmpty() )
  {
    delete mInformationButton;
    mInformationButton = nullptr;
  }
  else
  {
    if ( !mInformationButton )
    {
      createLocationWidget();
    }

    const QgsGpsInformation information = mConnection->lastInformation();

    const Qgis::GpsInformationComponents visibleComponents = settingShowInToolbar.value();

    QStringList parts;
    for ( Qgis::GpsInformationComponent component :
          {
            Qgis::GpsInformationComponent::Location,
            Qgis::GpsInformationComponent::Altitude,
            Qgis::GpsInformationComponent::EllipsoidAltitude,
            Qgis::GpsInformationComponent::Bearing,
            Qgis::GpsInformationComponent::GroundSpeed,
            Qgis::GpsInformationComponent::TotalTrackLength,
            Qgis::GpsInformationComponent::TrackDistanceFromStart,
          } )
    {
      if ( visibleComponents & component )
      {
        const QVariant value = information.componentValue( component );
        switch ( component )
        {
          case Qgis::GpsInformationComponent::Location:
            parts << QgsCoordinateUtils::formatCoordinateForProject( QgsProject::instance(), point, QgsCoordinateReferenceSystem(), 8 );
            break;
          case Qgis::GpsInformationComponent::Altitude:
          case Qgis::GpsInformationComponent::EllipsoidAltitude:
            parts << tr( "%1 m" ).arg( value.toDouble( ) );
            break;
          case Qgis::GpsInformationComponent::GroundSpeed:
            parts << tr( "%1 km/h" ).arg( value.toDouble( ) );
            break;
          case Qgis::GpsInformationComponent::Bearing:
            parts << QString::number( value.toDouble( ) ) + QChar( 176 );
            break;

          case Qgis::GpsInformationComponent::TotalTrackLength:
          case Qgis::GpsInformationComponent::TrackDistanceFromStart:
          {
            if ( mDigitizing )
            {
              const double measurement = component == Qgis::GpsInformationComponent::TotalTrackLength
                                         ? mDigitizing->totalTrackLength()
                                         : mDigitizing->trackDistanceFromStart();

              const QgsSettings settings;
              const bool keepBaseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();
              const int decimalPlaces = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), 3 ).toInt();

              if ( measurement > 0 )
                parts << mDigitizing->distanceArea().formatDistance( measurement, decimalPlaces, mDigitizing->distanceArea().lengthUnits(), keepBaseUnit );
              else
                parts << QStringLiteral( "0%1" ).arg( QgsUnitTypes::toAbbreviatedString( mDigitizing->distanceArea().lengthUnits() ) );
            }

            break;
          }

          case Qgis::GpsInformationComponent::GeoidalSeparation:
          case Qgis::GpsInformationComponent::Pdop:
          case Qgis::GpsInformationComponent::Hdop:
          case Qgis::GpsInformationComponent::Vdop:
          case Qgis::GpsInformationComponent::HorizontalAccuracy:
          case Qgis::GpsInformationComponent::VerticalAccuracy:
          case Qgis::GpsInformationComponent::HvAccuracy:
          case Qgis::GpsInformationComponent::SatellitesUsed:
          case Qgis::GpsInformationComponent::Timestamp:
          case Qgis::GpsInformationComponent::TrackStartTime:
          case Qgis::GpsInformationComponent::TrackEndTime:
          case Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
          case Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
            // not supported here
            break;
        }
      }
    }

    mInformationButton->setText( parts.join( ' ' ) );

    //ensure the label is big (and small) enough
    const int width = mInformationButton->fontMetrics().boundingRect( mInformationButton->text() ).width() + 16;
    bool allowResize = false;
    if ( mIsFirstSizeChange )
    {
      allowResize = true;
    }
    else if ( mInformationButton->minimumWidth() < width )
    {
      // always immediately grow to fit
      allowResize = true;
    }
    else if ( ( mInformationButton->minimumWidth() - width ) > mInformationButton->fontMetrics().averageCharWidth() * 2 )
    {
      // only allow shrinking when a sufficient time has expired since we last resized.
      // this avoids extraneous shrinking/growing resulting in distracting UI changes
      allowResize = mLastLabelSizeChangeTimer.hasExpired( 5000 );
    }

    if ( allowResize )
    {
      mInformationButton->setMinimumWidth( width );
      mInformationButton->setMaximumWidth( width );
      mLastLabelSizeChangeTimer.restart();
      mIsFirstSizeChange = false;
    }
  }

  adjustSize();
}

void QgsGpsToolBar::destinationLayerChanged( QgsVectorLayer *vlayer )
{
  if ( vlayer )
  {
    mDestinationLayerButton->setToolTip( tr( "GPS digitized features will be stored in %1" ).arg( vlayer->name() ) );
  }
  else
  {
    mDestinationLayerButton->setToolTip( tr( "Set destination layer for GPS digitized features" ) );
  }

  if ( !( vlayer && vlayer->isValid() ) )
  {
    mCreateFeatureAction->setEnabled( false );
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

    mCreateFeatureAction->setEnabled( enable );
  }
  else
  {
    mCreateFeatureAction->setEnabled( false );
  }
  mCreateFeatureAction->setText( buttonLabel );
  mCreateFeatureAction->setIcon( QgsApplication::getThemeIcon( icon ) );
  mCreateFeatureAction->setToolTip( buttonToolTip );
}

void QgsGpsToolBar::destinationMenuAboutToShow()
{
  mDestinationLayerMenu->clear();

  const QString currentLayerId = QgsProject::instance()->gpsSettings()->destinationLayer() ?
                                 QgsProject::instance()->gpsSettings()->destinationLayer()->id() : QString();

  QAction *followAction = new QAction( tr( "Follow Active Layer" ), mDestinationLayerMenu );
  followAction->setToolTip( tr( "Always add GPS digitized features to the active layer" ) );
  followAction->setCheckable( true );
  followAction->setChecked( QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() );

  connect( followAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    if ( checked && !QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() )
    {
      QgsProject::instance()->gpsSettings()->setDestinationFollowsActiveLayer( true );
      QgsProject::instance()->setDirty();
    }
  } );
  mDestinationLayerMenu->addAction( followAction );

  for ( int row = 0; row < mDestinationLayerModel->rowCount(); ++row )
  {
    const QModelIndex index = mDestinationLayerModel->index( row, 0 );

    QAction *layerAction = new QAction( index.data( Qt::DisplayRole ).toString(), mDestinationLayerMenu );
    layerAction->setToolTip( index.data( Qt::ToolTipRole ).toString() );
    layerAction->setIcon( index.data( Qt::DecorationRole ).value< QIcon >() );
    layerAction->setCheckable( true );

    const QString actionLayerId = index.data( QgsMapLayerModel::ItemDataRole::LayerIdRole ).toString();

    if ( actionLayerId == currentLayerId && !QgsProject::instance()->gpsSettings()->destinationFollowsActiveLayer() )
      layerAction->setChecked( true );

    connect( layerAction, &QAction::toggled, this, [ = ]( bool checked )
    {
      if ( checked )
      {
        QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProject::instance()->mapLayer( actionLayerId ) );
        if ( layer != QgsProject::instance()->gpsSettings()->destinationLayer() )
        {
          QgsProject::instance()->gpsSettings()->setDestinationFollowsActiveLayer( false );
          QgsProject::instance()->gpsSettings()->setDestinationLayer( layer );
          QgsProject::instance()->setDirty();
        }
      }
    } );

    mDestinationLayerMenu->addAction( layerAction );
  }
}

void QgsGpsToolBar::createLocationWidget()
{
  mInformationButton = new QToolButton();
  mInformationButton->setToolTip( tr( "Current GPS Information" ) );

  QMenu *locationMenu = new QMenu( mInformationButton );

  const Qgis::GpsInformationComponents visibleComponents = settingShowInToolbar.value();

  for ( const auto &it : std::vector< std::pair< Qgis::GpsInformationComponent, QString> >
{
  { Qgis::GpsInformationComponent::Location, tr( "Show Location" ) },
    { Qgis::GpsInformationComponent::Altitude, tr( "Show Altitude (Geoid)" ) },
    { Qgis::GpsInformationComponent::EllipsoidAltitude, tr( "Show Altitude (WGS-84 Ellipsoid)" ) },
    { Qgis::GpsInformationComponent::GroundSpeed, tr( "Show Ground Speed" ) },
    { Qgis::GpsInformationComponent::Bearing, tr( "Show Bearing" ) },
    { Qgis::GpsInformationComponent::TotalTrackLength, tr( "Show Total Track Length" ) },
    { Qgis::GpsInformationComponent::TrackDistanceFromStart, tr( "Show Distance from Start of Track" ) }

  } )
  {
    const Qgis::GpsInformationComponent component = it.first;
    QAction *showComponentAction = new QAction( it.second, locationMenu );
    showComponentAction->setCheckable( true );
    showComponentAction->setData( QVariant::fromValue( component ) );
    showComponentAction->setChecked( visibleComponents & component );
    locationMenu->addAction( showComponentAction );

    connect( showComponentAction, &QAction::toggled, this, [ = ]( bool checked )
    {
      const Qgis::GpsInformationComponents currentVisibleComponents = settingShowInToolbar.value();
      if ( checked )
      {
        settingShowInToolbar.setValue( currentVisibleComponents | component );
      }
      else
      {
        settingShowInToolbar.setValue( currentVisibleComponents & ~( static_cast< int >( component ) ) );
      }
      updateLocationLabel();
    } );
  }

  mInformationButton->setMenu( locationMenu );
  mInformationButton->setAutoRaise( true );
  mInformationButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  insertWidget( mSettingsMenuAction, mInformationButton );
}

void QgsGpsToolBar::adjustSize()
{
  // this is necessary to ensure that the toolbar in floating mode correctly resizes to fit the label!
  if ( isFloating() )
    setFixedWidth( sizeHint().width() );
  else
    setFixedWidth( QWIDGETSIZE_MAX );
}

