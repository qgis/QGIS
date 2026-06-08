/***************************************************************************
  qgsphongtexturedmaterialwidget.cpp
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongtexturedmaterialwidget.h"

#include "qgis.h"
#include "qgsphongtexturedmaterialsettings.h"

#include <QString>

#include "moc_qgsphongtexturedmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsPhongTexturedMaterialWidget::QgsPhongTexturedMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"phongtextured"_s );

  spinShininess->setClearValue( 0, tr( "None" ) );

  QgsPhongTexturedMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );
  textureScaleSpinBox->setClearValue( 100 );
  textureRotationSpinBox->setClearValue( 0 );
  textureOffsetXSpin->setClearValue( 0.0 );
  textureOffsetYSpin->setClearValue( 0.0 );

  connect( btnAmbient, &QgsColorButton::colorChanged, this, &QgsPhongTexturedMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsPhongTexturedMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this] {
    updateWidgetState();
    emit changed();
  } );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsPhongTexturedMaterialWidget::changed );
  connect( textureFile, &QgsImageSourceLineEdit::sourceChanged, this, &QgsPhongTexturedMaterialWidget::changed );
  connect( textureScaleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongTexturedMaterialWidget::changed );
  connect( textureRotationSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongTexturedMaterialWidget::changed );
  connect( textureOffsetXSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsPhongTexturedMaterialWidget::changed );
  connect( textureOffsetYSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsPhongTexturedMaterialWidget::changed );

  connect( this, &QgsPhongTexturedMaterialWidget::changed, this, &QgsPhongTexturedMaterialWidget::updatePreview );
}

QgsMaterialSettingsWidget *QgsPhongTexturedMaterialWidget::create()
{
  return new QgsPhongTexturedMaterialWidget();
}

void QgsPhongTexturedMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  const QgsPhongTexturedMaterialSettings *phongMaterial = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( settings );
  if ( !phongMaterial )
    return;
  btnAmbient->setColor( phongMaterial->ambient() );
  btnSpecular->setColor( phongMaterial->specular() );
  spinShininess->setValue( phongMaterial->shininess() );
  mOpacityWidget->setOpacity( phongMaterial->opacity() );
  textureFile->setSource( phongMaterial->diffuseTexturePath() );
  textureScaleSpinBox->setValue( 100.0 / phongMaterial->textureScale() );
  textureRotationSpinBox->setValue( phongMaterial->textureRotation() );
  textureOffsetXSpin->setValue( phongMaterial->textureOffset().x() );
  textureOffsetYSpin->setValue( phongMaterial->textureOffset().y() );

  mPropertyCollection = settings->dataDefinedProperties();

  mTextureRotationDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::TextureRotation ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mTextureScaleDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::TextureScale ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mTextureOffsetDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::TextureOffset ), mPropertyCollection, settings->propertyDefinitions(), layer, true );

  connect( mTextureRotationDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsPhongTexturedMaterialWidget::changed );
  connect( mTextureScaleDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsPhongTexturedMaterialWidget::changed );
  connect( mTextureOffsetDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsPhongTexturedMaterialWidget::changed );

  updateWidgetState();
  updatePreview();
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsPhongTexturedMaterialWidget::settings()
{
  auto m = std::make_unique<QgsPhongTexturedMaterialSettings>();
  m->setAmbient( btnAmbient->color() );
  m->setSpecular( btnSpecular->color() );
  m->setShininess( spinShininess->value() );
  m->setOpacity( mOpacityWidget->opacity() );
  m->setDiffuseTexturePath( textureFile->source() );
  m->setTextureScale( 100.0 / textureScaleSpinBox->value() );
  m->setTextureRotation( textureRotationSpinBox->value() );
  m->setTextureOffset( QPointF( textureOffsetXSpin->value(), textureOffsetYSpin->value() ) );

  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::TextureRotation, mTextureRotationDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::TextureScale, mTextureScaleDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::TextureOffset, mTextureOffsetDataDefinedButton->toProperty() );

  m->setDataDefinedProperties( mPropertyCollection );
  return m;
}

void QgsPhongTexturedMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsPhongTexturedMaterialWidget::updateWidgetState()
{
  if ( spinShininess->value() > 0 )
  {
    btnSpecular->setEnabled( true );
    btnSpecular->setToolTip( QString() );
  }
  else
  {
    btnSpecular->setEnabled( false );
    btnSpecular->setToolTip( tr( "Specular color is disabled because material has no shininess" ) );
  }
}

void QgsPhongTexturedMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
