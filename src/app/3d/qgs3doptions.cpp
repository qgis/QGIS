/***************************************************************************
    qgs3doptions.cpp
    -------------------------
    begin                : January 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3doptions.h"
#include "moc_qgs3doptions.cpp"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgis.h"
#include <Qt3DRender/QCamera>

//
// Qgs3DOptionsWidget
//

Qgs3DOptionsWidget::Qgs3DOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  mCameraNavigationModeCombo->addItem( tr( "Terrain Based" ), QVariant::fromValue( Qgis::NavigationMode::TerrainBased ) );
  mCameraNavigationModeCombo->addItem( tr( "Walk Mode (First Person)" ), QVariant::fromValue( Qgis::NavigationMode::Walk ) );

  cboCameraProjectionType->addItem( tr( "Perspective Projection" ), Qt3DRender::QCameraLens::PerspectiveProjection );
  cboCameraProjectionType->addItem( tr( "Orthogonal Projection" ), Qt3DRender::QCameraLens::OrthographicProjection );

  mInvertVerticalAxisCombo->addItem( tr( "Never" ), QVariant::fromValue( Qgis::VerticalAxisInversion::Never ) );
  mInvertVerticalAxisCombo->addItem( tr( "Only When Dragging" ), QVariant::fromValue( Qgis::VerticalAxisInversion::WhenDragging ) );
  mInvertVerticalAxisCombo->addItem( tr( "Always" ), QVariant::fromValue( Qgis::VerticalAxisInversion::Always ) );

  mCameraMovementSpeed->setClearValue( 4 );
  spinCameraFieldOfView->setClearValue( 45.0 );

  QgsSettings settings;
  const Qgis::NavigationMode defaultNavMode = settings.enumValue( QStringLiteral( "map3d/defaultNavigation" ), Qgis::NavigationMode::TerrainBased, QgsSettings::App );
  mCameraNavigationModeCombo->setCurrentIndex( mCameraNavigationModeCombo->findData( QVariant::fromValue( defaultNavMode ) ) );

  const Qgis::VerticalAxisInversion axisInversion = settings.enumValue( QStringLiteral( "map3d/axisInversion" ), Qgis::VerticalAxisInversion::WhenDragging, QgsSettings::App );
  mInvertVerticalAxisCombo->setCurrentIndex( mInvertVerticalAxisCombo->findData( QVariant::fromValue( axisInversion ) ) );

  const Qt3DRender::QCameraLens::ProjectionType defaultProjection = settings.enumValue( QStringLiteral( "map3d/defaultProjection" ), Qt3DRender::QCameraLens::PerspectiveProjection, QgsSettings::App );
  cboCameraProjectionType->setCurrentIndex( cboCameraProjectionType->findData( static_cast<int>( defaultProjection ) ) );

  mCameraMovementSpeed->setValue( settings.value( QStringLiteral( "map3d/defaultMovementSpeed" ), 5, QgsSettings::App ).toDouble() );
  spinCameraFieldOfView->setValue( settings.value( QStringLiteral( "map3d/defaultFieldOfView" ), 45, QgsSettings::App ).toInt() );

  mGpuMemoryLimit->setClearValue( 500 );
  mGpuMemoryLimit->setValue( settings.value( QStringLiteral( "map3d/gpuMemoryLimit" ), 500.0, QgsSettings::App ).toDouble() );
}

QString Qgs3DOptionsWidget::helpKey() const
{
  // typo IS correct here!
  return QStringLiteral( "introduction/qgis_configuration.html#d-options" );
}

void Qgs3DOptionsWidget::apply()
{
  QgsSettings settings;
  settings.setEnumValue( QStringLiteral( "map3d/defaultNavigation" ), mCameraNavigationModeCombo->currentData().value<Qgis::NavigationMode>(), QgsSettings::App );
  settings.setEnumValue( QStringLiteral( "map3d/axisInversion" ), mInvertVerticalAxisCombo->currentData().value<Qgis::VerticalAxisInversion>(), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/defaultProjection" ), static_cast<Qt3DRender::QCameraLens::ProjectionType>( cboCameraProjectionType->currentData().toInt() ), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/defaultMovementSpeed" ), mCameraMovementSpeed->value(), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/defaultFieldOfView" ), spinCameraFieldOfView->value(), QgsSettings::App );

  settings.setValue( QStringLiteral( "map3d/gpuMemoryLimit" ), mGpuMemoryLimit->value(), QgsSettings::App );
}


//
// Qgs3DOptionsFactory
//
Qgs3DOptionsFactory::Qgs3DOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "3D" ), QIcon(), QStringLiteral( "3d" ) )
{
}

QIcon Qgs3DOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/3d.svg" ) );
}

QgsOptionsPageWidget *Qgs3DOptionsFactory::createWidget( QWidget *parent ) const
{
  return new Qgs3DOptionsWidget( parent );
}

QString Qgs3DOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsPageColors" );
}
