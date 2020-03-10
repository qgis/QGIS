/***************************************************************************
                             qgsmodelviewtoolselect.cpp
                             ---------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewtoolselect.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include "qgsprocessingmodelalgorithm.h"


QgsModelViewToolSelect::QgsModelViewToolSelect( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::ArrowCursor );

  mRubberBand.reset( new QgsModelViewRectangularRubberBand( view ) );
  mRubberBand->setBrush( QBrush( QColor( 224, 178, 76, 63 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 254, 58, 29, 100 ) ), 0, Qt::DotLine ) );
}

QgsModelViewToolSelect::~QgsModelViewToolSelect()
{
#if 0
  if ( mMouseHandles )
  {
    // want to force them to be removed from the scene
    if ( mMouseHandles->scene() )
      mMouseHandles->scene()->removeItem( mMouseHandles );
    mMouseHandles->deleteLater();
  }
#endif
}

void QgsModelViewToolSelect::modelPressEvent( QgsModelViewMouseEvent *event )
{
#if 0
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
#endif

  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

#if 0
  QgsLayoutItem *selectedItem = nullptr;
  QgsLayoutItem *previousSelectedItem = nullptr;

  QList<QgsLayoutItem *> selectedItems = layout()->selectedLayoutItems();

  //select topmost item at position of event
  selectedItem = layout()->layoutItemAt( event->layoutPoint(), true );

  if ( !selectedItem )
  {
    //not clicking over an item, so start marquee selection
    mIsSelecting = true;
    mMousePressStartPos = event->pos();
    mRubberBand->start( event->modelPoint(), nullptr );
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
#endif
  event->ignore();
}

void QgsModelViewToolSelect::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  if ( mIsSelecting )
  {
    mRubberBand->update( event->modelPoint(), nullptr );
  }
  else
  {
    event->ignore();
  }
}

void QgsModelViewToolSelect::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
#if 0
  if ( event->button() != Qt::LeftButton && mMouseHandles->shouldBlockEvent( event ) )
  {
    //swallow clicks while dragging/resizing items
    return;
  }
#endif

  if ( !mIsSelecting || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

#if 0

  mIsSelecting = false;
  bool wasClick = !isClickAndDrag( mMousePressStartPos, event->pos() );

  // important -- we don't pass the event modifiers here, because we use them for a different meaning!
  // (modifying how the selection interacts with the items, rather than modifying the selection shape)
  QRectF rect = mRubberBand->finish( event->modelPoint() );

  bool subtractingSelection = false;
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //shift modifier means adding to selection, nothing required here
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
#endif
}

void QgsModelViewToolSelect::wheelEvent( QWheelEvent *event )
{
#if 0
  if ( mMouseHandles->shouldBlockEvent( event ) )
  {
    //ignore wheel events while dragging/resizing items
    return;
  }
  else
#endif
  {
    event->ignore();
  }
}

void QgsModelViewToolSelect::keyPressEvent( QKeyEvent *event )
{

#if 0
  if ( mMouseHandles->isDragging() || mMouseHandles->isResizing() )
  {
    return;
  }
  else
#endif
  {
    event->ignore();
  }
}

void QgsModelViewToolSelect::deactivate()
{
  if ( mIsSelecting )
  {
    mRubberBand->finish();
    mIsSelecting = false;
  }
  QgsModelViewTool::deactivate();
}

bool QgsModelViewToolSelect::allowItemInteraction()
{
  return !mIsSelecting;
}

QgsModelViewMouseHandles *QgsModelViewToolSelect::mouseHandles()
{
  return nullptr; //mMouseHandles;
}

void QgsModelViewToolSelect::setModel( QgsProcessingAlgorithmModel *model )
{
#if 0
  // existing handles are owned by previous layout
  if ( mMouseHandles )
    mMouseHandles->deleteLater();

  //add mouse selection handles to layout, and initially hide
  mMouseHandles = new QgsLayoutMouseHandles( layout, view() );
  mMouseHandles->hide();
  mMouseHandles->setZValue( QgsLayout::ZMouseHandles );
  layout->addItem( mMouseHandles );
#endif
}
///@endcond
