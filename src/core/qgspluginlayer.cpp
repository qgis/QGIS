/***************************************************************************
    qgspluginlayer.cpp
    ---------------------
    begin                : January 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspluginlayer.h"

QgsPluginLayer::QgsPluginLayer( QString layerType, QString layerName )
    : QgsMapLayer( PluginLayer, layerName ), mPluginLayerType( layerType )
{
}

QString QgsPluginLayer::pluginLayerType()
{
  return mPluginLayerType;
}

void QgsPluginLayer::setExtent( const QgsRectangle & extent )
{
  mLayerExtent = extent;
}
