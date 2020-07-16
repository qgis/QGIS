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

#include "qgsgoochmaterialsettings.h"
#include "qgis.h"

QgsGoochMaterialWidget::QgsGoochMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );

  QgsGoochMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  connect( btnDiffuse, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( btnWarm, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( btnCool, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsGoochMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsGoochMaterialWidget::changed );
}

QgsMaterialSettingsWidget *QgsGoochMaterialWidget::create()
{
  return new QgsGoochMaterialWidget();
}

void QgsGoochMaterialWidget::setDiffuseVisible( bool visible )
{
  lblDiffuse->setVisible( visible );
  btnDiffuse->setVisible( visible );
}

bool QgsGoochMaterialWidget::isDiffuseVisible() const
{
  return btnDiffuse->isVisible();
}

void QgsGoochMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsGoochMaterialSettings *goochMaterial = dynamic_cast< const QgsGoochMaterialSettings * >( settings );
  if ( !goochMaterial )
    return;
  btnDiffuse->setColor( goochMaterial->diffuse() );
  btnWarm->setColor( goochMaterial->warm() );
  btnCool->setColor( goochMaterial->cool() );
  btnSpecular->setColor( goochMaterial->specular() );
  spinShininess->setValue( goochMaterial->shininess() );
}

QgsAbstractMaterialSettings *QgsGoochMaterialWidget::settings()
{
  std::unique_ptr< QgsGoochMaterialSettings > m = qgis::make_unique< QgsGoochMaterialSettings >();
  m->setDiffuse( btnDiffuse->color() );
  m->setWarm( btnWarm->color() );
  m->setCool( btnCool->color() );
  m->setSpecular( btnSpecular->color() );
  m->setShininess( spinShininess->value() );
  return m.release();
}
