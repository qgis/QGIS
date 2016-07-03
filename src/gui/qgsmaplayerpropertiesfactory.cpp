/***************************************************************************
    qgslayeroptionsfactory.cpp
     --------------------------------------
    Date                 : 9.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerpropertiesfactory.h"

QgsMapLayerPanelFactory::QgsMapLayerPanelFactory()
{
}

QgsMapLayerPanelFactory::~QgsMapLayerPanelFactory()
{
}

bool QgsMapLayerPanelFactory::supportsLayer( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return true;
}

QgsMapLayerPropertiesPage *QgsMapLayerPanelFactory::createPropertiesPage( QgsVectorLayer *layer, QWidget *parent )
{
  return nullptr;
}

QgsMapLayerPanel *QgsMapLayerPanelFactory::createPanel( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
{
  return nullptr;
}
