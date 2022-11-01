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
#include "qgsgpsinformationwidget.h"
#include "qgsfileutils.h"
#include "qgsapplication.h"

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
  QgsSettings settings;

  mShowLocationMarkerAction = new QAction( tr( "Show Location Marker" ), this );
  mShowLocationMarkerAction->setCheckable( true );
  mShowLocationMarkerAction->setChecked( settings.value( QStringLiteral( "showMarker" ), "true", QgsSettings::Gps ).toBool() );
  connect( mShowLocationMarkerAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit locationMarkerToggled( checked );
    QgsSettings settings;
    settings.setValue( QStringLiteral( "showMarker" ), checked, QgsSettings::Gps );
  } );
  addAction( mShowLocationMarkerAction );


  mShowBearingLineAction = new QAction( tr( "Show Bearing Line" ), this );
  mShowBearingLineAction->setCheckable( true );
  mShowBearingLineAction->setChecked( settings.value( QStringLiteral( "showBearingLine" ), false, QgsSettings::Gps ).toBool() );
  connect( mShowBearingLineAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit bearingLineToggled( checked );
    QgsSettings settings;
    settings.setValue( QStringLiteral( "showBearingLine" ), checked, QgsSettings::Gps );
  } );

  addAction( mShowBearingLineAction );


  mRotateMapAction = new QAction( tr( "Rotate map to match GPS direction" ), this );
  mRotateMapAction->setCheckable( true );
  mRotateMapAction->setChecked( settings.value( QStringLiteral( "rotateMap" ), false, QgsSettings::Gps ).toBool() );
  connect( mRotateMapAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit rotateMapToggled( checked );
    QgsSettings settings;
    settings.setValue( QStringLiteral( "rotateMap" ), checked, QgsSettings::Gps );
  } );

  addAction( mRotateMapAction );

  QgsGpsMapRotationAction *rotateAction = new QgsGpsMapRotationAction( this );
  mRadioAlwaysRecenter = rotateAction->radioAlwaysRecenter();
  mRadioRecenterWhenOutside = rotateAction->radioRecenterWhenOutside();
  mRadioNeverRecenter = rotateAction->radioNeverRecenter();

  //pan mode
  const QString panMode = settings.value( QStringLiteral( "panMode" ), "recenterWhenNeeded", QgsSettings::Gps ).toString();
  if ( panMode == QLatin1String( "none" ) )
  {
    mRadioNeverRecenter->setChecked( true );
  }
  else if ( panMode == QLatin1String( "recenterAlways" ) )
  {
    mRadioAlwaysRecenter->setChecked( true );
  }
  else
  {
    mRadioRecenterWhenOutside->setChecked( true );
  }

  connect( mRadioAlwaysRecenter, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "panMode" ), "recenterAlways", QgsSettings::Gps );
      emit mapCenteringModeChanged( QgsAppGpsSettingsMenu::MapCenteringMode::Always );
    }
  } );

  connect( mRadioRecenterWhenOutside, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "panMode" ), "recenterWhenNeeded", QgsSettings::Gps );
      emit mapCenteringModeChanged( QgsAppGpsSettingsMenu::MapCenteringMode::WhenLeavingExtent );
    }
  } );

  connect( mRadioNeverRecenter, &QRadioButton::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "panMode" ), "none", QgsSettings::Gps );
      emit mapCenteringModeChanged( QgsAppGpsSettingsMenu::MapCenteringMode::Never );
    }
  } );

  addSeparator();
  addAction( rotateAction );

  addSeparator();

  mAutoAddTrackPointAction = new QAction( tr( "Automatically Add Track Points" ), this );
  mAutoAddTrackPointAction->setCheckable( true );
  mAutoAddTrackPointAction->setChecked( settings.value( QStringLiteral( "autoAddVertices" ), "false", QgsSettings::Gps ).toBool() );
  connect( mAutoAddTrackPointAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit autoAddTrackPointsChanged( checked );
    QgsSettings settings;
    settings.setValue( QStringLiteral( "autoAddVertices" ), checked, QgsSettings::Gps );
  } );

  addAction( mAutoAddTrackPointAction );

  mAutoSaveAddedFeatureAction = new QAction( tr( "Automatically Save Added Feature" ), this );
  mAutoSaveAddedFeatureAction->setCheckable( true );
  mAutoSaveAddedFeatureAction->setChecked( settings.value( QStringLiteral( "autoCommit" ), "false", QgsSettings::Gps ).toBool() );
  connect( mAutoAddTrackPointAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    emit autoAddFeatureChanged( checked );
    QgsSettings settings;
    settings.setValue( QStringLiteral( "autoCommit" ), checked, QgsSettings::Gps );
  } );

  addAction( mAutoSaveAddedFeatureAction );

  mFieldProxyModel = new QgsFieldProxyModel( this );
  mFieldProxyModel->sourceFieldModel()->setAllowEmptyFieldName( true );
  mFieldProxyModel->setFilters( QgsFieldProxyModel::Filter::String | QgsFieldProxyModel::Filter::DateTime );

  mTimeStampFieldMenu = new QMenu( tr( "Time Stamp Destination" ), this );
  connect( mTimeStampFieldMenu, &QMenu::aboutToShow, this, &QgsAppGpsSettingsMenu::timeStampMenuAboutToShow );
  addMenu( mTimeStampFieldMenu );

  addSeparator();

  mActionNmeaLog = new QAction( "Log NMEA Sentences…" );
  mActionNmeaLog->setCheckable( true );
  connect( mActionNmeaLog, &QAction::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      const QString lastLogFolder = QgsGpsInformationWidget::settingLastLogFolder.value();
      const QString initialFolder = lastLogFolder.isEmpty() ? QDir::homePath() : lastLogFolder;

      QString fileName = QFileDialog::getSaveFileName( this, tr( "GPS Log File" ), initialFolder, tr( "NMEA files" ) + " (*.nmea)" );
      if ( fileName.isEmpty() )
      {
        mActionNmeaLog->setChecked( false );
        emit enableNmeaLog( false );
        return;
      }

      fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { QStringLiteral( "nmea" ) } );
      QgsGpsInformationWidget::settingLastLogFolder.setValue( QFileInfo( fileName ).absolutePath() );

      emit nmeaLogFileChanged( fileName );
      emit enableNmeaLog( true );
    }
    else
    {
      emit enableNmeaLog( false );
    }
  } );
  addAction( mActionNmeaLog );

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

QgsAppGpsSettingsMenu::MapCenteringMode QgsAppGpsSettingsMenu::mapCenteringMode() const
{
  // pan mode
  if ( mRadioAlwaysRecenter->isChecked() )
  {
    return QgsAppGpsSettingsMenu::MapCenteringMode::Always;
  }
  else if ( mRadioRecenterWhenOutside->isChecked() )
  {
    return QgsAppGpsSettingsMenu::MapCenteringMode::WhenLeavingExtent;
  }
  else
  {
    return QgsAppGpsSettingsMenu::MapCenteringMode::Never;
  }
}

bool QgsAppGpsSettingsMenu::autoAddTrackPoints() const
{
  return mAutoAddTrackPointAction->isChecked();
}

bool QgsAppGpsSettingsMenu::autoAddFeature() const
{
  return mAutoSaveAddedFeatureAction->isChecked();
}

void QgsAppGpsSettingsMenu::setCurrentTimeStampField( const QString &fieldName )
{
  mCurrentTimeStampField = fieldName;
}

void QgsAppGpsSettingsMenu::timeStampMenuAboutToShow()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  mFieldProxyModel->sourceFieldModel()->setLayer( vlayer );

  mTimeStampFieldMenu->clear();
  bool foundPreviousField = false;
  for ( int row = 0; row < mFieldProxyModel->rowCount(); ++row )
  {
    QAction *fieldAction = new QAction( mFieldProxyModel->data( mFieldProxyModel->index( row, 0 ) ).toString(), this );
    fieldAction->setIcon( mFieldProxyModel->data( mFieldProxyModel->index( row, 0 ), Qt::DecorationRole ).value< QIcon >() );
    const QString fieldName = mFieldProxyModel->data( mFieldProxyModel->index( row, 0 ), QgsFieldModel::FieldNameRole ).toString();
    fieldAction->setData( fieldName );
    fieldAction->setCheckable( true );
    if ( mCurrentTimeStampField == fieldName )
    {
      foundPreviousField = true;
      fieldAction->setChecked( mCurrentTimeStampField == fieldName );
    }
    connect( fieldAction, &QAction::triggered, this, [ = ]()
    {
      mCurrentTimeStampField = fieldName;
      emit timeStampDestinationChanged( fieldName );
    } );
    mTimeStampFieldMenu->addAction( fieldAction );
  }

  if ( !foundPreviousField )
  {
    mTimeStampFieldMenu->actions().at( 0 )->setChecked( true );
    mCurrentTimeStampField.clear();
  }
}

