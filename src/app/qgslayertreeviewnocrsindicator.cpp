/***************************************************************************
  qgslayertreeviewnocrsindicator.h
  --------------------------------------
  Date                 : October 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewnocrsindicator.h"
#include "qgslayertreeview.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsannotationlayer.h"

QgsLayerTreeViewNoCrsIndicatorProvider::QgsLayerTreeViewNoCrsIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewNoCrsIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
  if ( !layer )
    return;

  QgsProjectionSelectionDialog selector( QgisApp::instance() );
  selector.showNoCrsForLayerMessage();
  if ( selector.exec() )
  {
    const QgsCoordinateReferenceSystem crs = selector.crs();
    layer->setCrs( selector.crs() );
    layer->triggerRepaint();
    updateLayerIndicator( layer );
  }
}

bool QgsLayerTreeViewNoCrsIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  return layer && layer->isValid() && layer->isSpatial() && !layer->crs().isValid() && !qobject_cast< QgsAnnotationLayer * >( layer );
}

QString QgsLayerTreeViewNoCrsIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorNoCRS.svg" );
}

QString QgsLayerTreeViewNoCrsIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return tr( "<b>Layer has no coordinate reference system set!</b><br>This layer is not georeferenced and has no geographic location available." );
}

void QgsLayerTreeViewNoCrsIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::connectSignals( layer );
  connect( layer, &QgsMapLayer::crsChanged, this, &QgsLayerTreeViewNoCrsIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewNoCrsIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::disconnectSignals( layer );
  disconnect( layer, &QgsMapLayer::crsChanged, this, &QgsLayerTreeViewNoCrsIndicatorProvider::onLayerChanged );
}
