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

#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>

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

