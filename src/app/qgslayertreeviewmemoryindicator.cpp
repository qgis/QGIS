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

#include "qgisapp.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgsvectorlayer.h"

#include "moc_qgslayertreeviewmemoryindicator.cpp"

QgsLayerTreeViewMemoryIndicatorProvider::QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewMemoryIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
  if ( !vlayer )
    return;

  QgisApp::instance()->makeMemoryLayerPermanent( vlayer );
}


bool QgsLayerTreeViewMemoryIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return false;

  return layer->isTemporary();
}

QString QgsLayerTreeViewMemoryIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return u"/mIndicatorMemory.svg"_s;
}

QString QgsLayerTreeViewMemoryIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  if ( layer->providerType() == "memory"_L1 )
    return tr( "<b>Temporary scratch layer only!</b><br>Contents will be discarded after closing this project" );

  return tr( "<b>Temporary layer only!</b><br>Contents will be discarded after closing QGIS." );
}
