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
}

QgsMaterialSettingsWidget *QgsGoochMaterialWidget::create()
{
  return new QgsGoochMaterialWidget();
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
  spinAlpha->setValue( goochMaterial->alpha() );
  spinBeta->setValue( goochMaterial->beta() );
}

QgsAbstractMaterialSettings *QgsGoochMaterialWidget::settings()
{
  std::unique_ptr< QgsGoochMaterialSettings > m = qgis::make_unique< QgsGoochMaterialSettings >();
  m->setDiffuse( btnDiffuse->color() );
  m->setWarm( btnWarm->color() );
  m->setCool( btnCool->color() );
  m->setSpecular( btnSpecular->color() );
  m->setShininess( spinShininess->value() );
  m->setAlpha( spinAlpha->value() );
  m->setBeta( spinBeta->value() );
  return m.release();
}
