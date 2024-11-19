/***************************************************************************
  qgs3ddebugwidget.cpp
  --------------------------------------
  Date                 : November 2024
  Copyright            : (C) 2024 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"

#include "moc_qgs3ddebugwidget.cpp"
#include "qgs3ddebugwidget.h"
#include "qgs3dmapcanvas.h"
#include "qgscameracontroller.h"

Qgs3DDebugWidget::Qgs3DDebugWidget( Qgs3DMapCanvas *canvas, QWidget *parent ) : QWidget( parent ), m3DMapCanvas( canvas )
{
  // set up the widget defined in ui file
  setupUi( this );

  // set up the fixed width of debug widget
  mCameraInfoGroupBox->setMinimumWidth( mCameraInfoGroupBox->sizeHint().width() );
  mCameraInfoGroupBox->adjustSize();
  scrollAreaWidgetContents->setMinimumWidth( scrollAreaWidgetContents->sizeHint().width() );
  scrollAreaWidgetContents->adjustSize();
  scrollArea->setMinimumWidth( scrollArea->sizeHint().width() );
  scrollArea->adjustSize();
  this->adjustSize();

  // set up the shadow map block
  mDebugShadowMapCornerComboBox->addItem( tr( "Top Left" ) );
  mDebugShadowMapCornerComboBox->addItem( tr( "Top Right" ) );
  mDebugShadowMapCornerComboBox->addItem( tr( "Bottom Left" ) );
  mDebugShadowMapCornerComboBox->addItem( tr( "Bottom Right" ) );
  mDebugShadowMapSizeSpinBox->setClearValue( 0.1 );

  // set up the depth map block
  mDebugDepthMapCornerComboBox->addItem( tr( "Top Left" ) );
  mDebugDepthMapCornerComboBox->addItem( tr( "Top Right" ) );
  mDebugDepthMapCornerComboBox->addItem( tr( "Bottom Left" ) );
  mDebugDepthMapCornerComboBox->addItem( tr( "Bottom Right" ) );
  mDebugDepthMapSizeSpinBox->setClearValue( 0.1 );

  // set up the camera info block
  mNearPlane->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mFarPlane->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraX->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraY->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraZ->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mLookingX->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mLookingY->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mLookingZ->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
}

/**
 * Sets up the interactive elements with values from Qgs3DMapSettings
 */
void Qgs3DDebugWidget::setMapSettings( Qgs3DMapSettings *mapSettings )
{
  mMap = mapSettings;
  whileBlocking( chkShowTileInfo )->setChecked( mMap->showTerrainTilesInfo() );
  whileBlocking( chkShowBoundingBoxes )->setChecked( mMap->showTerrainBoundingBoxes() );
  whileBlocking( chkShowCameraViewCenter )->setChecked( mMap->showCameraViewCenter() );
  whileBlocking( chkShowCameraRotationCenter )->setChecked( mMap->showCameraRotationCenter() );
  whileBlocking( chkShowLightSourceOrigins )->setChecked( mMap->showLightSourceOrigins() );
  whileBlocking( chkStopUpdates )->setChecked( mMap->stopUpdates() );
  whileBlocking( chkDebugOverlay )->setChecked( mMap->isDebugOverlayEnabled() );
  connect( chkShowTileInfo, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setShowTerrainTilesInfo( enabled ); } );
  connect( chkShowBoundingBoxes, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setShowTerrainBoundingBoxes( enabled ); } );
  connect( chkShowCameraViewCenter, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setShowCameraViewCenter( enabled ); } );
  connect( chkShowCameraRotationCenter, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setShowCameraRotationCenter( enabled ); } );
  connect( chkShowLightSourceOrigins, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setShowLightSourceOrigins( enabled ); } );
  connect( chkStopUpdates, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setStopUpdates( enabled ); } );
  connect( chkDebugOverlay, &QCheckBox::toggled, this, [ = ]( const bool enabled ) {mMap->setIsDebugOverlayEnabled( enabled ); } );

  whileBlocking( mDebugShadowMapGroupBox )->setChecked( mMap->debugShadowMapEnabled() );
  whileBlocking( mDebugShadowMapCornerComboBox )->setCurrentIndex( mMap->debugShadowMapCorner() );
  whileBlocking( mDebugShadowMapSizeSpinBox )->setValue( mMap->debugShadowMapSize() );
  // Do not display the shadow debug map if the shadow effect is not enabled.
  connect( mDebugShadowMapGroupBox, &QGroupBox::toggled, this, [ = ]( const bool enabled )
  {
    mMap->setDebugShadowMapSettings( enabled && mMap->shadowSettings().renderShadows(), static_cast<Qt::Corner>( mDebugShadowMapCornerComboBox->currentIndex() ), mDebugShadowMapSizeSpinBox->value() );
  } );
  connect( mDebugShadowMapCornerComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( const int index )
  {
    mMap->setDebugShadowMapSettings( mDebugShadowMapGroupBox->isChecked() && mMap->shadowSettings().renderShadows(), static_cast<Qt::Corner>( index ), mDebugShadowMapSizeSpinBox->value() );
  } );
  connect( mDebugShadowMapSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [ = ]( const double value )
  {
    mMap->setDebugShadowMapSettings( mDebugShadowMapGroupBox->isChecked() && mMap->shadowSettings().renderShadows(), static_cast<Qt::Corner>( mDebugShadowMapCornerComboBox->currentIndex() ), value );
  } );

  whileBlocking( mDebugDepthMapGroupBox )->setChecked( mMap->debugDepthMapEnabled() );
  whileBlocking( mDebugDepthMapCornerComboBox )->setCurrentIndex( mMap->debugDepthMapCorner() );
  whileBlocking( mDebugDepthMapSizeSpinBox )->setValue( mMap->debugDepthMapSize() );
  connect( mDebugDepthMapGroupBox, &QGroupBox::toggled, this, [ = ]( const bool enabled )
  {
    mMap->setDebugDepthMapSettings( enabled, static_cast<Qt::Corner>( mDebugDepthMapCornerComboBox->currentIndex() ), mDebugDepthMapSizeSpinBox->value() );
  } );
  connect( mDebugDepthMapCornerComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( const int index )
  {
    mMap->setDebugDepthMapSettings( mDebugDepthMapGroupBox->isChecked(), static_cast<Qt::Corner>( index ), mDebugDepthMapSizeSpinBox->value() );
  } );
  connect( mDebugDepthMapSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [ = ]( const double value )
  {
    mMap->setDebugDepthMapSettings( mDebugDepthMapGroupBox->isChecked(), static_cast<Qt::Corner>( mDebugDepthMapCornerComboBox->currentIndex() ), value );
  } );
}

/**
   * Update the state of navigation widget from camera's state
   */
void Qgs3DDebugWidget::updateFromCamera() const
{
  mNearPlane->setValue( m3DMapCanvas->cameraController()->camera()->nearPlane() );
  mFarPlane->setValue( m3DMapCanvas->cameraController()->camera()->farPlane() );
  mCameraX->setValue( m3DMapCanvas->cameraController()->camera()->position().x() );
  mCameraY->setValue( m3DMapCanvas->cameraController()->camera()->position().y() );
  mCameraZ->setValue( m3DMapCanvas->cameraController()->camera()->position().z() );
  mLookingX->setValue( m3DMapCanvas->cameraController()->lookingAtPoint().x() );
  mLookingY->setValue( m3DMapCanvas->cameraController()->lookingAtPoint().y() );
  mLookingZ->setValue( m3DMapCanvas->cameraController()->lookingAtPoint().z() );
}
