/***************************************************************************
    qgspointcloudlayerstylewidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerstylewidget.h"

#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudrendererpropertieswidget.h"
#include "qgsstyle.h"

#include "moc_qgspointcloudlayerstylewidget.cpp"

QgsPointCloudRendererWidgetFactory::QgsPointCloudRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( u"propertyicons/symbology.svg"_s ) );
  setTitle( tr( "Symbology" ) );
}

QgsMapLayerConfigWidget *QgsPointCloudRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *, bool, QWidget *parent ) const
{
  return new QgsPointCloudRendererPropertiesWidget( qobject_cast<QgsPointCloudLayer *>( layer ), QgsStyle::defaultStyle(), parent );
}

bool QgsPointCloudRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsPointCloudRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsPointCloudRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::PointCloud;
}

QString QgsPointCloudRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return u"mOptsPage_Rendering"_s;
}
