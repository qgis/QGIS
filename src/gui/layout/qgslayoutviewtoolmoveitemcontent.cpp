/***************************************************************************
                             qgslayoutviewtoolmoveitemcontent.cpp
                             ------------------------------------
    Date                 : October 2017
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

#include "qgslayoutviewtoolmoveitemcontent.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemnodeitem.h"
#include "qgssettings.h"
#include "qgslayoutundostack.h"

QgsLayoutViewToolMoveItemContent::QgsLayoutViewToolMoveItemContent( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::ArrowCursor );
}

void QgsLayoutViewToolMoveItemContent::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  const QList<QGraphicsItem *> itemsAtCursorPos = view()->items( event->pos() );
  if ( itemsAtCursorPos.isEmpty() )
    return;

  //find highest non-locked QgsLayoutItem at clicked position
  //(other graphics items may be higher, e.g., selection handles)
  for ( QGraphicsItem *graphicsItem : itemsAtCursorPos )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    if ( item && !item->isLocked() )
    {
      //we've found the highest QgsLayoutItem
      mMoveContentStartPos = event->layoutPoint();
      mMoveContentItem = item;
      mMovingItemContent = true;
      break;
    }
  }
}

void QgsLayoutViewToolMoveItemContent::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mMovingItemContent || !mMoveContentItem )
  {
    event->ignore();
    return;
  }

  //update item preview
  mMoveContentItem->setMoveContentPreviewOffset( event->layoutPoint().x() - mMoveContentStartPos.x(),
      event->layoutPoint().y() - mMoveContentStartPos.y() );
  mMoveContentItem->update();
}

void QgsLayoutViewToolMoveItemContent::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton || !mMovingItemContent || !mMoveContentItem )
  {
    event->ignore();
    return;
  }

  //update item preview
  mMoveContentItem->setMoveContentPreviewOffset( 0, 0 );

  const double moveX = event->layoutPoint().x() - mMoveContentStartPos.x();
  const double moveY = event->layoutPoint().y() - mMoveContentStartPos.y();

  mMoveContentItem->layout()->undoStack()->beginCommand( mMoveContentItem, tr( "Move Item Content" ) );
  mMoveContentItem->moveContent( -moveX, -moveY );
  mMoveContentItem->layout()->undoStack()->endCommand();
  mMoveContentItem = nullptr;
  mMovingItemContent = false;
}

void QgsLayoutViewToolMoveItemContent::wheelEvent( QWheelEvent *event )
{
  event->accept();

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
  QPointF scenePoint = view()->mapToScene( event->pos().x(), event->pos().y() );
#else
  const QPointF scenePoint = view()->mapToScene( event->position().x(), event->position().y() );
#endif

  //select topmost item at position of event
  QgsLayoutItem *item = layout()->layoutItemAt( scenePoint, true );
  if ( !item || !item->isSelected() )
    return;

  const QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2.0 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  const bool zoomIn = event->angleDelta().y() > 0;
  const double scaleFactor = ( zoomIn ? zoomFactor : 1 / zoomFactor );

  const QPointF itemPoint = item->mapFromScene( scenePoint );
  item->layout()->undoStack()->beginCommand( item, tr( "Zoom Item Content" ), QgsLayoutItem::UndoZoomContent );
  item->zoomContent( scaleFactor, itemPoint );
  item->layout()->undoStack()->endCommand();
}
