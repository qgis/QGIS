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
#include "qgis.h"

QgsPhongMaterialWidget::QgsPhongMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );

  QgsPhongMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  connect( btnDiffuse, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnAmbient, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongMaterialWidget::changed );
}

QgsMaterialSettingsWidget *QgsPhongMaterialWidget::create()
{
  return new QgsPhongMaterialWidget();
}

void QgsPhongMaterialWidget::setTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    {
      lblDiffuse->setVisible( true );
      btnDiffuse->setVisible( true );
      break;
    }

    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    {
      lblDiffuse->setVisible( false );
      btnDiffuse->setVisible( false );
      break;
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
      // not supported
      break;
  }
}

void QgsPhongMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsPhongMaterialSettings *phongMaterial = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  if ( !phongMaterial )
    return;
  btnDiffuse->setColor( phongMaterial->diffuse() );
  btnAmbient->setColor( phongMaterial->ambient() );
  btnSpecular->setColor( phongMaterial->specular() );
  spinShininess->setValue( phongMaterial->shininess() );
}

QgsAbstractMaterialSettings *QgsPhongMaterialWidget::settings()
{
  std::unique_ptr< QgsPhongMaterialSettings > m = qgis::make_unique< QgsPhongMaterialSettings >();
  m->setDiffuse( btnDiffuse->color() );
  m->setAmbient( btnAmbient->color() );
  m->setSpecular( btnSpecular->color() );
  m->setShininess( spinShininess->value() );
  return m.release();
}
