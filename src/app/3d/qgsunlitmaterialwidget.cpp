/***************************************************************************
  qgsunlitmaterialwidget.cpp
  --------------------------------------
  Date                 : June 2026
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

#include "qgsunlitmaterialwidget.h"

#include "qgis.h"
#include "qgsunlitmaterialsettings.h"

#include <QString>

#include "moc_qgsunlitmaterialwidget.cpp"

using namespace Qt::StringLiterals;

QgsUnlitMaterialWidget::QgsUnlitMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mPreviewWidget->hide();
  mPreviewWidget->setMaterialType( u"unlit"_s );

  QgsUnlitMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  connect( btnColor, &QgsColorButton::colorChanged, this, &QgsUnlitMaterialWidget::changed );
  connect( mColorDataDefinedButton, &QgsPropertyOverrideButton::changed, this, &QgsUnlitMaterialWidget::changed );

  connect( this, &QgsUnlitMaterialWidget::changed, this, &QgsUnlitMaterialWidget::updatePreview );
}

QgsMaterialSettingsWidget *QgsUnlitMaterialWidget::create()
{
  return new QgsUnlitMaterialWidget();
}

void QgsUnlitMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer )
{
  const QgsUnlitMaterialSettings *unlitMaterial = dynamic_cast<const QgsUnlitMaterialSettings *>( settings );
  if ( !unlitMaterial )
    return;

  btnColor->setColor( unlitMaterial->color() );

  mPropertyCollection = settings->dataDefinedProperties();
  mColorDataDefinedButton->init( static_cast<int>( QgsAbstractMaterialSettings::Property::BaseColor ), mPropertyCollection, settings->propertyDefinitions(), layer, true );

  updatePreview();
}

std::unique_ptr<QgsAbstractMaterialSettings> QgsUnlitMaterialWidget::settings()
{
  auto m = std::make_unique<QgsUnlitMaterialSettings>();
  m->setColor( btnColor->color() );

  mPropertyCollection.setProperty( QgsAbstractMaterialSettings::Property::BaseColor, mColorDataDefinedButton->toProperty() );
  m->setDataDefinedProperties( mPropertyCollection );

  return m;
}

void QgsUnlitMaterialWidget::setPreviewVisible( bool visible )
{
  mPreviewWidget->setVisible( visible );
  updatePreview();
}

void QgsUnlitMaterialWidget::updatePreview()
{
  if ( mPreviewWidget->isHidden() )
    return;
  const std::unique_ptr<QgsAbstractMaterialSettings> newSettings( settings() );
  mPreviewWidget->updatePreview( newSettings.get() );
}
