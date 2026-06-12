/***************************************************************************
  qgsclothmaterialwidget.cpp
  --------------------------------------
  Date                 : May 2026
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

#include "qgsclothmaterialwidget.h"

#include "qgis.h"
#include "qgsclothmaterialsettings.h"
#include "qgsdoublespinbox.h"

#include <QString>

#include "moc_qgsclothmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsClothMaterialWidget::QgsClothMaterialWidget( QWidget *parent, bool )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"cloth"_s );

  QgsClothMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  // clear has no meaning here
  mRoughnessWidget->spinBox()->setShowClearButton( false );

  //  mButtonSheenColor->setShowNull( true, tr( "No Emission" ) );

  connect( mButtonBaseColor, &QgsColorButton::colorChanged, this, &QgsClothMaterialWidget::changed );
  connect( mRoughnessWidget, &QgsPercentageWidget::valueChanged, this, [this] {
    updateWidgetState();
    emit changed();
  } );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsClothMaterialWidget::changed );
  connect( mButtonSheenColor, &QgsColorButton::colorChanged, this, &QgsClothMaterialWidget::changed );

  connect( mBaseColorDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsClothMaterialWidget::changed );
  connect( mSheenColorDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsClothMaterialWidget::changed );

  connect( this, &QgsClothMaterialWidget::changed, this, &QgsClothMaterialWidget::updatePreview );
}

QgsMaterialSettingsWidget *QgsClothMaterialWidget::create()
{
  return new QgsClothMaterialWidget();
}

void QgsClothMaterialWidget::setTechnique( Qgis::MaterialRenderingTechnique technique )
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
      mSheenColorDataDefinedButton->setVisible( false );
      break;
    }

    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    {
      mBaseColorDataDefinedButton->setVisible( true );
      mSheenColorDataDefinedButton->setVisible( true );
      break;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      // not supported
      break;
  }
}

void QgsClothMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  const QgsClothMaterialSettings *material = dynamic_cast<const QgsClothMaterialSettings *>( settings );
  if ( !material )
    return;
  mButtonBaseColor->setColor( material->baseColor() );
  mRoughnessWidget->setValue( material->roughness() );
  mOpacityWidget->setOpacity( material->opacity() );
  mButtonSheenColor->setColor( material->sheenColor() );

  mPropertyCollection = settings->dataDefinedProperties();

  mBaseColorDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::BaseColor ), mPropertyCollection, settings->propertyDefinitions(), layer, true );
  mSheenColorDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::SheenColor ), mPropertyCollection, settings->propertyDefinitions(), layer, true );

  updateWidgetState();
  updatePreview();
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsClothMaterialWidget::settings()
{
  auto m = std::make_unique<QgsClothMaterialSettings>();
  m->setBaseColor( mButtonBaseColor->color() );
  m->setRoughness( mRoughnessWidget->value() );
  m->setOpacity( mOpacityWidget->opacity() );
  m->setSheenColor( mButtonSheenColor->color() );

  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::BaseColor, mBaseColorDataDefinedButton->toProperty() );
  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::SheenColor, mSheenColorDataDefinedButton->toProperty() );

  m->setDataDefinedProperties( mPropertyCollection );
  return m;
}

void QgsClothMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsClothMaterialWidget::updateWidgetState()
{}

void QgsClothMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
