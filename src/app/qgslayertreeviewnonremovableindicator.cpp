/***************************************************************************
  qgslayertreeviewnonremovableindicator.cpp
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewnonremovableindicator.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"


QgsLayerTreeViewNonRemovableIndicatorProvider::QgsLayerTreeViewNonRemovableIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

QString QgsLayerTreeViewNonRemovableIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return QStringLiteral( "/mIndicatorNonRemovable.svg" );
}

QString QgsLayerTreeViewNonRemovableIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return tr( "Layer required by the project" );
}

bool QgsLayerTreeViewNonRemovableIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  return layer->flags() & QgsMapLayer::Removable;
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  connect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  disconnect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onLayerChanged );
}

