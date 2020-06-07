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

#include <QMessageBox>


QgsLightsWidget::QgsLightsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  btnAddLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveLight->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

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
  connect( spinDirectionX, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
  connect( spinDirectionY, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
  connect( spinDirectionZ, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
  connect( spinDirectionalIntensity, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
  connect( btnDirectionalColor, &QgsColorButton::colorChanged, this, &QgsLightsWidget::updateCurrentDirectionalLightParameters );
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
  whileBlocking( spinDirectionX )->setValue( light.direction().x() );
  whileBlocking( spinDirectionY )->setValue( light.direction().y() );
  whileBlocking( spinDirectionZ )->setValue( light.direction().z() );
  whileBlocking( btnDirectionalColor )->setColor( light.color() );
  whileBlocking( spinDirectionalIntensity )->setValue( light.intensity() );
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
  int index = cboDirectionalLights->currentIndex();
  if ( index < 0 || index >= cboDirectionalLights->count() )
    return;

  QgsDirectionalLightSettings light;
  light.setDirection( QgsVector3D( spinDirectionX->value(), spinDirectionY->value(), spinDirectionZ->value() ) );
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
