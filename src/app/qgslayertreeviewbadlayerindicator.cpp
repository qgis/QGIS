/***************************************************************************
  qgslayertreeviewbadlayerindicatorprovider.cpp - QgsLayerTreeViewBadLayerIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewbadlayerindicator.h"
#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowsertreeview.h"
#include "qgsbrowserproxymodel.h"

#include <functional>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

QgsLayerTreeViewBadLayerIndicatorProvider::QgsLayerTreeViewBadLayerIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( QgsLayerTree::toLayer( node )->layer() );

  if ( !layer )
    return;

  emit requestChangeDataSource( layer );
}

QString QgsLayerTreeViewBadLayerIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return QStringLiteral( "/mIndicatorBadLayer.svg" );
}

QString QgsLayerTreeViewBadLayerIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return tr( "<b>Unavailable layer!</b><br>Layer data source could not be found. Click to set a new data source" );
}

bool QgsLayerTreeViewBadLayerIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  return ! layer->isValid();
}
