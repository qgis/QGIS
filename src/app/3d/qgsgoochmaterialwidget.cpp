/***************************************************************************
  qgsgoochmaterialwidget.cpp
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

#include "qgsgoochmaterialwidget.h"

#include "qgis.h"
#include "qgsgoochmaterialsettings.h"

#include <QString>

#include "moc_qgsgoochmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsGoochMaterialWidget::QgsGoochMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"gooch"_s );

  QgsGoochMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  spinShininess->setClearValue( 100 );
  spinAlpha->setClearValue( 0.25 );
  spinBeta->setClearValue( 0.5 );

  btnWarm->setDefaultColor( QColor( 107, 0, 107 ) );
  btnCool->setDefaultColor( QColor( 255, 130, 0 ) );

  connect( btnDiffuse, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( btnWarm, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( btnCool, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsGoochMaterialWidget::changed );
  connect( spinAlpha, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsGoochMaterialWidget::changed );
  connect( spinBeta, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsGoochMaterialWidget::changed );
  connect( mDiffuseDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsGoochMaterialWidget::changed );
  connect( mWarmDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsGoochMaterialWidget::changed );
  connect( mCoolDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsGoochMaterialWidget::changed );
  connect( mSpecularDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsGoochMaterialWidget::changed );

  connect( this, &QgsGoochMaterialWidget::changed, this, &QgsGoochMaterialWidget::updatePreview );
}

QgsMaterialSettingsWidget *QgsGoochMaterialWidget::create()
{
  return new QgsGoochMaterialWidget();
}

void QgsGoochMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  const QgsGoochMaterialSettings *goochMaterial = dynamic_cast<const QgsGoochMaterialSettings *>( settings );
  if ( !goochMaterial )
    return;
  btnDiffuse->setColor( goochMaterial->diffuse() );
  btnWarm->setColor( goochMaterial->warm() );
  btnCool->setColor( goochMaterial->cool() );
  btnSpecular->setColor( goochMaterial->specular() );
  spinShininess->setValue( goochMaterial->shininess() );
  spinAlpha->setValue( goochMaterial->alpha() );
  spinBeta->setValue( goochMaterial->beta() );

  mPropertyCollection = settings->dataDefinedProperties();

  mDiffuseDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::Diffuse ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mWarmDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::Warm ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mCoolDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::Cool ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mSpecularDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::Specular ), mPropertyCollection, settings->propertyDefinitions(), layer, true );

  updatePreview();
}

void QgsGoochMaterialWidget::setTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
      mDiffuseDataDefinedButton->setVisible( false );
      mWarmDataDefinedButton->setVisible( false );
      mCoolDataDefinedButton->setVisible( false );
      mSpecularDataDefinedButton->setVisible( false );
      break;
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
      mDiffuseDataDefinedButton->setVisible( true );
      mWarmDataDefinedButton->setVisible( true );
      mCoolDataDefinedButton->setVisible( true );
      mSpecularDataDefinedButton->setVisible( true );
      break;
    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      // not supported
      break;
  }
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsGoochMaterialWidget::settings()
{
  auto m = std::make_unique<QgsGoochMaterialSettings>();
  m->setDiffuse( btnDiffuse->color() );
  m->setWarm( btnWarm->color() );
  m->setCool( btnCool->color() );
  m->setSpecular( btnSpecular->color() );
  m->setShininess( spinShininess->value() );
  m->setAlpha( spinAlpha->value() );
  m->setBeta( spinBeta->value() );

  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::Diffuse, mDiffuseDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::Warm, mWarmDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::Cool, mCoolDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::Specular, mSpecularDataDefinedButton->toProperty() );
  m->setDataDefinedProperties( mPropertyCollection );

  return m;
}

void QgsGoochMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsGoochMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
