/***************************************************************************
                             qgslayoutviewtooladditem.cpp
                             ----------------------------
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

#include "qgslayoutviewtooladditem.h"
#include "qgsapplication.h"
#include "qgscursors.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslogger.h"
#include "qgslayoutviewrubberband.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>

QgsLayoutViewToolAddItem::QgsLayoutViewToolAddItem( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Add item" ) )
{
  QPixmap crosshairQPixmap = QPixmap( ( const char ** )( cross_hair_cursor ) );
  setCursor( QCursor( crosshairQPixmap, 8, 8 ) );
}

void QgsLayoutViewToolAddItem::setItemType( int itemType )
{
  mItemType = itemType;
}

void QgsLayoutViewToolAddItem::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    return;
  }

  mRubberBand.reset( QgsApplication::layoutItemRegistry()->createItemRubberBand( mItemType, view() ) );
  if ( mRubberBand )
  {
    mRubberBand->start( event->layoutPoint(), event->modifiers() );
  }
}

void QgsLayoutViewToolAddItem::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( mRubberBand )
  {
    mRubberBand->update( event->layoutPoint(), event->modifiers() );
  }
}

void QgsLayoutViewToolAddItem::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    return;
  }

  QRectF rect = QRectF( view()->mapToScene( mMousePressStartPos ),
                        event->layoutPoint() );
  if ( mRubberBand )
  {
    rect = mRubberBand->finish( event->layoutPoint(), event->modifiers() );
  }

  // click? or click-and-drag?
  QPoint mousePressStopPoint = event->pos();
  int diffX = mousePressStopPoint.x() - mMousePressStartPos.x();
  int diffY = mousePressStopPoint.y() - mMousePressStartPos.y();
  bool clickOnly = false;
  if ( qAbs( diffX ) < 2 && qAbs( diffY ) < 2 )
  {
    clickOnly = true;
  }
  Q_UNUSED( clickOnly );

  QgsLayoutItem *item = QgsApplication::layoutItemRegistry()->createItem( mItemType, layout() );
  item->setRect( rect );
  layout()->addItem( item );
}

int QgsLayoutViewToolAddItem::itemType() const
{
  return mItemType;
}
