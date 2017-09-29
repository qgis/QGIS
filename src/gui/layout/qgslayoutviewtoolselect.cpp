/***************************************************************************
                             qgslayoutviewtoolselect.cpp
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

#include "qgslayoutviewtoolselect.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitempage.h"

QgsLayoutViewToolSelect::QgsLayoutViewToolSelect( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::ArrowCursor );

  mRubberBand.reset( new QgsLayoutViewRectangularRubberBand( view ) );
  mRubberBand->setBrush( QBrush( QColor( 224, 178, 76, 63 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 254, 58, 29, 100 ) ), 0, Qt::DotLine ) );
}

void QgsLayoutViewToolSelect::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsSelecting = true;
  mMousePressStartPos = event->pos();
  mRubberBand->start( event->layoutPoint(), 0 );
}

void QgsLayoutViewToolSelect::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( mIsSelecting )
  {
    mRubberBand->update( event->layoutPoint(), 0 );
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolSelect::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mIsSelecting || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsSelecting = false;
  bool wasClick = !isClickAndDrag( mMousePressStartPos, event->pos() );

  QRectF rect = mRubberBand->finish( event->layoutPoint(), event->modifiers() );

  bool subtractingSelection = false;
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //shift modifer means adding to selection, nothing required here
  }
  else if ( event->modifiers() & Qt::ControlModifier )
  {
    //control modifier means subtract from current selection
    subtractingSelection = true;
  }
  else
  {
    //not adding to or removing from selection, so clear current selection
    layout()->deselectAll();
  }

  //determine item selection mode, default to intersection
  Qt::ItemSelectionMode selectionMode = Qt::IntersectsItemShape;
  if ( event->modifiers() & Qt::AltModifier )
  {
    //alt modifier switches to contains selection mode
    selectionMode = Qt::ContainsItemShape;
  }

  //find all items in rect
  QList<QGraphicsItem *> itemList;
  if ( wasClick )
    itemList = layout()->items( rect.center(), selectionMode );
  else
    itemList = layout()->items( rect, selectionMode );
  for ( QGraphicsItem *item : qgsAsConst( itemList ) )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item );
    QgsLayoutItemPage *paperItem = dynamic_cast<QgsLayoutItemPage *>( item );
    if ( layoutItem && !paperItem )
    {
      if ( !layoutItem->isLocked() )
      {
        if ( subtractingSelection )
        {
          layoutItem->setSelected( false );
        }
        else
        {
          layoutItem->setSelected( true );
        }
        if ( wasClick )
        {
          // found an item, and only a click - nothing more to do
          break;
        }
      }
    }
  }

#if 0 //TODO
  //update item panel
  QList<QgsComposerItem *> selectedItemList = composition()->selectedComposerItems();
  if ( !selectedItemList.isEmpty() )
  {
    emit selectedItemChanged( selectedItemList[0] );
  }
#endif
}

void QgsLayoutViewToolSelect::deactivate()
{
  if ( mIsSelecting )
  {
    mRubberBand->finish();
    mIsSelecting = false;
  }
  QgsLayoutViewTool::deactivate();
}
