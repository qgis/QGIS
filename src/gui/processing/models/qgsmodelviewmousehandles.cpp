/***************************************************************************
                             qgsmodelviewmousehandles.cpp
                             ------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewmousehandles.h"
#include "qgis.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelviewtool.h"
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QWidget>
#include <limits>

///@cond PRIVATE


QgsModelViewMouseHandles::QgsModelViewMouseHandles( QgsModelGraphicsView *view )
  : QgsGraphicsViewMouseHandles( view )
  , mView( view )
{
  //listen for selection changes, and update handles accordingly
  connect( modelScene(), &QGraphicsScene::selectionChanged, this, &QgsModelViewMouseHandles::selectionChanged );
}

QgsModelGraphicsScene *QgsModelViewMouseHandles::modelScene() const
{
  return qobject_cast< QgsModelGraphicsScene * >( mView->scene() );
}

void QgsModelViewMouseHandles::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  paintInternal( painter, !( modelScene()->flags() & QgsModelGraphicsScene::FlagHideControls ),
                 false, false, option, widget );
}

void QgsModelViewMouseHandles::selectionChanged()
{
#if 0
  //listen out for selected items' size and rotation changed signals
  const QList<QGraphicsItem *> itemList = layout()->items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    if ( !item )
      continue;

    if ( item->isSelected() )
    {
      connect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
      connect( item, &QgsLayoutItem::rotationChanged, this, &QgsModelViewMouseHandles::selectedItemRotationChanged );
      connect( item, &QgsLayoutItem::frameChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
      connect( item, &QgsLayoutItem::lockChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
    }
    else
    {
      disconnect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
      disconnect( item, &QgsLayoutItem::rotationChanged, this, &QgsModelViewMouseHandles::selectedItemRotationChanged );
      disconnect( item, &QgsLayoutItem::frameChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
      disconnect( item, &QgsLayoutItem::lockChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
    }
  }

#endif
  resetStatusBar();
  updateHandles();
}

void QgsModelViewMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  //workaround qt bug #3732 by setting cursor for QGraphicsView viewport,
  //rather then setting it directly here

  if ( qobject_cast< QgsModelViewTool *>( mView->tool() ) )
  {
    mView->viewport()->setCursor( cursor );
  }
}

QList<QGraphicsItem *> QgsModelViewMouseHandles::sceneItemsAtPoint( QPointF scenePoint )
{
  return modelScene()->items( scenePoint );
}

QList<QGraphicsItem *> QgsModelViewMouseHandles::selectedSceneItems( bool ) const
{
  QList<QGraphicsItem *> res;
  const QList<QgsModelComponentGraphicItem *> componentItems = modelScene()->selectedComponentItems();
  res.reserve( componentItems.size() );
  for ( QgsModelComponentGraphicItem *item : componentItems )
    res << item;
  return res;
}

QRectF QgsModelViewMouseHandles::itemRect( QGraphicsItem *item ) const
{
  if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
    return componentItem->itemRect();
  else
    return QRectF();
}

void QgsModelViewMouseHandles::moveItem( QGraphicsItem *item, double deltaX, double deltaY )
{
  if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
  {
    componentItem->moveComponentBy( deltaX, deltaY );
  }
}

void QgsModelViewMouseHandles::previewItemMove( QGraphicsItem *item, double deltaX, double deltaY )
{
  if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
  {
    componentItem->previewItemMove( deltaX, deltaY );
  }
}

void QgsModelViewMouseHandles::setItemRect( QGraphicsItem *item, QRectF rect )
{
  if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
  {

  }
}


///@endcond PRIVATE
