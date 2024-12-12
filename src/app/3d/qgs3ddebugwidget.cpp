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

Qgs3DDebugWidget::Qgs3DDebugWidget( Qgs3DMapCanvas *canvas, QWidget *parent )
  : QWidget( parent )
  , m3DMapCanvas( canvas )
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
  adjustSize();

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

void Qgs3DDebugWidget::setMapSettings( Qgs3DMapSettings *mapSettings )
{
  mMap = mapSettings;

  // set up the checkbox block
  whileBlocking( chkShowTileInfo )->setChecked( mMap->showTerrainTilesInfo() );
  whileBlocking( chkShowBoundingBoxes )->setChecked( mMap->showTerrainBoundingBoxes() );
  whileBlocking( chkShowCameraViewCenter )->setChecked( mMap->showCameraViewCenter() );
  whileBlocking( chkShowCameraRotationCenter )->setChecked( mMap->showCameraRotationCenter() );
  whileBlocking( chkShowLightSourceOrigins )->setChecked( mMap->showLightSourceOrigins() );
  whileBlocking( chkStopUpdates )->setChecked( mMap->stopUpdates() );
  whileBlocking( chkDebugOverlay )->setChecked( mMap->isDebugOverlayEnabled() );
  connect( chkShowTileInfo, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setShowTerrainTilesInfo( enabled ); } );
  connect( chkShowBoundingBoxes, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setShowTerrainBoundingBoxes( enabled ); } );
  connect( chkShowCameraViewCenter, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setShowCameraViewCenter( enabled ); } );
  connect( chkShowCameraRotationCenter, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setShowCameraRotationCenter( enabled ); } );
  connect( chkShowLightSourceOrigins, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setShowLightSourceOrigins( enabled ); } );
  connect( chkStopUpdates, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setStopUpdates( enabled ); } );
  connect( chkDebugOverlay, &QCheckBox::toggled, this, [=]( const bool enabled ) { mMap->setIsDebugOverlayEnabled( enabled ); } );

  // set up the shadow map block
  whileBlocking( mDebugShadowMapGroupBox )->setChecked( mMap->debugShadowMapEnabled() );
  whileBlocking( mDebugShadowMapCornerComboBox )->setCurrentIndex( mMap->debugShadowMapCorner() );
  whileBlocking( mDebugShadowMapSizeSpinBox )->setValue( mMap->debugShadowMapSize() );
  // Do not display the shadow debug map if the shadow effect is not enabled.
  connect( mDebugShadowMapGroupBox, &QGroupBox::toggled, this, [=]( const bool enabled ) {
    mMap->setDebugShadowMapSettings( enabled && mMap->shadowSettings().renderShadows(), static_cast<Qt::Corner>( mDebugShadowMapCornerComboBox->currentIndex() ), mDebugShadowMapSizeSpinBox->value() );
  } );
  connect( mDebugShadowMapCornerComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( const int index ) {
    mMap->setDebugShadowMapSettings( mDebugShadowMapGroupBox->isChecked() && mMap->shadowSettings().renderShadows(), static_cast<Qt::Corner>( index ), mDebugShadowMapSizeSpinBox->value() );
  } );
  connect( mDebugShadowMapSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    mMap->setDebugShadowMapSettings( mDebugShadowMapGroupBox->isChecked() && mMap->shadowSettings().renderShadows(), static_cast<Qt::Corner>( mDebugShadowMapCornerComboBox->currentIndex() ), value );
  } );

  // set up the depth map block
  whileBlocking( mDebugDepthMapGroupBox )->setChecked( mMap->debugDepthMapEnabled() );
  whileBlocking( mDebugDepthMapCornerComboBox )->setCurrentIndex( mMap->debugDepthMapCorner() );
  whileBlocking( mDebugDepthMapSizeSpinBox )->setValue( mMap->debugDepthMapSize() );
  connect( mDebugDepthMapGroupBox, &QGroupBox::toggled, this, [=]( const bool enabled ) {
    mMap->setDebugDepthMapSettings( enabled, static_cast<Qt::Corner>( mDebugDepthMapCornerComboBox->currentIndex() ), mDebugDepthMapSizeSpinBox->value() );
  } );
  connect( mDebugDepthMapCornerComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( const int index ) {
    mMap->setDebugDepthMapSettings( mDebugDepthMapGroupBox->isChecked(), static_cast<Qt::Corner>( index ), mDebugDepthMapSizeSpinBox->value() );
  } );
  connect( mDebugDepthMapSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    mMap->setDebugDepthMapSettings( mDebugDepthMapGroupBox->isChecked(), static_cast<Qt::Corner>( mDebugDepthMapCornerComboBox->currentIndex() ), value );
  } );

  // connect the camera info spin boxes with changing functions
  connect( mNearPlane, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    m3DMapCanvas->cameraController()->camera()->setNearPlane( static_cast<float>( value ) );
  } );
  connect( mFarPlane, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    m3DMapCanvas->cameraController()->camera()->setFarPlane( static_cast<float>( value ) );
  } );
  connect( mCameraX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    QVector3D newPosition = m3DMapCanvas->cameraController()->camera()->position();
    newPosition.setX( static_cast<float>( value ) );
    m3DMapCanvas->cameraController()->camera()->setPosition( newPosition );
  } );
  connect( mCameraY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    QVector3D newPosition = m3DMapCanvas->cameraController()->camera()->position();
    newPosition.setY( static_cast<float>( value ) );
    m3DMapCanvas->cameraController()->camera()->setPosition( newPosition );
  } );
  connect( mCameraZ, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    QVector3D newPosition = m3DMapCanvas->cameraController()->camera()->position();
    newPosition.setZ( static_cast<float>( value ) );
    m3DMapCanvas->cameraController()->camera()->setPosition( newPosition );
  } );
  connect( mLookingX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    QgsVector3D newLookingAt = m3DMapCanvas->cameraController()->lookingAtPoint();
    newLookingAt.setX( value );
    m3DMapCanvas->cameraController()->setLookingAtPoint(
      newLookingAt,
      m3DMapCanvas->cameraController()->distance(),
      m3DMapCanvas->cameraController()->pitch(),
      m3DMapCanvas->cameraController()->yaw()
    );
  } );
  connect( mLookingY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    QgsVector3D newLookingAt = m3DMapCanvas->cameraController()->lookingAtPoint();
    newLookingAt.setY( value );
    m3DMapCanvas->cameraController()->setLookingAtPoint(
      newLookingAt,
      m3DMapCanvas->cameraController()->distance(),
      m3DMapCanvas->cameraController()->pitch(),
      m3DMapCanvas->cameraController()->yaw()
    );
  } );
  connect( mLookingZ, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( const double value ) {
    QgsVector3D newLookingAt = m3DMapCanvas->cameraController()->lookingAtPoint();
    newLookingAt.setZ( value );
    m3DMapCanvas->cameraController()->setLookingAtPoint(
      newLookingAt,
      m3DMapCanvas->cameraController()->distance(),
      m3DMapCanvas->cameraController()->pitch(),
      m3DMapCanvas->cameraController()->yaw()
    );
  } );
}

void Qgs3DDebugWidget::updateFromCamera() const
{
  whileBlocking( mNearPlane )->setValue( m3DMapCanvas->cameraController()->camera()->nearPlane() );
  whileBlocking( mFarPlane )->setValue( m3DMapCanvas->cameraController()->camera()->farPlane() );
  whileBlocking( mCameraX )->setValue( m3DMapCanvas->cameraController()->camera()->position().x() );
  whileBlocking( mCameraY )->setValue( m3DMapCanvas->cameraController()->camera()->position().y() );
  whileBlocking( mCameraZ )->setValue( m3DMapCanvas->cameraController()->camera()->position().z() );
  whileBlocking( mLookingX )->setValue( m3DMapCanvas->cameraController()->lookingAtPoint().x() );
  whileBlocking( mLookingY )->setValue( m3DMapCanvas->cameraController()->lookingAtPoint().y() );
  whileBlocking( mLookingZ )->setValue( m3DMapCanvas->cameraController()->lookingAtPoint().z() );
}
