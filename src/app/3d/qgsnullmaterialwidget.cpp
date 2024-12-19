/***************************************************************************
  qgsnullmaterialwidget.cpp
  --------------------------------------
  Date                 : November 2020
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

#include "qgsnullmaterialwidget.h"
#include "moc_qgsnullmaterialwidget.cpp"

#include "qgsnullmaterialsettings.h"
#include "qgis.h"

QgsNullMaterialWidget::QgsNullMaterialWidget( QWidget *parent )
  : QgsMaterialSettingsWidget( parent )
{
  setupUi( this );
}

QgsMaterialSettingsWidget *QgsNullMaterialWidget::create()
{
  return new QgsNullMaterialWidget();
}

void QgsNullMaterialWidget::setSettings( const QgsAbstractMaterialSettings *, QgsVectorLayer * )
{
}

QgsAbstractMaterialSettings *QgsNullMaterialWidget::settings()
{
  return new QgsNullMaterialSettings();
}
