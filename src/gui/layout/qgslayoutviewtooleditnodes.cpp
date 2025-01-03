/***************************************************************************
                             qgslayoutviewtooleditnodes.cpp
                             ---------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutviewtooleditnodes.h"
#include "moc_qgslayoutviewtooleditnodes.cpp"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemnodeitem.h"
#include "qgslayoutundostack.h"

QgsLayoutViewToolEditNodes::QgsLayoutViewToolEditNodes( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::CrossCursor );
  setFlags( QgsLayoutViewTool::FlagSnaps );
}

void QgsLayoutViewToolEditNodes::deleteSelectedNode()
{
  if ( mNodesItem && mNodesItemIndex != -1 )
  {
    layout()->undoStack()->beginCommand( mNodesItem, tr( "Remove Item Node" ) );
    if ( mNodesItem->removeNode( mNodesItemIndex ) )
    {
      layout()->undoStack()->endCommand();
      if ( mNodesItem->nodesSize() > 0 )
      {
        mNodesItemIndex = mNodesItem->selectedNode();
        // setSelectedNode( mNodesItem, mNodesItemIndex );
      }
      else
      {
        mNodesItemIndex = -1;
        mNodesItem = nullptr;
      }
      if ( mNodesItem )
        mNodesItem->update();
    }
    else
    {
      layout()->undoStack()->cancelCommand();
    }
  }
}

void QgsLayoutViewToolEditNodes::activate()
{
  displayNodes( true );
  QgsLayoutViewTool::activate();
}

void QgsLayoutViewToolEditNodes::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  const QList<QGraphicsItem *> itemsAtCursorPos = view()->items( event->pos().x(), event->pos().y(), mMoveContentSearchRadius, mMoveContentSearchRadius );
  if ( itemsAtCursorPos.isEmpty() )
    return;

  mNodesItemIndex = -1;
  mNodesItem = nullptr;
  isMoving = false;

  for ( QGraphicsItem *graphicsItem : itemsAtCursorPos )
  {
    QgsLayoutNodesItem *item = dynamic_cast<QgsLayoutNodesItem *>( graphicsItem );

    if ( item && !item->isLocked() )
    {
      int index = item->nodeAtPosition( event->layoutPoint() );
      if ( index != -1 )
      {
        mNodesItemIndex = index;
        mNodesItem = item;
        mMoveContentStartPos = event->layoutPoint();
      }
    }

    if ( mNodesItem && mNodesItemIndex != -1 )
    {
      layout()->undoStack()->beginCommand( mNodesItem, tr( "Move Item Node" ) );
      setSelectedNode( mNodesItem, mNodesItemIndex );
      isMoving = true;
      break;
    }
  }
}

void QgsLayoutViewToolEditNodes::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !isMoving )
  {
    event->ignore();
    return;
  }

  if ( mNodesItem && mNodesItemIndex != -1 && event->layoutPoint() != mMoveContentStartPos )
  {
    mNodesItem->moveNode( mNodesItemIndex, event->snappedPoint() );
  }
}

void QgsLayoutViewToolEditNodes::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton || !isMoving )
  {
    event->ignore();
    return;
  }

  isMoving = false;
  if ( mNodesItemIndex != -1 )
  {
    if ( event->layoutPoint() != mMoveContentStartPos )
    {
      layout()->undoStack()->endCommand();
    }
    else
    {
      layout()->undoStack()->cancelCommand();
    }
  }
}

void QgsLayoutViewToolEditNodes::layoutDoubleClickEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  // erase status previously set by the mousePressEvent method
  if ( mNodesItemIndex != -1 )
  {
    mNodesItem = nullptr;
    mNodesItemIndex = -1;
    deselectNodes();
  }

  // search items in layout
  const QList<QGraphicsItem *> itemsAtCursorPos = view()->items( event->pos().x(), event->pos().y(), mMoveContentSearchRadius, mMoveContentSearchRadius );

  if ( itemsAtCursorPos.isEmpty() )
    return;

  bool rc = false;
  for ( QGraphicsItem *graphicsItem : itemsAtCursorPos )
  {
    QgsLayoutNodesItem *item = dynamic_cast<QgsLayoutNodesItem *>( graphicsItem );

    if ( item && !item->isLocked() )
    {
      layout()->undoStack()->beginCommand( item, tr( "Add Item Node" ) );
      rc = item->addNode( event->layoutPoint() );

      if ( rc )
      {
        layout()->undoStack()->endCommand();
        mNodesItem = item;
        mNodesItemIndex = mNodesItem->nodeAtPosition( event->layoutPoint() );
      }
      else
        layout()->undoStack()->cancelCommand();
    }

    if ( rc )
      break;
  }

  if ( rc )
  {
    setSelectedNode( mNodesItem, mNodesItemIndex );
    mNodesItem->update();
  }
}

void QgsLayoutViewToolEditNodes::keyPressEvent( QKeyEvent *event )
{
  if ( mNodesItem && mNodesItemIndex != -1 && ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Right || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ) )
  {
    QPointF currentPos;

    if ( mNodesItem->nodePosition( mNodesItemIndex, currentPos ) )
    {
      QPointF delta = view()->deltaForKeyEvent( event );

      currentPos.setX( currentPos.x() + delta.x() );
      currentPos.setY( currentPos.y() + delta.y() );

      layout()->undoStack()->beginCommand( mNodesItem, tr( "Move Item Node" ), QgsLayoutItem::UndoNodeMove );
      mNodesItem->moveNode( mNodesItemIndex, currentPos );
      layout()->undoStack()->endCommand();
      layout()->update();
    }
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolEditNodes::deactivate()
{
  displayNodes( false );
  deselectNodes();
  QgsLayoutViewTool::deactivate();
}

QList<QgsLayoutItem *> QgsLayoutViewToolEditNodes::ignoredSnapItems() const
{
  QList<QgsLayoutItem *> items;
  if ( mNodesItem )
    items << mNodesItem;
  return items;
}

void QgsLayoutViewToolEditNodes::displayNodes( bool display )
{
  QList<QgsLayoutNodesItem *> nodesShapes;
  layout()->layoutItems( nodesShapes );

  for ( QgsLayoutNodesItem *item : std::as_const( nodesShapes ) )
  {
    item->setDisplayNodes( display );
    item->update();
  }
}

void QgsLayoutViewToolEditNodes::deselectNodes()
{
  QList<QgsLayoutNodesItem *> nodesShapes;
  layout()->layoutItems( nodesShapes );

  for ( QgsLayoutNodesItem *item : std::as_const( nodesShapes ) )
  {
    item->deselectNode();
    item->update();
  }
}

void QgsLayoutViewToolEditNodes::setSelectedNode( QgsLayoutNodesItem *shape, int index )
{
  QList<QgsLayoutNodesItem *> nodesShapes;
  layout()->layoutItems( nodesShapes );

  for ( QgsLayoutNodesItem *item : std::as_const( nodesShapes ) )
  {
    if ( item == shape )
    {
      item->setSelectedNode( index );
      layout()->setSelectedItem( item );
      item->update();
    }
    else
    {
      item->deselectNode();
      item->update();
    }
  }
}
