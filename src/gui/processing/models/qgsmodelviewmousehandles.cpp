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
#include "moc_qgsmodelviewmousehandles.cpp"
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
  setHandleSize( 5 );
}

QgsModelGraphicsScene *QgsModelViewMouseHandles::modelScene() const
{
  return qobject_cast<QgsModelGraphicsScene *>( mView->scene() );
}

void QgsModelViewMouseHandles::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  paintInternal( painter, !( modelScene()->flags() & QgsModelGraphicsScene::FlagHideControls ), true, false, option, widget );
}

void QgsModelViewMouseHandles::selectionChanged()
{
  //listen out for selected items' size and rotation changed signals
  const QList<QGraphicsItem *> itemList = mView->items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsModelComponentGraphicItem *item = dynamic_cast<QgsModelComponentGraphicItem *>( graphicsItem );
    if ( !item )
      continue;

    if ( item->isSelected() )
    {
      connect( item, &QgsModelComponentGraphicItem::sizePositionChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
    }
    else
    {
      disconnect( item, &QgsModelComponentGraphicItem::sizePositionChanged, this, &QgsModelViewMouseHandles::selectedItemSizeChanged );
    }
  }

  resetStatusBar();
  updateHandles();
}

void QgsModelViewMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  //workaround qt bug #3732 by setting cursor for QGraphicsView viewport,
  //rather then setting it directly here

  if ( qobject_cast<QgsModelViewTool *>( mView->tool() ) )
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

QRectF QgsModelViewMouseHandles::storedItemRect( QGraphicsItem *item ) const
{
  if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
    return componentItem->itemRect( true );
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
    componentItem->finalizePreviewedItemRectChange( rect );
  }
}

QRectF QgsModelViewMouseHandles::previewSetItemRect( QGraphicsItem *item, QRectF rect )
{
  if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
  {
    return componentItem->previewItemRectChange( rect );
  }
  return rect;
}

void QgsModelViewMouseHandles::startMacroCommand( const QString &text )
{
  mView->startMacroCommand( text );
}

void QgsModelViewMouseHandles::endMacroCommand()
{
  mView->endMacroCommand();
}

QPointF QgsModelViewMouseHandles::snapPoint( QPointF originalPoint, QgsGraphicsViewMouseHandles::SnapGuideMode mode, bool snapHorizontal, bool snapVertical )
{
  bool snapped = false;

  QPointF snappedPoint;
  switch ( mode )
  {
    case Item:
    case Point:
      snappedPoint = mView->snapper()->snapPoint( originalPoint, mView->transform().m11(), snapped, snapHorizontal, snapVertical );
      break;
  }

  return snapped ? snappedPoint : originalPoint;
}


///@endcond PRIVATE
