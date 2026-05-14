/***************************************************************************
  qgsmetalroughtexturedmaterialwidget.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughtexturedmaterialwidget.h"

#include "qgis.h"
#include "qgsmetalroughtexturedmaterialsettings.h"

#include <QString>

#include "moc_qgsmetalroughtexturedmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsMetalRoughTexturedMaterialWidget::QgsMetalRoughTexturedMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"metalroughtextured"_s );

  QgsMetalRoughTexturedMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );
  textureScaleSpinBox->setClearValue( 100 );
  textureRotationSpinBox->setClearValue( 0 );
  mEmissionStrengthSpinBox->setClearValue( 100 );
  mParallaxScaleSpinBox->setClearValue( 100 );

  mParallaxScaleSpinBox->setEnabled( false );
  mEmissionStrengthSpinBox->setEnabled( false );

  connect( mBaseColorTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mMetalnessTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mRoughnessTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mNormalTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mHeightTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mAmbientOcclusionTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mEmissionTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );

  connect( mParallaxScaleSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mEmissionStrengthSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( textureScaleSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( textureRotationSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsMetalRoughTexturedMaterialWidget::changed );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsMetalRoughTexturedMaterialWidget::changed );

  connect( this, &QgsMetalRoughTexturedMaterialWidget::changed, this, &QgsMetalRoughTexturedMaterialWidget::updatePreview );

  connect( mHeightTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, [this] { mParallaxScaleSpinBox->setEnabled( !mHeightTextureWidget->source().isEmpty() ); } );

  connect( mEmissionTextureWidget, &QgsImageSourceLineEdit::sourceChanged, this, [this] { mEmissionStrengthSpinBox->setEnabled( !mEmissionTextureWidget->source().isEmpty() ); } );
}

QgsMaterialSettingsWidget *QgsMetalRoughTexturedMaterialWidget::create()
{
  return new QgsMetalRoughTexturedMaterialWidget();
}

void QgsMetalRoughTexturedMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsMetalRoughTexturedMaterialSettings *metalRoughMaterial = dynamic_cast<const QgsMetalRoughTexturedMaterialSettings *>( settings );
  if ( !metalRoughMaterial )
    return;

  mBaseColorTextureWidget->setSource( metalRoughMaterial->baseColorTexturePath() );
  mMetalnessTextureWidget->setSource( metalRoughMaterial->metalnessTexturePath() );
  mRoughnessTextureWidget->setSource( metalRoughMaterial->roughnessTexturePath() );
  mNormalTextureWidget->setSource( metalRoughMaterial->normalTexturePath() );
  mHeightTextureWidget->setSource( metalRoughMaterial->heightTexturePath() );
  mAmbientOcclusionTextureWidget->setSource( metalRoughMaterial->ambientOcclusionTexturePath() );
  mEmissionTextureWidget->setSource( metalRoughMaterial->emissionTexturePath() );
  mParallaxScaleSpinBox->setValue( metalRoughMaterial->parallaxScale() * 1000 );
  mEmissionStrengthSpinBox->setValue( metalRoughMaterial->emissionFactor() * 100 );
  textureScaleSpinBox->setValue( 100.0 / metalRoughMaterial->textureScale() );
  textureRotationSpinBox->setValue( metalRoughMaterial->textureRotation() );
  mOpacityWidget->setOpacity( metalRoughMaterial->opacity() );

  mParallaxScaleSpinBox->setEnabled( !mHeightTextureWidget->source().isEmpty() );
  mEmissionStrengthSpinBox->setEnabled( !mEmissionTextureWidget->source().isEmpty() );

  mPropertyCollection = settings->dataDefinedProperties();

  updatePreview();
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsMetalRoughTexturedMaterialWidget::settings()
{
  auto m = std::make_unique<QgsMetalRoughTexturedMaterialSettings>();
  m->setBaseColorTexturePath( mBaseColorTextureWidget->source() );
  m->setMetalnessTexturePath( mMetalnessTextureWidget->source() );
  m->setRoughnessTexturePath( mRoughnessTextureWidget->source() );
  m->setNormalTexturePath( mNormalTextureWidget->source() );
  m->setHeightTexturePath( mHeightTextureWidget->source() );
  m->setAmbientOcclusionTexturePath( mAmbientOcclusionTextureWidget->source() );
  m->setEmissionTexturePath( mEmissionTextureWidget->source() );
  m->setParallaxScale( mParallaxScaleSpinBox->value() / 1000.0 );
  m->setEmissionFactor( mEmissionStrengthSpinBox->value() / 100.0 );
  m->setTextureScale( 100.0 / textureScaleSpinBox->value() );
  m->setTextureRotation( textureRotationSpinBox->value() );
  m->setOpacity( mOpacityWidget->opacity() );
  m->setDataDefinedProperties( mPropertyCollection );

  return m;
}

void QgsMetalRoughTexturedMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsMetalRoughTexturedMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
