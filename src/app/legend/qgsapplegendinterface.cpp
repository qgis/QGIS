/***************************************************************************
    qgsapplegendinterface.cpp
     --------------------------------------
    Date                 : 19-Nov-2009
    Copyright            : (C) 2009 by Andres Manz
    Email                : manz dot andres at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplegendinterface.h"

#include "qgslegend.h"
#include "qgslegendlayer.h"
#include "qgsmaplayer.h"


QgsAppLegendInterface::QgsAppLegendInterface( QgsLegend * legend )
    : mLegend( legend )
{
  connect( legend, SIGNAL( itemAdded( QModelIndex ) ), this, SIGNAL( itemAdded( QModelIndex ) ) );
  connect( legend, SIGNAL( itemMoved( QModelIndex, QModelIndex ) ), this, SLOT( updateIndex( QModelIndex, QModelIndex ) ) );
  connect( legend, SIGNAL( itemMoved( QModelIndex, QModelIndex ) ), this, SIGNAL( groupRelationsChanged( ) ) );
  connect( legend, SIGNAL( itemMovedGroup( QgsLegendItem *, int ) ), this, SIGNAL( groupRelationsChanged() ) );
  // connect( legend, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SIGNAL( groupRelationsChanged() ) );
  connect( legend, SIGNAL( itemRemoved() ), this, SIGNAL( itemRemoved() ) );
  connect( legend, SIGNAL( currentLayerChanged( QgsMapLayer * ) ), this, SIGNAL( currentLayerChanged( QgsMapLayer * ) ) );
}

QgsAppLegendInterface::~QgsAppLegendInterface()
{
}

int QgsAppLegendInterface::addGroup( QString name, bool expand, QTreeWidgetItem* parent )
{
  return mLegend->addGroup( name, expand, parent );
}

int QgsAppLegendInterface::addGroup( QString name, bool expand, int parentIndex )
{
  return mLegend->addGroup( name, expand, parentIndex );
}

void QgsAppLegendInterface::removeGroup( int groupIndex )
{
  mLegend->removeGroup( groupIndex );
}

void QgsAppLegendInterface::moveLayer( QgsMapLayer * ml, int groupIndex )
{
  mLegend->moveLayer( ml, groupIndex );
}

void QgsAppLegendInterface::updateIndex( QModelIndex oldIndex, QModelIndex newIndex )
{
  if ( mLegend->isLegendGroup( newIndex ) )
  {
    emit groupIndexChanged( oldIndex.row(), newIndex.row() );
  }
}

void QgsAppLegendInterface::setGroupExpanded( int groupIndex, bool expand )
{
  QTreeWidgetItem * item = getItem( groupIndex );
  if ( !item )
  {
    return;
  }

  item->setExpanded( expand );
}

void QgsAppLegendInterface::setGroupVisible( int groupIndex, bool visible )
{
  if ( !groupExists( groupIndex ) )
  {
    return;
  }

  Qt::CheckState state = visible ? Qt::Checked : Qt::Unchecked;
  getItem( groupIndex )->setCheckState( 0, state );
}

QTreeWidgetItem *QgsAppLegendInterface::getItem( int itemIndex )
{
  int itemCount = 0;
  for ( QTreeWidgetItem* theItem = mLegend->firstItem(); theItem; theItem = mLegend->nextItem( theItem ) )
  {
    QgsLegendItem* legendItem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( legendItem->type() == QgsLegendItem::LEGEND_GROUP )
    {
      if ( itemCount == itemIndex )
      {
        return theItem;
      }
      else
      {
        itemCount = itemCount + 1;
      }
    }
  }

  return NULL;
}

void QgsAppLegendInterface::setLayerVisible( QgsMapLayer * ml, bool visible )
{
  mLegend->setLayerVisible( ml, visible );
}

void QgsAppLegendInterface::setLayerExpanded( QgsMapLayer * ml, bool expand )
{
  QgsLegendLayer * item = mLegend->findLegendLayer( ml );
  if ( item ) item->setExpanded( expand );
}

QStringList QgsAppLegendInterface::groups()
{
  return mLegend->groups();
}

QList< GroupLayerInfo > QgsAppLegendInterface::groupLayerRelationship()
{
  if ( mLegend )
  {
    return mLegend->groupLayerRelationship();
  }
  return QList< GroupLayerInfo >();
}

bool QgsAppLegendInterface::groupExists( int groupIndex )
{
  QTreeWidgetItem * item = getItem( groupIndex );
  QgsLegendItem* legendItem = dynamic_cast<QgsLegendItem *>( item );

  if ( !legendItem )
  {
    return false;
  }

  return legendItem->type() == QgsLegendItem::LEGEND_GROUP;
}

bool QgsAppLegendInterface::isGroupExpanded( int groupIndex )
{
  QTreeWidgetItem * item = getItem( groupIndex );
  if ( !item )
  {
    return false;
  }

  return item->isExpanded();
}

bool QgsAppLegendInterface::isGroupVisible( int groupIndex )
{
  if ( !groupExists( groupIndex ) )
  {
    return false;
  }

  return ( Qt::Checked == getItem( groupIndex )->checkState( 0 ) );
}

bool QgsAppLegendInterface::isLayerExpanded( QgsMapLayer * ml )
{
  return mLegend->layerIsExpanded( ml );
}

bool QgsAppLegendInterface::isLayerVisible( QgsMapLayer * ml )
{
  return ( Qt::Checked == mLegend->layerCheckState( ml ) );
}

QList<QgsMapLayer *> QgsAppLegendInterface::selectedLayers( bool inDrawOrder ) const
{
  return mLegend->selectedLayers( inDrawOrder );
}

QList< QgsMapLayer * > QgsAppLegendInterface::layers() const
{
  return mLegend->layers();
}

void QgsAppLegendInterface::refreshLayerSymbology( QgsMapLayer *ml )
{
  mLegend->refreshLayerSymbology( ml->id() );
}

void QgsAppLegendInterface::addLegendLayerAction( QAction* action,
    QString menu, QString id, QgsMapLayer::LayerType type, bool allLayers )
{
  mLegend->addLegendLayerAction( action, menu, id, type, allLayers );
}

void QgsAppLegendInterface::addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer )
{
  mLegend->addLegendLayerActionForLayer( action, layer );
}

bool QgsAppLegendInterface::removeLegendLayerAction( QAction* action )
{
  return mLegend->removeLegendLayerAction( action );
}

QgsMapLayer* QgsAppLegendInterface::currentLayer()
{
  return mLegend->currentLayer();
}

bool QgsAppLegendInterface::setCurrentLayer( QgsMapLayer *layer )
{
  return mLegend->setCurrentLayer( layer );
}
