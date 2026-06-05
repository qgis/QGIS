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

#include "qgis.h"
#include "qgisapp.h"
#include "qgs3d.h"
#include "qgs3dmapcanvas.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"

#include <QString>
#include <Qt3DRender/QCamera>

#include "moc_qgs3doptions.cpp"

using namespace Qt::StringLiterals;

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

  mVerticalAxisInversionComboBox->setDefaultText( tr( "Do not invert" ) );
  mVerticalAxisInversionComboBox->addItem( tr( "When rotating (mouse captured)" ), QVariant::fromValue( Qgis::VerticalAxisInversion::WhenRotatingCaptured ) );
  mVerticalAxisInversionComboBox->addItem( tr( "When rotating (when dragging)" ), QVariant::fromValue( Qgis::VerticalAxisInversion::WhenRotatingDragging ) );
  mVerticalAxisInversionComboBox->addItem( tr( "When pivoting around terrain" ), QVariant::fromValue( Qgis::VerticalAxisInversion::WhenPivoting ) );

  mTextureFilterQualityCombo->addItem( tr( "Off (Trilinear)" ), QVariant::fromValue( Qgis::TextureFilterQuality::Trilinear ) );
  mTextureFilterQualityCombo->addItem( tr( "2×" ), QVariant::fromValue( Qgis::TextureFilterQuality::Anisotropic2x ) );
  mTextureFilterQualityCombo->addItem( tr( "4×" ), QVariant::fromValue( Qgis::TextureFilterQuality::Anisotropic4x ) );
  mTextureFilterQualityCombo->addItem( tr( "8×" ), QVariant::fromValue( Qgis::TextureFilterQuality::Anisotropic8x ) );
  mTextureFilterQualityCombo->addItem( tr( "16×" ), QVariant::fromValue( Qgis::TextureFilterQuality::Anisotropic16x ) );

  mShadowQualityCombo->addItem( tr( "Low" ), QVariant::fromValue( Qgis::ShadowQuality::Low ) );
  mShadowQualityCombo->addItem( tr( "Medium" ), QVariant::fromValue( Qgis::ShadowQuality::Medium ) );
  mShadowQualityCombo->addItem( tr( "High" ), QVariant::fromValue( Qgis::ShadowQuality::High ) );
  mShadowQualityCombo->addItem( tr( "Very High" ), QVariant::fromValue( Qgis::ShadowQuality::VeryHigh ) );
  mShadowQualityCombo->addItem( tr( "Extreme" ), QVariant::fromValue( Qgis::ShadowQuality::Extreme ) );

  mCameraMovementSpeed->setClearValue( 4 );
  spinCameraFieldOfView->setClearValue( 45.0 );

  QgsSettings settings;
  const Qgis::NavigationMode defaultNavMode = settings.enumValue( u"map3d/defaultNavigation"_s, Qgis::NavigationMode::TerrainBased, QgsSettings::App );
  mCameraNavigationModeCombo->setCurrentIndex( mCameraNavigationModeCombo->findData( QVariant::fromValue( defaultNavMode ) ) );

  const Qgis::VerticalAxisInversionFlags axisInversion
    = settings.flagValue( u"map3d/axisInversion"_s, Qgis::VerticalAxisInversion::WhenPivoting | Qgis::VerticalAxisInversion::WhenRotatingDragging, QgsSettings::App );
  mVerticalAxisInversionComboBox->setItemCheckState( 0, ( axisInversion & Qgis::VerticalAxisInversion::WhenRotatingCaptured ) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
  mVerticalAxisInversionComboBox->setItemCheckState( 1, ( axisInversion & Qgis::VerticalAxisInversion::WhenRotatingDragging ) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
  mVerticalAxisInversionComboBox->setItemCheckState( 2, ( axisInversion & Qgis::VerticalAxisInversion::WhenPivoting ) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

  const Qt3DRender::QCameraLens::ProjectionType defaultProjection = settings.enumValue( u"map3d/defaultProjection"_s, Qt3DRender::QCameraLens::PerspectiveProjection, QgsSettings::App );
  cboCameraProjectionType->setCurrentIndex( cboCameraProjectionType->findData( static_cast<int>( defaultProjection ) ) );

  mCameraMovementSpeed->setValue( settings.value( u"map3d/defaultMovementSpeed"_s, 5, QgsSettings::App ).toDouble() );
  spinCameraFieldOfView->setValue( settings.value( u"map3d/defaultFieldOfView"_s, 45, QgsSettings::App ).toInt() );

  mGpuMemoryLimit->setClearValue( 500 );
  mGpuMemoryLimit->setValue( settings.value( u"map3d/gpuMemoryLimit"_s, 500.0, QgsSettings::App ).toDouble() );

  mMSAA->setChecked( Qgs3D::settingMsaaEnabled->value() );

  mTextureFilterQualityCombo->setCurrentIndex( mTextureFilterQualityCombo->findData( QVariant::fromValue( Qgs3D::settingTextureFilterQuality->value() ) ) );
  mShadowQualityCombo->setCurrentIndex( mShadowQualityCombo->findData( QVariant::fromValue( Qgs3D::settingShadowQuality->value() ) ) );
}

QString Qgs3DOptionsWidget::helpKey() const
{
  // typo IS correct here!
  return u"introduction/qgis_configuration.html#d-options"_s;
}

void Qgs3DOptionsWidget::apply()
{
  QgsSettings settings;
  settings.setEnumValue( u"map3d/defaultNavigation"_s, mCameraNavigationModeCombo->currentData().value<Qgis::NavigationMode>(), QgsSettings::App );
  settings.setValue( u"map3d/defaultProjection"_s, static_cast<Qt3DRender::QCameraLens::ProjectionType>( cboCameraProjectionType->currentData().toInt() ), QgsSettings::App );
  settings.setValue( u"map3d/defaultMovementSpeed"_s, mCameraMovementSpeed->value(), QgsSettings::App );
  settings.setValue( u"map3d/defaultFieldOfView"_s, spinCameraFieldOfView->value(), QgsSettings::App );

  settings.setValue( u"map3d/gpuMemoryLimit"_s, mGpuMemoryLimit->value(), QgsSettings::App );

  Qgs3D::settingMsaaEnabled->setValue( mMSAA->isChecked() );
  Qgs3D::settingTextureFilterQuality->setValue( mTextureFilterQualityCombo->currentData().value< Qgis::TextureFilterQuality >() );
  Qgs3D::settingShadowQuality->setValue( mShadowQualityCombo->currentData().value< Qgis::ShadowQuality >() );

  Qgis::VerticalAxisInversionFlags axisInversion;
  for ( QVariant flag : mVerticalAxisInversionComboBox->checkedItemsData() )
    axisInversion.setFlag( flag.value<Qgis::VerticalAxisInversion>() );

  settings.setFlagValue( u"map3d/axisInversion"_s, axisInversion, QgsSettings::App );

  // Apply axis inversion setting to existing map views
  for ( Qgs3DMapCanvas *canvas : QgisApp::instance()->mapCanvases3D() )
  {
    if ( QgsCameraController *cameraController = canvas->cameraController() )
      cameraController->setVerticalAxisInversion( axisInversion );
  }
}


//
// Qgs3DOptionsFactory
//
Qgs3DOptionsFactory::Qgs3DOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "3D" ), QIcon(), u"3d"_s )
{}

QIcon Qgs3DOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( u"/3d.svg"_s );
}

QgsOptionsPageWidget *Qgs3DOptionsFactory::createWidget( QWidget *parent ) const
{
  return new Qgs3DOptionsWidget( parent );
}

QString Qgs3DOptionsFactory::pagePositionHint() const
{
  return u"mOptionsPageColors"_s;
}
