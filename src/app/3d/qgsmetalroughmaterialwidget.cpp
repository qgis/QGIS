/***************************************************************************
  qgsmetalroughmaterialwidget.cpp
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughmaterialwidget.h"

#include "qgis.h"
#include "qgsdoublespinbox.h"
#include "qgsmetalroughmaterialsettings.h"

#include <QString>

#include "moc_qgsmetalroughmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsMetalRoughMaterialWidget::QgsMetalRoughMaterialWidget( QWidget *parent, bool )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"metalrough"_s );

  QgsMetalRoughMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  // clear has no meaning here
  mMetalnessWidget->spinBox()->setShowClearButton( false );
  mRoughnessWidget->spinBox()->setShowClearButton( false );
  mReflectanceWidget->spinBox()->setClearValue( 50 );
  mAnisotropyWidget->spinBox()->setClearValue( 0 );
  mClearCoatFactorWidget->spinBox()->setClearValue( 0 );
  mClearCoatRoughnessWidget->spinBox()->setClearValue( 0 );

  mEmissionStrengthSpinBox->setClearValue( 100 );
  mEmissionStrengthSpinBox->setEnabled( false );
  mButtonEmissionColor->setShowNull( true, tr( "No Emission" ) );

  connect( mButtonBaseColor, &QgsColorButton::colorChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mMetalnessWidget, &QgsPercentageWidget::valueChanged, this, [this] {
    updateWidgetState();
    emit changed();
  } );
  connect( mRoughnessWidget, &QgsPercentageWidget::valueChanged, this, [this] {
    updateWidgetState();
    emit changed();
  } );
  connect( mReflectanceWidget, &QgsPercentageWidget::valueChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mAnisotropyWidget, &QgsPercentageWidget::valueChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mAnisotropyRotationWidget, &QSlider::valueChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mEmissionStrengthSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsMetalRoughMaterialWidget::changed );
  connect( mButtonEmissionColor, &QgsColorButton::colorChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mButtonEmissionColor, &QgsColorButton::colorChanged, this, [this] {
    mEmissionStrengthSpinBox->setEnabled( mButtonEmissionColor->color().isValid() || mEmissionColorDataDefinedButton->isActive() );
  } );
  connect( mEmissionColorDataDefinedButton, &QgsPropertyOverrideButton::activated, this, [this] {
    mEmissionStrengthSpinBox->setEnabled( mButtonEmissionColor->color().isValid() || mEmissionColorDataDefinedButton->isActive() );
  } );

  connect( mClearCoatFactorWidget, &QgsPercentageWidget::valueChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mClearCoatFactorWidget, &QgsPercentageWidget::valueChanged, this, [this] { mClearCoatRoughnessWidget->setEnabled( mClearCoatFactorWidget->value() > 0 ); } );
  connect( mClearCoatRoughnessWidget, &QgsPercentageWidget::valueChanged, this, &QgsMetalRoughMaterialWidget::changed );

  connect( mBaseColorDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mEmissionColorDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsMetalRoughMaterialWidget::changed );

  connect( this, &QgsMetalRoughMaterialWidget::changed, this, &QgsMetalRoughMaterialWidget::updatePreview );
}

QgsMaterialSettingsWidget *QgsMetalRoughMaterialWidget::create()
{
  return new QgsMetalRoughMaterialWidget();
}

void QgsMetalRoughMaterialWidget::setTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    {
      mBaseColorDataDefinedButton->setVisible( false );
      mEmissionColorDataDefinedButton->setVisible( false );
      break;
    }

    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    {
      mBaseColorDataDefinedButton->setVisible( true );
      mEmissionColorDataDefinedButton->setVisible( true );
      break;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      // not supported
      break;
  }
}

void QgsMetalRoughMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  const QgsMetalRoughMaterialSettings *material = dynamic_cast<const QgsMetalRoughMaterialSettings *>( settings );
  if ( !material )
    return;
  mButtonBaseColor->setColor( material->baseColor() );
  mMetalnessWidget->setValue( material->metalness() );
  mRoughnessWidget->setValue( material->roughness() );
  mReflectanceWidget->setValue( material->reflectance() );
  mAnisotropyWidget->setValue( material->anisotropy() );
  mAnisotropyRotationWidget->setValue( material->anisotropyRotation() );
  mOpacityWidget->setOpacity( material->opacity() );
  mButtonEmissionColor->setColor( material->emissionColor() );
  mEmissionStrengthSpinBox->setValue( material->emissionFactor() * 100 );
  mEmissionStrengthSpinBox->setEnabled( mButtonEmissionColor->color().isValid() );

  mClearCoatFactorWidget->setValue( material->clearCoatFactor() );
  mClearCoatRoughnessWidget->setValue( material->clearCoatRoughness() );
  mClearCoatRoughnessWidget->setEnabled( mClearCoatFactorWidget->value() > 0 );

  mPropertyCollection = settings->dataDefinedProperties();

  mBaseColorDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::BaseColor ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mEmissionColorDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::EmissionColor ), mPropertyCollection, settings->propertyDefinitions(), layer, true );

  updateWidgetState();
  updatePreview();
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsMetalRoughMaterialWidget::settings()
{
  auto m = std::make_unique<QgsMetalRoughMaterialSettings>();
  m->setBaseColor( mButtonBaseColor->color() );
  m->setMetalness( mMetalnessWidget->value() );
  m->setRoughness( mRoughnessWidget->value() );
  m->setReflectance( mReflectanceWidget->value() );
  m->setAnisotropy( mAnisotropyWidget->value() );
  m->setAnisotropyRotation( mAnisotropyRotationWidget->value() );
  m->setOpacity( mOpacityWidget->opacity() );
  m->setEmissionColor( mButtonEmissionColor->color() );
  m->setEmissionFactor( mEmissionStrengthSpinBox->value() / 100.0 );

  m->setClearCoatFactor( mClearCoatFactorWidget->value() );
  m->setClearCoatRoughness( mClearCoatRoughnessWidget->value() );

  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::BaseColor, mBaseColorDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::EmissionColor, mEmissionColorDataDefinedButton->toProperty() );

  m->setDataDefinedProperties( mPropertyCollection );
  return m;
}

void QgsMetalRoughMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsMetalRoughMaterialWidget::updateWidgetState()
{}

void QgsMetalRoughMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
