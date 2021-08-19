/***************************************************************************
  qgssimplelinematerialwidget.cpp
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

#include "qgssimplelinematerialwidget.h"

#include "qgssimplelinematerialsettings.h"
#include "qgis.h"

QgsSimpleLineMaterialWidget::QgsSimpleLineMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );

  QgsSimpleLineMaterialSettings defaultMaterial;
  setSettings( &defaultMaterial, nullptr );

  connect( btnAmbient, &QgsColorButton::colorChanged, this, &QgsSimpleLineMaterialWidget::changed );
}

QgsMaterialSettingsWidget *QgsSimpleLineMaterialWidget::create()
{
  return new QgsSimpleLineMaterialWidget();
}

void QgsSimpleLineMaterialWidget::setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer * )
{
  const QgsSimpleLineMaterialSettings *lineMaterial = dynamic_cast< const QgsSimpleLineMaterialSettings * >( settings );
  if ( !lineMaterial )
    return;

  btnAmbient->setColor( lineMaterial->ambient() );
}

QgsAbstractMaterialSettings *QgsSimpleLineMaterialWidget::settings()
{
  std::unique_ptr< QgsSimpleLineMaterialSettings > m = std::make_unique< QgsSimpleLineMaterialSettings >();
  m->setAmbient( btnAmbient->color() );
  return m.release();
}
