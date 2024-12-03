/***************************************************************************
  qgslayertreeviewnotesindicator.h
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewnotesindicator.h"
#include "moc_qgslayertreeviewnotesindicator.cpp"
#include "qgslayertreeview.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgsvectorlayer.h"
#include "qgslayernotesmanager.h"
#include "qgslayernotesutils.h"
#include "qgisapp.h"

QgsLayerTreeViewNotesIndicatorProvider::QgsLayerTreeViewNotesIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewNotesIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  if ( QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer() )
  {
    QgsLayerNotesManager::editLayerNotes( layer, QgisApp::instance() );
  }
}

void QgsLayerTreeViewNotesIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::connectSignals( layer );
  connect( layer, &QgsMapLayer::customPropertyChanged, this, &QgsLayerTreeViewNotesIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewNotesIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::disconnectSignals( layer );
  disconnect( layer, &QgsMapLayer::customPropertyChanged, this, &QgsLayerTreeViewNotesIndicatorProvider::onLayerChanged );
}

bool QgsLayerTreeViewNotesIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return false;

  return QgsLayerNotesUtils::layerHasNotes( layer );
}

QString QgsLayerTreeViewNotesIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorNotes.svg" );
}

QString QgsLayerTreeViewNotesIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  return QgsLayerNotesUtils::layerNotes( layer );
}
