/***************************************************************************
  qgsphongmaterialwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongmaterialwidget.h"

#include "qgsphongmaterialsettings.h"
#include "qgis.h"

QgsPhongMaterialWidget::QgsPhongMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );

  activateTexturingUI( true );

  QgsPhongMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );
  textureScaleSpinBox->setClearValue( 0 );
  textureRotationSpinBox->setClearValue( 0 );

  connect( btnDiffuse, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnAmbient, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongMaterialWidget::changed );
  connect( useDiffuseCheckBox, &QCheckBox::stateChanged, this, &QgsPhongMaterialWidget::changed );
  connect( textureFile, &QgsImageSourceLineEdit::sourceChanged, this, &QgsPhongMaterialWidget::changed );
  connect( textureScaleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongMaterialWidget::changed );
  connect( textureRotationSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongMaterialWidget::changed );
}

QgsMaterialSettingsWidget *QgsPhongMaterialWidget::create()
{
  return new QgsPhongMaterialWidget();
}

void QgsPhongMaterialWidget::setDiffuseVisible( bool visible )
{
  lblDiffuse->setVisible( visible );
  btnDiffuse->setVisible( visible );
}

bool QgsPhongMaterialWidget::isDiffuseVisible() const
{
  return btnDiffuse->isVisible();
}

void QgsPhongMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsPhongMaterialSettings *phongMaterial = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  if ( !phongMaterial )
    return;
  btnDiffuse->setColor( phongMaterial->diffuse() );
  btnAmbient->setColor( phongMaterial->ambient() );
  btnSpecular->setColor( phongMaterial->specular() );
  spinShininess->setValue( phongMaterial->shininess() );
  useDiffuseCheckBox->setCheckState( phongMaterial->diffuseTextureEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
  textureFile->setSource( phongMaterial->texturePath() );
  textureScaleSpinBox->setValue( phongMaterial->textureScale() );
  textureRotationSpinBox->setValue( phongMaterial->textureRotation() );
}

QgsAbstractMaterialSettings *QgsPhongMaterialWidget::settings()
{
  std::unique_ptr< QgsPhongMaterialSettings > m = qgis::make_unique< QgsPhongMaterialSettings >();
  m->setDiffuse( btnDiffuse->color() );
  m->setAmbient( btnAmbient->color() );
  m->setSpecular( btnSpecular->color() );
  m->setShininess( spinShininess->value() );
  m->setDiffuseTextureEnabled( useDiffuseCheckBox->checkState() == Qt::CheckState::Checked );
  m->setTexturePath( textureFile->source() );
  m->setTextureScale( textureScaleSpinBox->value() );
  m->setTextureRotation( textureRotationSpinBox->value() );
  return m.release();
}

void QgsPhongMaterialWidget::activateTexturingUI( bool activated )
{
  lblTextureScale->setVisible( activated );
  lblTextureRotation->setVisible( activated );
  textureScaleSpinBox->setVisible( activated );
  textureRotationSpinBox->setVisible( activated );
  useDiffuseCheckBox->setVisible( activated );
  textureFile->setVisible( activated );
}
