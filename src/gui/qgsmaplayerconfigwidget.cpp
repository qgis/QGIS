/***************************************************************************
    qgsmaplayerconfigwidget.cpp
    ---------------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaplayerconfigwidget.h"
#include "moc_qgsmaplayerconfigwidget.cpp"
#include "qgspanelwidget.h"
#include "qgslayertreegroup.h"

//
// QgsMapLayerConfigWidgetContext
//

void QgsMapLayerConfigWidgetContext::setLayerTreeGroup( QgsLayerTreeGroup *group )
{
  mLayerTreeGroup = group;
}

QgsLayerTreeGroup *QgsMapLayerConfigWidgetContext::layerTreeGroup() const
{
  return mLayerTreeGroup;
}

//
//  QgsMapLayerConfigWidget
//

QgsMapLayerConfigWidget::QgsMapLayerConfigWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsPanelWidget( parent )
  , mLayer( layer )
  , mMapCanvas( canvas )
{
}

void QgsMapLayerConfigWidget::setMapLayerConfigWidgetContext( const QgsMapLayerConfigWidgetContext &context )
{
  mMapLayerConfigWidgetContext = context;
}

void QgsMapLayerConfigWidget::focusDefaultWidget()
{
}
