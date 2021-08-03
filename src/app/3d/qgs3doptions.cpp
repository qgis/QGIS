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
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgis.h"
#include "qgsgui.h"
#include "qgscameracontroller.h"
#include <Qt3DRender/QCamera>

//
// Qgs3DOptionsWidget
//

Qgs3DOptionsWidget::Qgs3DOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  mCameraNavigationModeCombo->addItem( tr( "Terrain Based" ), QgsCameraController::TerrainBasedNavigation );
  mCameraNavigationModeCombo->addItem( tr( "Walk Mode (First Person)" ), QgsCameraController::WalkNavigation );

  cboCameraProjectionType->addItem( tr( "Perspective Projection" ), Qt3DRender::QCameraLens::PerspectiveProjection );
  cboCameraProjectionType->addItem( tr( "Orthogonal Projection" ), Qt3DRender::QCameraLens::OrthographicProjection );

  mInvertVerticalAxisCombo->addItem( tr( "Never" ), QgsCameraController::Never );
  mInvertVerticalAxisCombo->addItem( tr( "Only When Dragging" ), QgsCameraController::WhenDragging );
  mInvertVerticalAxisCombo->addItem( tr( "Always" ), QgsCameraController::Always );

  mCameraMovementSpeed->setClearValue( 4 );
  spinCameraFieldOfView->setClearValue( 45.0 );

  QgsSettings settings;
  const QgsCameraController::NavigationMode defaultNavMode = settings.enumValue( QStringLiteral( "map3d/defaultNavigation" ), QgsCameraController::TerrainBasedNavigation, QgsSettings::App );
  mCameraNavigationModeCombo->setCurrentIndex( mCameraNavigationModeCombo->findData( static_cast< int >( defaultNavMode ) ) );

  const QgsCameraController::VerticalAxisInversion axisInversion = settings.enumValue( QStringLiteral( "map3d/axisInversion" ), QgsCameraController::WhenDragging, QgsSettings::App );
  mInvertVerticalAxisCombo->setCurrentIndex( mInvertVerticalAxisCombo->findData( static_cast< int >( axisInversion ) ) );

  const Qt3DRender::QCameraLens::ProjectionType defaultProjection = settings.enumValue( QStringLiteral( "map3d/defaultProjection" ), Qt3DRender::QCameraLens::PerspectiveProjection, QgsSettings::App );
  cboCameraProjectionType->setCurrentIndex( cboCameraProjectionType->findData( static_cast< int >( defaultProjection ) ) );

  mCameraMovementSpeed->setValue( settings.value( QStringLiteral( "map3d/defaultMovementSpeed" ), 5, QgsSettings::App ).toDouble() );
  spinCameraFieldOfView->setValue( settings.value( QStringLiteral( "map3d/defaultFieldOfView" ), 45, QgsSettings::App ).toInt() );
}

void Qgs3DOptionsWidget::apply()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "map3d/defaultNavigation" ), static_cast< QgsCameraController::NavigationMode >( mCameraNavigationModeCombo->currentData().toInt() ), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/axisInversion" ), static_cast< QgsCameraController::VerticalAxisInversion >( mInvertVerticalAxisCombo->currentData().toInt() ), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/defaultProjection" ), static_cast< Qt3DRender::QCameraLens::ProjectionType >( cboCameraProjectionType->currentData().toInt() ), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/defaultMovementSpeed" ), mCameraMovementSpeed->value(), QgsSettings::App );
  settings.setValue( QStringLiteral( "map3d/defaultFieldOfView" ), spinCameraFieldOfView->value(), QgsSettings::App );
}


//
// Qgs3DOptionsFactory
//
Qgs3DOptionsFactory::Qgs3DOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "3D" ), QIcon() )
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
