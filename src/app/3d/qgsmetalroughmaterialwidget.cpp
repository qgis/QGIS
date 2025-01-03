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
#include "moc_qgsmetalroughmaterialwidget.cpp"

#include "qgsmetalroughmaterialsettings.h"
#include "qgis.h"

QgsMetalRoughMaterialWidget::QgsMetalRoughMaterialWidget( QWidget *parent, bool )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
  mSpinMetalness->setClearValue( 0, tr( "None" ) );
  mSpinRoughness->setClearValue( 0, tr( "None" ) );

  QgsMetalRoughMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  connect( mButtonBaseColor, &QgsColorButton::colorChanged, this, &QgsMetalRoughMaterialWidget::changed );
  connect( mSpinMetalness, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [=] {
    updateWidgetState();
    emit changed();
  } );
  connect( mSpinRoughness, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [=] {
    updateWidgetState();
    emit changed();
  } );
}

QgsMaterialSettingsWidget *QgsMetalRoughMaterialWidget::create()
{
  return new QgsMetalRoughMaterialWidget();
}

void QgsMetalRoughMaterialWidget::setTechnique( QgsMaterialSettingsRenderingTechnique )
{
}

void QgsMetalRoughMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsMetalRoughMaterialSettings *material = dynamic_cast<const QgsMetalRoughMaterialSettings *>( settings );
  if ( !material )
    return;
  mButtonBaseColor->setColor( material->baseColor() );
  mSpinMetalness->setValue( material->metalness() );
  mSpinRoughness->setValue( material->roughness() );

  mPropertyCollection = settings->dataDefinedProperties();

  updateWidgetState();
}

QgsAbstractMaterialSettings *QgsMetalRoughMaterialWidget::settings()
{
  std::unique_ptr<QgsMetalRoughMaterialSettings> m = std::make_unique<QgsMetalRoughMaterialSettings>();
  m->setBaseColor( mButtonBaseColor->color() );
  m->setMetalness( static_cast<float>( mSpinMetalness->value() ) );
  m->setRoughness( static_cast<float>( mSpinRoughness->value() ) );
  m->setDataDefinedProperties( mPropertyCollection );
  return m.release();
}

void QgsMetalRoughMaterialWidget::updateWidgetState()
{
}
