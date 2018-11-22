/***************************************************************************
  qgslayertreeviewmemoryindicator.h
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewmemoryindicator.h"
#include "qgslayertreeview.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"

QgsLayerTreeViewMemoryIndicatorProvider::QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewMemoryIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
  if ( !vlayer )
    return;

  QgisApp::instance()->makeMemoryLayerPermanent( vlayer );
}


bool QgsLayerTreeViewMemoryIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !( vlayer && vlayer->isValid() ) )
    return false;
  return  vlayer->dataProvider()->name() == QLatin1String( "memory" );
}

QString QgsLayerTreeViewMemoryIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return QStringLiteral( "/mIndicatorMemory.svg" );
}

QString QgsLayerTreeViewMemoryIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return tr( "<b>Temporary scratch layer only!</b><br>Contents will be discarded after closing this project" );
}

