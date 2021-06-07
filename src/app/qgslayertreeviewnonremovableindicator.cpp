/***************************************************************************
  qgslayertreeviewnonremovableindicator.cpp
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorNonRemovable.svg" );
}

QString QgsLayerTreeViewNonRemovableIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return tr( "Layer required by the project" );
}

bool QgsLayerTreeViewNonRemovableIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  return ! layer->flags().testFlag( QgsMapLayer::LayerFlag::Removable );
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::connectSignals( layer );
  connect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::disconnectSignals( layer );
  disconnect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onLayerChanged );
}

