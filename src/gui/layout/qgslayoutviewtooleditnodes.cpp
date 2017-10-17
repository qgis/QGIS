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
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemnodeitem.h"

QgsLayoutViewToolEditNodes::QgsLayoutViewToolEditNodes( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::CrossCursor );
  setFlags( QgsLayoutViewTool::FlagSnaps );
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

  const QList<QGraphicsItem *> itemsAtCursorPos = view()->items( event->pos().x(), event->pos().y(),
      mMoveContentSearchRadius,
      mMoveContentSearchRadius );
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

    if ( mNodesItemIndex != -1 )
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

  if ( mNodesItemIndex != -1 && event->layoutPoint() != mMoveContentStartPos )
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

void QgsLayoutViewToolEditNodes::deactivate()
{
  displayNodes( false );
  deselectNodes();
  QgsLayoutViewTool::deactivate();
}

void QgsLayoutViewToolEditNodes::displayNodes( bool display )
{
  QList<QgsLayoutNodesItem *> nodesShapes;
  layout()->layoutItems( nodesShapes );

  for ( QgsLayoutNodesItem *item : qgis::as_const( nodesShapes ) )
  {
    item->setDisplayNodes( display );
  }

  layout()->update();
}

void QgsLayoutViewToolEditNodes::deselectNodes()
{
  QList<QgsLayoutNodesItem *> nodesShapes;
  layout()->layoutItems( nodesShapes );

  for ( QgsLayoutNodesItem *item : qgis::as_const( nodesShapes ) )
  {
    item->deselectNode();
  }

  layout()->update();
}

void QgsLayoutViewToolEditNodes::setSelectedNode( QgsLayoutNodesItem *shape, int index )
{
  QList<QgsLayoutNodesItem *> nodesShapes;
  layout()->layoutItems( nodesShapes );

  for ( QgsLayoutNodesItem *item : qgis::as_const( nodesShapes ) )
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

