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


QgsPhongMaterialWidget::QgsPhongMaterialWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setMaterial( QgsPhongMaterialSettings() );
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
  this->activateTexturingUI( false );
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

void QgsPhongMaterialWidget::setMaterial( const QgsPhongMaterialSettings &material )
{
  btnDiffuse->setColor( material.diffuse() );
  btnAmbient->setColor( material.ambient() );
  btnSpecular->setColor( material.specular() );
  spinShininess->setValue( material.shininess() );
  useDiffuseCheckBox->setCheckState( material.diffuseTextureEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
  textureFile->setSource( material.texturePath() );
  textureScaleSpinBox->setValue( material.textureScale() );
  textureRotationSpinBox->setValue( material.textureRotation() );
}

QgsPhongMaterialSettings QgsPhongMaterialWidget::material() const
{
  QgsPhongMaterialSettings m;
  m.setDiffuse( btnDiffuse->color() );
  m.setAmbient( btnAmbient->color() );
  m.setSpecular( btnSpecular->color() );
  m.setShininess( spinShininess->value() );
  m.setDiffuseTextureEnabled( useDiffuseCheckBox->checkState() == Qt::CheckState::Checked );
  m.setTexturePath( textureFile->source() );
  m.setTextureScale( textureScaleSpinBox->value() );
  m.setTextureRotation( textureRotationSpinBox->value() );
  return m;
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
