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
#include "qgslayoutmousehandles.h"

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
  if ( mMouseHandles->shouldBlockEvent( event ) )
  {
    //swallow clicks while dragging/resizing items
    return;
  }

  if ( mMouseHandles->isVisible() )
  {
    //selection handles are being shown, get mouse action for current cursor position
    QgsLayoutMouseHandles::MouseAction mouseAction = mMouseHandles->mouseActionForScenePos( event->layoutPoint() );

    if ( mouseAction != QgsLayoutMouseHandles::MoveItem
         && mouseAction != QgsLayoutMouseHandles::NoAction
         && mouseAction != QgsLayoutMouseHandles::SelectItem )
    {
      //mouse is over a resize handle, so propagate event onward
      event->ignore();
      return;
    }
  }

  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  QgsLayoutItem *selectedItem = nullptr;
  QgsLayoutItem *previousSelectedItem = nullptr;

  QList<QgsLayoutItem *> selectedItems = layout()->selectedLayoutItems();

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //CTRL modifier, so we are trying to select the next item below the current one
    //first, find currently selected item
    if ( !selectedItems.isEmpty() )
    {
      previousSelectedItem = selectedItems.at( 0 );
    }
  }

  if ( previousSelectedItem )
  {
    //select highest item just below previously selected item at position of event
    selectedItem = layout()->layoutItemAt( event->layoutPoint(), previousSelectedItem, true );

    //if we didn't find a lower item we'll use the top-most as fall-back
    //this duplicates mapinfo/illustrator/etc behavior where ctrl-clicks are "cyclic"
    if ( !selectedItem )
    {
      selectedItem = layout()->layoutItemAt( event->layoutPoint(), true );
    }
  }
  else
  {
    //select topmost item at position of event
    selectedItem = layout()->layoutItemAt( event->layoutPoint(), true );
  }

  if ( !selectedItem )
  {
    //not clicking over an item, so start marquee selection
    mIsSelecting = true;
    mMousePressStartPos = event->pos();
    mRubberBand->start( event->layoutPoint(), 0 );
    return;
  }

  if ( ( event->modifiers() & Qt::ShiftModifier ) && ( selectedItem->isSelected() ) )
  {
    //SHIFT-clicking a selected item deselects it
    selectedItem->setSelected( false );

    //Check if we have any remaining selected items, and if so, update the item panel
    const QList<QgsLayoutItem *> selectedItems = layout()->selectedLayoutItems();
    if ( !selectedItems.isEmpty() )
    {
      emit itemFocused( selectedItems.at( 0 ) );
    }
    else
    {
      emit itemFocused( nullptr );
    }
  }
  else
  {
    if ( ( !selectedItem->isSelected() ) &&       //keep selection if an already selected item pressed
         !( event->modifiers() & Qt::ShiftModifier ) ) //keep selection if shift key pressed
    {
      layout()->setSelectedItem( selectedItem ); // clears existing selection
    }
    else
    {
      selectedItem->setSelected( true );
    }
    event->ignore();
    emit itemFocused( selectedItem );
  }
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
  if ( event->button() != Qt::LeftButton && mMouseHandles->shouldBlockEvent( event ) )
  {
    //swallow clicks while dragging/resizing items
    return;
  }

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
    whileBlocking( layout() )->deselectAll();
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
  for ( QGraphicsItem *item : qgis::as_const( itemList ) )
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

  //update item panel
  const QList<QgsLayoutItem *> selectedItemList = layout()->selectedLayoutItems();
  if ( !selectedItemList.isEmpty() )
  {
    emit itemFocused( selectedItemList.at( 0 ) );
  }
  else
  {
    emit itemFocused( nullptr );
  }
  mMouseHandles->selectionChanged();
}

void QgsLayoutViewToolSelect::wheelEvent( QWheelEvent *event )
{
  if ( mMouseHandles->shouldBlockEvent( event ) )
  {
    //ignore wheel events while dragging/resizing items
    return;
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolSelect::keyPressEvent( QKeyEvent *event )
{
  if ( mMouseHandles->isDragging() || mMouseHandles->isResizing() )
  {
    return;
  }
  else
  {
    event->ignore();
  }
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

///@cond PRIVATE
QgsLayoutMouseHandles *QgsLayoutViewToolSelect::mouseHandles()
{
  return mMouseHandles;
}

void QgsLayoutViewToolSelect::setLayout( QgsLayout *layout )
{
  // existing handles are owned by previous layout

  //add mouse selection handles to layout, and initially hide
  mMouseHandles = new QgsLayoutMouseHandles( layout, view() );
  mMouseHandles->hide();
  mMouseHandles->setZValue( QgsLayout::ZMouseHandles );
  layout->addItem( mMouseHandles );
}
///@endcond
