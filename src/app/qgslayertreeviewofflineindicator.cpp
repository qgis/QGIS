/***************************************************************************
  qgslayertreeviewofflineindicator.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by David Signer
  Email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewofflineindicator.h"
#include "moc_qgslayertreeviewofflineindicator.cpp"
#include "qgslayertreeview.h"
#include "qgisapp.h"

QgsLayerTreeViewOfflineIndicatorProvider::QgsLayerTreeViewOfflineIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewOfflineIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  if ( !layer )
    return;

  connect( layer, &QgsMapLayer::customPropertyChanged, this, &QgsLayerTreeViewOfflineIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewOfflineIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  if ( !layer )
    return;

  disconnect( layer, &QgsMapLayer::customPropertyChanged, this, &QgsLayerTreeViewOfflineIndicatorProvider::onLayerChanged );
}

bool QgsLayerTreeViewOfflineIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  return layer->customProperty( QStringLiteral( "isOfflineEditable" ), false ).toBool();
}

QString QgsLayerTreeViewOfflineIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorOffline.svg" );
}

QString QgsLayerTreeViewOfflineIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return tr( "<b>Offline layer</b>" );
}
