/***************************************************************************
    qgsappgpssettingsmenu.cpp
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

#include "qgsappgpssettingsmenu.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgsfieldproxymodel.h"
#include "qgsfieldmodel.h"
#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "qgsgpsmarker.h"
#include "qgsappgpsdigitizing.h"
#include "qgsproject.h"
#include "qgsprojectgpssettings.h"
#include "qgsappgpslogging.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QFileDialog>

QgsGpsMapRotationAction::QgsGpsMapRotationAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 8, 2, 3, 2 );

  mRadioAlwaysRecenter = new QRadioButton( "Always Recenter Map", parent );
  mRadioRecenterWhenOutside = new QRadioButton( "Recenter Map When Leaving Extent", parent );
  mRadioNeverRecenter = new QRadioButton( "Never Recenter", parent );
  QButtonGroup *recenterGroup = new QButtonGroup( this );
  recenterGroup->addButton( mRadioAlwaysRecenter );
  recenterGroup->addButton( mRadioRecenterWhenOutside );
  recenterGroup->addButton( mRadioNeverRecenter );

  gLayout->addWidget( mRadioAlwaysRecenter, 0, 0, 1, 2 );
  gLayout->addWidget( mRadioRecenterWhenOutside, 1, 0, 1, 2 );
  gLayout->addWidget( mRadioNeverRecenter, 2, 0, 1, 2 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}

//
// QgsAppGpsSettingsMenu
//

QgsAppGpsSettingsMenu::QgsAppGpsSettingsMenu( QWidget *parent )
  : QMenu( parent )
{
  bool showLocationMarker = true;
  bool showBearingLine = false;
  Qgis::MapRecenteringMode mapCenteringMode = Qgis::MapRecenteringMode::WhenOutsideVisibleExtent;
  bool rotateMap = false;

  if ( QgsGpsCanvasBridge::settingShowBearingLine.exists( ) )
  {
    showLocationMarker = QgsGpsMarker::settingShowLocationMarker.value();
    showBearingLine = QgsGpsCanvasBridge::settingShowBearingLine.value();
    mapCenteringMode = QgsGpsCanvasBridge::settingMapCenteringMode.value();
    rotateMap = QgsGpsCanvasBridge::settingRotateMap.value();
  }
  else
  {
    // migrate old settings
    QgsSettings settings;
    showLocationMarker = settings.value( QStringLiteral( "showMarker" ), "true", QgsSettings::Gps ).toBool();
    showBearingLine = settings.value( QStringLiteral( "showBearingLine" ), false, QgsSettings::Gps ).toBool();
    rotateMap = settings.value( QStringLiteral( "rotateMap" ), false, QgsSettings::Gps ).toBool();

    const QString panMode = settings.value( QStringLiteral( "panMode" ), "recenterWhenNeeded", QgsSettings::Gps ).toString();
    if ( panMode == QLatin1String( "none" ) )
    {
      mapCenteringMode = Qgis::MapRecenteringMode::Never;
    }
    else if ( panMode == QLatin1String( "recenterAlways" ) )
    {
      mapCenteringMode = Qgis::MapRecenteringMode::Always;
    }
    else
    {
      mapCenteringMode = Qgis::MapRecenteringMode::WhenOutsideVisibleExtent;
    }
  }

  mShowLocationMarkerAction = new QAction( tr( "Show Location Marker" ), this );
  mShowLocationMarkerAction->setCheckable( true );
  mShowLocationMarkerAction->setChecked( showLocationMarker );
  connect( mShowLocationMarkerAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit locationMarkerToggled( checked );
    QgsGpsMarker::settingShowLocationMarker.setValue( checked );
  } );
  addAction( mShowLocationMarkerAction );


  mShowBearingLineAction = new QAction( tr( "Show Bearing Line" ), this );
  mShowBearingLineAction->setCheckable( true );
  mShowBearingLineAction->setChecked( showBearingLine );
  connect( mShowBearingLineAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit bearingLineToggled( checked );
    QgsGpsCanvasBridge::settingShowBearingLine.setValue( checked );
  } );

  addAction( mShowBearingLineAction );


  mRotateMapAction = new QAction( tr( "Rotate Map to Match GPS Direction" ), this );
  mRotateMapAction->setCheckable( true );
  mRotateMapAction->setChecked( rotateMap );
  connect( mRotateMapAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    QgsGpsCanvasBridge::settingRotateMap.setValue( checked );
    emit rotateMapToggled( checked );
  } );

  addAction( mRotateMapAction );

  QgsGpsMapRotationAction *rotateAction = new QgsGpsMapRotationAction( this );
  mRadioAlwaysRecenter = rotateAction->radioAlwaysRecenter();
  mRadioRecenterWhenOutside = rotateAction->radioRecenterWhenOutside();
  mRadioNeverRecenter = rotateAction->radioNeverRecenter();

  //pan mode
  switch ( mapCenteringMode )
  {
    case Qgis::MapRecenteringMode::Always:
      mRadioAlwaysRecenter->setChecked( true );
      break;
    case Qgis::MapRecenteringMode::WhenOutsideVisibleExtent:
      mRadioRecenterWhenOutside->setChecked( true );
      break;
    case Qgis::MapRecenteringMode::Never:
      mRadioNeverRecenter->setChecked( true );
      break;
  }

  connect( mRadioAlwaysRecenter, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      QgsGpsCanvasBridge::settingMapCenteringMode.setValue( Qgis::MapRecenteringMode::Always );
      emit mapCenteringModeChanged( Qgis::MapRecenteringMode::Always );
    }
  } );

  connect( mRadioRecenterWhenOutside, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      QgsGpsCanvasBridge::settingMapCenteringMode.setValue( Qgis::MapRecenteringMode::WhenOutsideVisibleExtent );
      emit mapCenteringModeChanged( Qgis::MapRecenteringMode::WhenOutsideVisibleExtent );
    }
  } );

  connect( mRadioNeverRecenter, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      QgsGpsCanvasBridge::settingMapCenteringMode.setValue( Qgis::MapRecenteringMode::Never );
      emit mapCenteringModeChanged( Qgis::MapRecenteringMode::Never );
    }
  } );

  addSeparator();
  addAction( rotateAction );

  addSeparator();

  mAutoAddTrackVerticesAction = new QAction( tr( "Automatically Add Track Vertices" ), this );
  mAutoAddTrackVerticesAction->setCheckable( true );
  mAutoAddTrackVerticesAction->setChecked( QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() );
  connect( mAutoAddTrackVerticesAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    if ( checked != QgsProject::instance()->gpsSettings()->automaticallyAddTrackVertices() )
    {
      QgsProject::instance()->gpsSettings()->setAutomaticallyAddTrackVertices( checked );
      QgsProject::instance()->setDirty();
    }
  } );
  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::automaticallyAddTrackVerticesChanged, mAutoAddTrackVerticesAction, &QAction::setChecked );

  addAction( mAutoAddTrackVerticesAction );

  mAutoSaveAddedFeatureAction = new QAction( tr( "Automatically Save Added Feature" ), this );
  mAutoSaveAddedFeatureAction->setCheckable( true );
  mAutoSaveAddedFeatureAction->setChecked( QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() );
  connect( mAutoSaveAddedFeatureAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    if ( checked != QgsProject::instance()->gpsSettings()->automaticallyCommitFeatures() )
    {
      QgsProject::instance()->gpsSettings()->setAutomaticallyCommitFeatures( checked );
      QgsProject::instance()->setDirty();
    }
  } );
  connect( QgsProject::instance()->gpsSettings(), &QgsProjectGpsSettings::automaticallyCommitFeaturesChanged, mAutoSaveAddedFeatureAction, &QAction::setChecked );

  addAction( mAutoSaveAddedFeatureAction );

  mFieldProxyModel = new QgsFieldProxyModel( this );
  mFieldProxyModel->sourceFieldModel()->setAllowEmptyFieldName( true );
  mFieldProxyModel->setFilters( QgsFieldProxyModel::Filter::String | QgsFieldProxyModel::Filter::DateTime );

  mTimeStampDestinationFieldMenu = new QMenu( tr( "Time Stamp Destination" ), this );
  connect( mTimeStampDestinationFieldMenu, &QMenu::aboutToShow, this, &QgsAppGpsSettingsMenu::timeStampMenuAboutToShow );
  addMenu( mTimeStampDestinationFieldMenu );

  addSeparator();

  mActionGpkgLog = new QAction( "Log to GeoPackage/Spatialite…" );
  mActionGpkgLog->setCheckable( true );
  connect( mActionGpkgLog, &QAction::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      const QString lastGpkgLog = QgsAppGpsLogging::settingLastGpkgLog.value();
      const QString initialPath = lastGpkgLog.isEmpty() ? QDir::homePath() : lastGpkgLog;

      QString selectedFilter;
      QString fileName = QFileDialog::getSaveFileName( this, tr( "GPS Log File" ), initialPath,
                         tr( "GeoPackage" ) + " (*.gpkg *.GPKG);;" + tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db);;",
                         &selectedFilter, QFileDialog::Option::DontConfirmOverwrite );
      if ( fileName.isEmpty() )
      {
        mActionGpkgLog->setChecked( false );
        emit gpkgLogDestinationChanged( QString() );
        return;
      }

      fileName = QgsFileUtils::addExtensionFromFilter( fileName, selectedFilter );
      QgsAppGpsLogging::settingLastGpkgLog.setValue( fileName );

      emit gpkgLogDestinationChanged( fileName );
    }
    else
    {
      emit gpkgLogDestinationChanged( QString() );
    }
  } );
  addAction( mActionGpkgLog );

  mActionNmeaLog = new QAction( "Log NMEA Sentences…" );
  mActionNmeaLog->setCheckable( true );
  connect( mActionNmeaLog, &QAction::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      const QString lastLogFolder = QgsAppGpsLogging::settingLastLogFolder.value();
      const QString initialFolder = lastLogFolder.isEmpty() ? QDir::homePath() : lastLogFolder;

      QString fileName = QFileDialog::getSaveFileName( this, tr( "GPS Log File" ), initialFolder, tr( "NMEA files" ) + " (*.nmea)" );
      if ( fileName.isEmpty() )
      {
        mActionNmeaLog->setChecked( false );
        emit enableNmeaLog( false );
        return;
      }

      fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { QStringLiteral( "nmea" ) } );
      QgsAppGpsLogging::settingLastLogFolder.setValue( QFileInfo( fileName ).absolutePath() );

      emit nmeaLogFileChanged( fileName );
      emit enableNmeaLog( true );
    }
    else
    {
      emit enableNmeaLog( false );
    }
  } );
  addAction( mActionNmeaLog );

  addSeparator();

  QAction *settingsAction = new QAction( tr( "GPS Settings…" ), this );
  settingsAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  connect( settingsAction, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->showOptionsDialog( QgisApp::instance(), QStringLiteral( "mGpsOptions" ) );
  } );

  addAction( settingsAction );
}

bool QgsAppGpsSettingsMenu::locationMarkerVisible() const
{
  return mShowLocationMarkerAction->isChecked();
}

bool QgsAppGpsSettingsMenu::bearingLineVisible() const
{
  return mShowBearingLineAction->isChecked();
}

bool QgsAppGpsSettingsMenu::rotateMap() const
{
  return mRotateMapAction->isChecked();
}

Qgis::MapRecenteringMode QgsAppGpsSettingsMenu::mapCenteringMode() const
{
  // pan mode
  if ( mRadioAlwaysRecenter->isChecked() )
  {
    return Qgis::MapRecenteringMode::Always;
  }
  else if ( mRadioRecenterWhenOutside->isChecked() )
  {
    return Qgis::MapRecenteringMode::WhenOutsideVisibleExtent;
  }
  else
  {
    return Qgis::MapRecenteringMode::Never;
  }
}

void QgsAppGpsSettingsMenu::onGpkgLoggingFailed()
{
  mActionGpkgLog->setChecked( false );
}

void QgsAppGpsSettingsMenu::timeStampMenuAboutToShow()
{
  mFieldProxyModel->sourceFieldModel()->setLayer( QgsProject::instance()->gpsSettings()->destinationLayer() );
  const QString currentTimeStampField = QgsProject::instance()->gpsSettings()->destinationTimeStampField();

  mTimeStampDestinationFieldMenu->clear();
  bool foundPreviousField = false;
  for ( int row = 0; row < mFieldProxyModel->rowCount(); ++row )
  {
    QAction *fieldAction = new QAction( mFieldProxyModel->data( mFieldProxyModel->index( row, 0 ) ).toString(), this );
    if ( fieldAction->text().isEmpty() )
    {
      fieldAction->setText( tr( "Do Not Store" ) );
    }
    fieldAction->setIcon( mFieldProxyModel->data( mFieldProxyModel->index( row, 0 ), Qt::DecorationRole ).value< QIcon >() );
    const QString fieldName = mFieldProxyModel->data( mFieldProxyModel->index( row, 0 ), QgsFieldModel::FieldNameRole ).toString();
    fieldAction->setData( fieldName );
    fieldAction->setCheckable( true );
    if ( currentTimeStampField == fieldName )
    {
      foundPreviousField = true;
      fieldAction->setChecked( currentTimeStampField == fieldName );
    }
    connect( fieldAction, &QAction::triggered, this, [ = ]()
    {
      if ( QgsProject::instance()->gpsSettings()->destinationTimeStampField() != fieldName )
      {
        QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( QgsProject::instance()->gpsSettings()->destinationLayer(), fieldName );
        QgsProject::instance()->setDirty();
      }
    } );
    mTimeStampDestinationFieldMenu->addAction( fieldAction );
  }

  if ( !foundPreviousField )
  {
    mTimeStampDestinationFieldMenu->actions().at( 0 )->setChecked( true );
  }
}

