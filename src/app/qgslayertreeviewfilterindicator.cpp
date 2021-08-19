/***************************************************************************
  qgslayertreeviewfilterindicator.cpp
  --------------------------------------
  Date                 : January 2018
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

#include "qgslayertreeviewfilterindicator.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgsquerybuilder.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"
#include "qgsstringutils.h"

QgsLayerTreeViewFilterIndicatorProvider::QgsLayerTreeViewFilterIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewFilterIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgisApp::instance()->layerSubsetString( QgsLayerTree::toLayer( node )->layer() );

}

QString QgsLayerTreeViewFilterIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorFilter.svg" );
}

QString QgsLayerTreeViewFilterIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  QString filter;
  if ( vlayer )
    filter = vlayer->subsetString();

  // PG raster
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( rlayer && rlayer->dataProvider() && rlayer->dataProvider()->supportsSubsetString() )
    filter = rlayer->subsetString();

  if ( filter.isEmpty() )
    return QString();

  if ( filter.size() > 1024 )
  {
    filter = QgsStringUtils::truncateMiddleOfString( filter, 1024 );
  }

  return QStringLiteral( "<b>%1:</b><br>%2" ).arg( tr( "Filter" ), filter.toHtmlEscaped() );
}

void QgsLayerTreeViewFilterIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::connectSignals( layer );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
    connect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );

  // PG raster
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( rlayer && rlayer->dataProvider() && rlayer->dataProvider()->supportsSubsetString() )
    connect( rlayer, &QgsRasterLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );

}

void QgsLayerTreeViewFilterIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::disconnectSignals( layer );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
    disconnect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );

  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( rlayer && rlayer->dataProvider() && rlayer->dataProvider()->supportsSubsetString() )
    disconnect( rlayer, &QgsRasterLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
}

bool QgsLayerTreeViewFilterIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
    return ! vlayer->subsetString().isEmpty();

  // PG raster
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( rlayer && rlayer->dataProvider() && rlayer->dataProvider()->supportsSubsetString() )
    return ! rlayer->subsetString().isEmpty();

  return false;
}

