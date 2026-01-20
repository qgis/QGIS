/***************************************************************************
    qgstiledscenelayerstylewidget.cpp
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenelayerstylewidget.h"

#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsstyle.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenerendererpropertieswidget.h"

#include "moc_qgstiledscenelayerstylewidget.cpp"

QgsTiledSceneRendererWidgetFactory::QgsTiledSceneRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( u"propertyicons/symbology.svg"_s ) );
  setTitle( tr( "Symbology" ) );
}

QgsMapLayerConfigWidget *QgsTiledSceneRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *, bool, QWidget *parent ) const
{
  return new QgsTiledSceneRendererPropertiesWidget( qobject_cast<QgsTiledSceneLayer *>( layer ), QgsStyle::defaultStyle(), parent );
}

bool QgsTiledSceneRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsTiledSceneRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsTiledSceneRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::TiledScene;
}

QString QgsTiledSceneRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return u"mOptsPage_Rendering"_s;
}
