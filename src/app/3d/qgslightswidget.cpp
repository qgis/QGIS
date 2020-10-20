/***************************************************************************
  qgslightswidget.cpp
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslightswidget.h"

#include "qgs3dmapsettings.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QMessageBox>

QgsLightsWidget::QgsLightsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  btnAddLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  dialAzimuth->setMaximum( 359 );

  connect( btnAddLight, &QToolButton::clicked, this, &QgsLightsWidget::onAddLight );
  connect( btnRemoveLight, &QToolButton::clicked, this, &QgsLightsWidget::onRemoveLight );

  connect( cboLights, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsLightsWidget::onCurrentLightChanged );
  connect( spinPositionX, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinPositionY, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinPositionZ, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinIntensity, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( btnColor, &QgsColorButton::colorChanged, this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinA0, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinA1, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );
  connect( spinA2, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentLightParameters );

  btnAddDirectionalLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveDirectionalLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  connect( btnAddDirectionalLight, &QToolButton::clicked, this, &QgsLightsWidget::onAddDirectionalLight );
  connect( btnRemoveDirectionalLight, &QToolButton::clicked, this, &QgsLightsWidget::onRemoveDirectionalLight );

  connect( cboDirectionalLights, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsLightsWidget::onCurrentDirectionalLightChanged );
  connect( spinDirectionalIntensity, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
  connect( btnDirectionalColor, &QgsColorButton::colorChanged, this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );

  connect( dialAzimuth, &QSlider::valueChanged, [this]( int value ) {spinBoxAzimuth->setValue( ( value + 180 ) % 360 );} );
  connect( sliderAltitude, &QSlider::valueChanged, spinBoxAltitude, &QgsDoubleSpinBox::setValue );
  connect( spinBoxAzimuth, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::onDirectionChange );
  connect( spinBoxAltitude, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::onDirectionChange );

  tabWidget->setCurrentIndex( QgsSettings().value( QStringLiteral( "UI/last3DLightsTab" ), 1 ).toInt() );
}

QgsLightsWidget::~QgsLightsWidget()
{
  QgsSettings().setValue( QStringLiteral( "UI/last3DLightsTab" ), tabWidget->currentIndex() );
}

void QgsLightsWidget::setPointLights( const QList<QgsPointLightSettings> &pointLights )
{
  mPointLights = pointLights;
  updateLightsList();
  cboLights->setCurrentIndex( 0 );
  onCurrentLightChanged( 0 );
}

void QgsLightsWidget::setDirectionalLights( const QList<QgsDirectionalLightSettings> &directionalLights )
{
  mDirectionalLights = directionalLights;
  updateDirectionalLightsList();
  cboDirectionalLights->setCurrentIndex( 0 );
  onCurrentDirectionalLightChanged( 0 );
}

QList<QgsPointLightSettings> QgsLightsWidget::pointLights()
{
  return mPointLights;
}

QList<QgsDirectionalLightSettings> QgsLightsWidget::directionalLights()
{
  return mDirectionalLights;
}

void QgsLightsWidget::onCurrentLightChanged( int index )
{
  if ( index < 0 || index >= cboLights->count() )
    return;

  QgsPointLightSettings light = mPointLights.at( index );
  whileBlocking( spinPositionX )->setValue( light.position().x() );
  whileBlocking( spinPositionY )->setValue( light.position().y() );
  whileBlocking( spinPositionZ )->setValue( light.position().z() );
  whileBlocking( btnColor )->setColor( light.color() );
  whileBlocking( spinIntensity )->setValue( light.intensity() );
  whileBlocking( spinA0 )->setValue( light.constantAttenuation() );
  whileBlocking( spinA1 )->setValue( light.linearAttenuation() );
  whileBlocking( spinA2 )->setValue( light.quadraticAttenuation() );
}

void QgsLightsWidget::onCurrentDirectionalLightChanged( int index )
{
  if ( index < 0 || index >= cboDirectionalLights->count() )
    return;

  QgsDirectionalLightSettings light = mDirectionalLights.at( index );
  mDirectionX = light.direction().x();
  mDirectionY = light.direction().y();
  mDirectionZ = light.direction().z();
  whileBlocking( btnDirectionalColor )->setColor( light.color() );
  whileBlocking( spinDirectionalIntensity )->setValue( light.intensity() );
  setAzimuthAltitude();
}


void QgsLightsWidget::updateCurrentLightParameters()
{
  int index = cboLights->currentIndex();
  if ( index < 0 || index >= cboLights->count() )
    return;

  QgsPointLightSettings light;
  light.setPosition( QgsVector3D( spinPositionX->value(), spinPositionY->value(), spinPositionZ->value() ) );
  light.setColor( btnColor->color() );
  light.setIntensity( spinIntensity->value() );
  light.setConstantAttenuation( spinA0->value() );
  light.setLinearAttenuation( spinA1->value() );
  light.setQuadraticAttenuation( spinA2->value() );
  mPointLights[index] = light;
}

void QgsLightsWidget::updateCurrentDirectionalLightParameters()
{
  labelX->setText( QString::number( mDirectionX, 'f', 2 ) );
  labelY->setText( QString::number( mDirectionY, 'f', 2 ) );
  labelZ->setText( QString::number( mDirectionZ, 'f', 2 ) );

  int index = cboDirectionalLights->currentIndex();
  if ( index < 0 || index >= cboDirectionalLights->count() )
    return;

  QgsDirectionalLightSettings light;
  light.setDirection( QgsVector3D( mDirectionX, mDirectionY, mDirectionZ ) );
  light.setColor( btnDirectionalColor->color() );
  light.setIntensity( spinDirectionalIntensity->value() );
  mDirectionalLights[index] = light;
}

void QgsLightsWidget::onAddLight()
{
  if ( mPointLights.count() >= 8 )
  {
    QMessageBox::warning( this, tr( "Add Light" ), tr( "It is not possible to add more than 8 lights to the scene." ) );
    return;
  }

  mPointLights << QgsPointLightSettings();
  updateLightsList();
  cboLights->setCurrentIndex( cboLights->count() - 1 );
  // To set default parameters of the light
  onCurrentDirectionalLightChanged( 0 );
}

void QgsLightsWidget::onAddDirectionalLight()
{
  if ( mDirectionalLights.count() > 4 )
  {
    QMessageBox::warning( this, tr( "Add Directional Light" ), tr( "It is not possible to add more than 4 directional lights to the scene." ) );
    return;
  }

  mDirectionalLights << QgsDirectionalLightSettings();
  updateDirectionalLightsList();
  cboDirectionalLights->setCurrentIndex( cboDirectionalLights->count() - 1 );
  // To set default parameters of the light
  onCurrentDirectionalLightChanged( 0 );

  emit directionalLightsCountChanged( cboDirectionalLights->count() );
}

void QgsLightsWidget::onRemoveLight()
{
  int index = cboLights->currentIndex();
  if ( index < 0 || index >= cboLights->count() )
    return;

  mPointLights.removeAt( index );
  updateLightsList();
  if ( index >= cboLights->count() )
    --index;  // in case we removed the last light
  cboLights->setCurrentIndex( index );
  onCurrentLightChanged( index );
}

void QgsLightsWidget::onRemoveDirectionalLight()
{
  int index = cboDirectionalLights->currentIndex();
  if ( index < 0 || index >= cboDirectionalLights->count() )
    return;

  mDirectionalLights.removeAt( index );
  updateDirectionalLightsList();
  if ( index >= cboDirectionalLights->count() )
    --index;  // in case we removed the last light
  cboDirectionalLights->setCurrentIndex( index );
  onCurrentDirectionalLightChanged( index );

  emit directionalLightsCountChanged( cboDirectionalLights->count() );
}

void QgsLightsWidget::setAzimuthAltitude()
{
  double azimuthAngle;
  double altitudeAngle;

  double horizontalVectorMagnitude = sqrt( mDirectionX * mDirectionX + mDirectionZ * mDirectionZ );

  if ( horizontalVectorMagnitude == 0 )
    azimuthAngle = 0;
  else
  {
    azimuthAngle = ( asin( -mDirectionX / horizontalVectorMagnitude ) ) / M_PI * 180;
    if ( mDirectionZ < 0 )
      azimuthAngle = 180 - azimuthAngle;
    azimuthAngle = std::fmod( azimuthAngle + 360.0, 360.0 );
  }

  dialAzimuth->setValue( int( azimuthAngle + 180 ) % 360 );
  spinBoxAzimuth->setValue( azimuthAngle );

  if ( horizontalVectorMagnitude == 0 )
    altitudeAngle = mDirectionY >= 0 ? 90 : -90;
  else
    altitudeAngle = -atan( mDirectionY / horizontalVectorMagnitude ) / M_PI * 180;

  spinBoxAltitude->setValue( altitudeAngle );
  sliderAltitude->setValue( altitudeAngle );

  updateCurrentDirectionalLightParameters();
}

void QgsLightsWidget::onDirectionChange()
{
  double altitudeValue = spinBoxAltitude->value();
  double azimuthValue = spinBoxAzimuth->value();

  double horizontalVectorMagnitude = cos( altitudeValue / 180 * M_PI );
  mDirectionX = -horizontalVectorMagnitude * sin( azimuthValue / 180 * M_PI );
  mDirectionZ = horizontalVectorMagnitude * cos( azimuthValue / 180 * M_PI );
  mDirectionY = -sin( altitudeValue / 180 * M_PI );


  whileBlocking( sliderAltitude )->setValue( altitudeValue );
  updateCurrentDirectionalLightParameters();
}

void QgsLightsWidget::updateLightsList()
{
  cboLights->blockSignals( true );
  cboLights->clear();
  for ( int i = 0; i < mPointLights.count(); ++i )
  {
    cboLights->addItem( tr( "Light %1" ).arg( i + 1 ) );
  }
  cboLights->blockSignals( false );
}

void QgsLightsWidget::updateDirectionalLightsList()
{
  cboDirectionalLights->blockSignals( true );
  cboDirectionalLights->clear();
  for ( int i = 0; i < mDirectionalLights.count(); ++i )
  {
    cboDirectionalLights->addItem( tr( "Directional light %1" ).arg( i + 1 ) );
  }
  cboDirectionalLights->blockSignals( false );
}
