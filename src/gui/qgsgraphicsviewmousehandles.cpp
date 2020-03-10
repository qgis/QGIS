/***************************************************************************
                             qgsgraphicsviewmousehandles.cpp
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

#include "qgsgraphicsviewmousehandles.h"
#include "qgis.h"
#include <QGraphicsView>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QWidget>
#include <limits>

///@cond PRIVATE

QgsGraphicsViewMouseHandles::QgsGraphicsViewMouseHandles( QGraphicsView *view )
  : QObject( nullptr )
  , QGraphicsRectItem( nullptr )
  , mView( view )
{
  //accept hover events, required for changing cursor to resize cursors
  setAcceptHoverEvents( true );
}

void QgsGraphicsViewMouseHandles::paintInternal( QPainter *painter, bool showHandles, bool showBoundingBoxes, const QStyleOptionGraphicsItem *, QWidget * )
{
  if ( !showHandles )
  {
    return;
  }

  if ( showBoundingBoxes )
  {
    //draw resize handles around bounds of entire selection
    double rectHandlerSize = rectHandlerBorderTolerance();
    drawHandles( painter, rectHandlerSize );
  }

  if ( mIsResizing || mIsDragging || showBoundingBoxes )
  {
    //draw dotted boxes around selected items
    drawSelectedItemBounds( painter );
  }
}

void QgsGraphicsViewMouseHandles::drawHandles( QPainter *painter, double rectHandlerSize )
{
  //blue, zero width cosmetic pen for outline
  QPen handlePen = QPen( QColor( 55, 140, 195, 255 ) );
  handlePen.setWidth( 0 );
  painter->setPen( handlePen );

  //draw box around entire selection bounds
  painter->setBrush( Qt::NoBrush );
  painter->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );

  //draw resize handles, using a filled white box
  painter->setBrush( QColor( 255, 255, 255, 255 ) );
  //top left
  painter->drawRect( QRectF( 0, 0, rectHandlerSize, rectHandlerSize ) );
  //mid top
  painter->drawRect( QRectF( ( rect().width() - rectHandlerSize ) / 2, 0, rectHandlerSize, rectHandlerSize ) );
  //top right
  painter->drawRect( QRectF( rect().width() - rectHandlerSize, 0, rectHandlerSize, rectHandlerSize ) );
  //mid left
  painter->drawRect( QRectF( 0, ( rect().height() - rectHandlerSize ) / 2, rectHandlerSize, rectHandlerSize ) );
  //mid right
  painter->drawRect( QRectF( rect().width() - rectHandlerSize, ( rect().height() - rectHandlerSize ) / 2, rectHandlerSize, rectHandlerSize ) );
  //bottom left
  painter->drawRect( QRectF( 0, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
  //mid bottom
  painter->drawRect( QRectF( ( rect().width() - rectHandlerSize ) / 2, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
  //bottom right
  painter->drawRect( QRectF( rect().width() - rectHandlerSize, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
}

double QgsGraphicsViewMouseHandles::rectHandlerBorderTolerance()
{
  if ( !mView )
    return 0;

  //calculate size for resize handles
  //get view scale factor
  double viewScaleFactor = mView->transform().m11();

  //size of handle boxes depends on zoom level in layout view
  double rectHandlerSize = 10.0 / viewScaleFactor;

  //make sure the boxes don't get too large
  if ( rectHandlerSize > ( rect().width() / 3 ) )
  {
    rectHandlerSize = rect().width() / 3;
  }
  if ( rectHandlerSize > ( rect().height() / 3 ) )
  {
    rectHandlerSize = rect().height() / 3;
  }
  return rectHandlerSize;
}

Qt::CursorShape QgsGraphicsViewMouseHandles::cursorForPosition( QPointF itemCoordPos )
{
  QgsGraphicsViewMouseHandles::MouseAction mouseAction = mouseActionForPosition( itemCoordPos );
  switch ( mouseAction )
  {
    case NoAction:
      return Qt::ForbiddenCursor;
    case MoveItem:
      return Qt::SizeAllCursor;
    case ResizeUp:
    case ResizeDown:
      //account for rotation
      if ( ( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeVerCursor;
      }
      else if ( ( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeBDiagCursor;
      }
      else if ( ( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
      {
        return Qt::SizeHorCursor;
      }
      else
      {
        return Qt::SizeFDiagCursor;
      }
    case ResizeLeft:
    case ResizeRight:
      //account for rotation
      if ( ( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeHorCursor;
      }
      else if ( ( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeFDiagCursor;
      }
      else if ( ( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
      {
        return Qt::SizeVerCursor;
      }
      else
      {
        return Qt::SizeBDiagCursor;
      }

    case ResizeLeftUp:
    case ResizeRightDown:
      //account for rotation
      if ( ( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeFDiagCursor;
      }
      else if ( ( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeVerCursor;
      }
      else if ( ( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
      {
        return Qt::SizeBDiagCursor;
      }
      else
      {
        return Qt::SizeHorCursor;
      }
    case ResizeRightUp:
    case ResizeLeftDown:
      //account for rotation
      if ( ( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeBDiagCursor;
      }
      else if ( ( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeHorCursor;
      }
      else if ( ( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
      {
        return Qt::SizeFDiagCursor;
      }
      else
      {
        return Qt::SizeVerCursor;
      }
    case SelectItem:
      return Qt::ArrowCursor;
  }

  return Qt::ArrowCursor;
}

QgsGraphicsViewMouseHandles::MouseAction QgsGraphicsViewMouseHandles::mouseActionForPosition( QPointF itemCoordPos )
{
  bool nearLeftBorder = false;
  bool nearRightBorder = false;
  bool nearLowerBorder = false;
  bool nearUpperBorder = false;

  bool withinWidth = false;
  bool withinHeight = false;
  if ( itemCoordPos.x() >= 0 && itemCoordPos.x() <= rect().width() )
  {
    withinWidth = true;
  }
  if ( itemCoordPos.y() >= 0 && itemCoordPos.y() <= rect().height() )
  {
    withinHeight = true;
  }

  double borderTolerance = rectHandlerBorderTolerance();

  if ( itemCoordPos.x() >= 0 && itemCoordPos.x() < borderTolerance )
  {
    nearLeftBorder = true;
  }
  if ( itemCoordPos.y() >= 0 && itemCoordPos.y() < borderTolerance )
  {
    nearUpperBorder = true;
  }
  if ( itemCoordPos.x() <= rect().width() && itemCoordPos.x() > ( rect().width() - borderTolerance ) )
  {
    nearRightBorder = true;
  }
  if ( itemCoordPos.y() <= rect().height() && itemCoordPos.y() > ( rect().height() - borderTolerance ) )
  {
    nearLowerBorder = true;
  }

  if ( nearLeftBorder && nearUpperBorder )
  {
    return QgsGraphicsViewMouseHandles::ResizeLeftUp;
  }
  else if ( nearLeftBorder && nearLowerBorder )
  {
    return QgsGraphicsViewMouseHandles::ResizeLeftDown;
  }
  else if ( nearRightBorder && nearUpperBorder )
  {
    return QgsGraphicsViewMouseHandles::ResizeRightUp;
  }
  else if ( nearRightBorder && nearLowerBorder )
  {
    return QgsGraphicsViewMouseHandles::ResizeRightDown;
  }
  else if ( nearLeftBorder && withinHeight )
  {
    return QgsGraphicsViewMouseHandles::ResizeLeft;
  }
  else if ( nearRightBorder && withinHeight )
  {
    return QgsGraphicsViewMouseHandles::ResizeRight;
  }
  else if ( nearUpperBorder && withinWidth )
  {
    return QgsGraphicsViewMouseHandles::ResizeUp;
  }
  else if ( nearLowerBorder && withinWidth )
  {
    return QgsGraphicsViewMouseHandles::ResizeDown;
  }

  //find out if cursor position is over a selected item
  QPointF scenePoint = mapToScene( itemCoordPos );
  const QList<QGraphicsItem *> itemsAtCursorPos = sceneItemsAtPoint( scenePoint );
  if ( itemsAtCursorPos.isEmpty() )
  {
    //no items at cursor position
    return QgsGraphicsViewMouseHandles::SelectItem;
  }
  for ( QGraphicsItem *graphicsItem : itemsAtCursorPos )
  {
    if ( graphicsItem && graphicsItem->isSelected() )
    {
      //cursor is over a selected layout item
      return QgsGraphicsViewMouseHandles::MoveItem;
    }
  }

  //default
  return QgsGraphicsViewMouseHandles::SelectItem;
}

QgsGraphicsViewMouseHandles::MouseAction QgsGraphicsViewMouseHandles::mouseActionForScenePos( QPointF sceneCoordPos )
{
  // convert sceneCoordPos to item coordinates
  QPointF itemPos = mapFromScene( sceneCoordPos );
  return mouseActionForPosition( itemPos );
}

bool QgsGraphicsViewMouseHandles::shouldBlockEvent( QInputEvent * ) const
{
  return mIsDragging || mIsResizing;
}

void QgsGraphicsViewMouseHandles::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
  setViewportCursor( cursorForPosition( event->pos() ) );
}

void QgsGraphicsViewMouseHandles::hoverLeaveEvent( QGraphicsSceneHoverEvent *event )
{
  Q_UNUSED( event )
  setViewportCursor( Qt::ArrowCursor );
}


void QgsGraphicsViewMouseHandles::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
  Q_UNUSED( event )
}

QSizeF QgsGraphicsViewMouseHandles::calcCursorEdgeOffset( QPointF cursorPos )
{
  //find offset between cursor position and actual edge of item
  QPointF sceneMousePos = mapFromScene( cursorPos );

  switch ( mCurrentMouseMoveAction )
  {
    //vertical resize
    case QgsGraphicsViewMouseHandles::ResizeUp:
      return QSizeF( 0, sceneMousePos.y() );

    case QgsGraphicsViewMouseHandles::ResizeDown:
      return QSizeF( 0, sceneMousePos.y() - rect().height() );

    //horizontal resize
    case QgsGraphicsViewMouseHandles::ResizeLeft:
      return QSizeF( sceneMousePos.x(), 0 );

    case QgsGraphicsViewMouseHandles::ResizeRight:
      return QSizeF( sceneMousePos.x() - rect().width(), 0 );

    //diagonal resize
    case QgsGraphicsViewMouseHandles::ResizeLeftUp:
      return QSizeF( sceneMousePos.x(), sceneMousePos.y() );

    case QgsGraphicsViewMouseHandles::ResizeRightDown:
      return QSizeF( sceneMousePos.x() - rect().width(), sceneMousePos.y() - rect().height() );

    case QgsGraphicsViewMouseHandles::ResizeRightUp:
      return QSizeF( sceneMousePos.x() - rect().width(), sceneMousePos.y() );

    case QgsGraphicsViewMouseHandles::ResizeLeftDown:
      return QSizeF( sceneMousePos.x(), sceneMousePos.y() - rect().height() );

    case MoveItem:
    case SelectItem:
    case NoAction:
      return QSizeF();
  }

  return QSizeF();
}

///@endcond PRIVATE
