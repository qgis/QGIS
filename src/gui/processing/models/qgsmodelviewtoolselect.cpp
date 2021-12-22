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
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelviewmousehandles.h"
#include "qgsmodelgraphicitem.h"

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
  if ( mMouseHandles )
  {
    // want to force them to be removed from the scene
    if ( mMouseHandles->scene() )
      mMouseHandles->scene()->removeItem( mMouseHandles );
    delete mMouseHandles;
  }
}

void QgsModelViewToolSelect::modelPressEvent( QgsModelViewMouseEvent *event )
{
  if ( mMouseHandles->shouldBlockEvent( event ) )
  {
    //swallow clicks while dragging/resizing items
    return;
  }

  if ( mMouseHandles->isVisible() )
  {
    //selection handles are being shown, get mouse action for current cursor position
    QgsGraphicsViewMouseHandles::MouseAction mouseAction = mMouseHandles->mouseActionForScenePos( event->modelPoint() );

    if ( mouseAction != QgsGraphicsViewMouseHandles::MoveItem
         && mouseAction != QgsGraphicsViewMouseHandles::NoAction
         && mouseAction != QgsGraphicsViewMouseHandles::SelectItem )
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


  QgsModelComponentGraphicItem *selectedItem = nullptr;

  //select topmost item at position of event
  selectedItem = scene()->componentItemAt( event->modelPoint() );

  if ( !selectedItem )
  {
    //not clicking over an item, so start marquee selection
    mIsSelecting = true;
    mMousePressStartPos = event->pos();
    mRubberBand->start( event->modelPoint(), Qt::KeyboardModifiers() );
    return;
  }

  if ( ( event->modifiers() & Qt::ShiftModifier ) && ( selectedItem->isSelected() ) )
  {
    //SHIFT-clicking a selected item deselects it
    selectedItem->setSelected( false );

    //Check if we have any remaining selected items, and if so, update the item panel
    const QList<QgsModelComponentGraphicItem *> selectedItems = scene()->selectedComponentItems();
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
      scene()->setSelectedItem( selectedItem ); // clears existing selection
    }
    else
    {
      selectedItem->setSelected( true );
    }
    event->ignore();
    emit itemFocused( selectedItem );

    if ( !( event->modifiers() & Qt::ShiftModifier ) )
    {
      // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate events
      // to multiple items
      QList< QGraphicsItem * > items = scene()->items( event->modelPoint() );
      for ( QGraphicsItem *item : items )
      {
        if ( QgsModelDesignerFlatButtonGraphicItem *button = dynamic_cast< QgsModelDesignerFlatButtonGraphicItem * >( item ) )
        {
          // arghhh - if the event happens outside the mouse handles bounding rect, then it's ALREADY passed on!
          if ( mMouseHandles->sceneBoundingRect().contains( event->modelPoint() ) )
          {
            button->modelPressEvent( event );
            event->accept();
            return;
          }
        }
      }
    }
  }

  event->ignore();
}

void QgsModelViewToolSelect::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  if ( mIsSelecting )
  {
    mRubberBand->update( event->modelPoint(), Qt::KeyboardModifiers() );
  }
  else
  {
    // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate events
    // to multiple items
    QList< QGraphicsItem * > items = scene()->items( event->modelPoint() );
    for ( QGraphicsItem *item : items )
    {
      if ( mHoverEnteredItems.contains( item ) )
      {
        if ( QgsModelComponentGraphicItem *component = dynamic_cast< QgsModelComponentGraphicItem * >( item ) )
        {
          component->modelHoverMoveEvent( event );
        }
      }
      else
      {
        mHoverEnteredItems.append( item );
        if ( QgsModelComponentGraphicItem *component = dynamic_cast< QgsModelComponentGraphicItem * >( item ) )
        {
          component->modelHoverEnterEvent( event );
        }
        else if ( QgsModelDesignerFlatButtonGraphicItem *button = dynamic_cast<QgsModelDesignerFlatButtonGraphicItem *>( item ) )
        {
          // arghhh - if the event happens outside the mouse handles bounding rect, then it's ALREADY passed on!
          if ( mMouseHandles->sceneBoundingRect().contains( event->modelPoint() ) )
            button->modelHoverEnterEvent( event );
        }
      }
    }
    const QList< QGraphicsItem * > prevHovered = mHoverEnteredItems;
    for ( QGraphicsItem *item : prevHovered )
    {
      if ( ! items.contains( item ) )
      {
        mHoverEnteredItems.removeAll( item );
        if ( QgsModelComponentGraphicItem *component = dynamic_cast< QgsModelComponentGraphicItem * >( item ) )
        {
          component->modelHoverLeaveEvent( event );
        }
        else if ( QgsModelDesignerFlatButtonGraphicItem *button = dynamic_cast<QgsModelDesignerFlatButtonGraphicItem *>( item ) )
        {
          // arghhh - if the event happens outside the mouse handles bounding rect, then it's ALREADY passed on!
          if ( mMouseHandles->sceneBoundingRect().contains( event->modelPoint() ) )
            button->modelHoverLeaveEvent( event );
        }
      }
    }

    event->ignore();
  }
}

void QgsModelViewToolSelect::modelDoubleClickEvent( QgsModelViewMouseEvent *event )
{
  if ( !mIsSelecting )
  {
    // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate events
    // to multiple items
    QList< QGraphicsItem * > items = scene()->items( event->modelPoint() );
    for ( QGraphicsItem *item : items )
    {
      if ( QgsModelComponentGraphicItem *component = dynamic_cast< QgsModelComponentGraphicItem * >( item ) )
      {
        scene()->setSelectedItem( component ); // clears existing selection
        component->modelDoubleClickEvent( event );
        break;
      }
    }
  }
}

void QgsModelViewToolSelect::modelReleaseEvent( QgsModelViewMouseEvent *event )
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
    whileBlocking( scene() )->deselectAll();
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
    itemList = scene()->items( rect.center(), selectionMode );
  else
    itemList = scene()->items( rect, selectionMode );
  for ( QGraphicsItem *item : std::as_const( itemList ) )
  {
    if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
    {
      if ( subtractingSelection )
      {
        componentItem->setSelected( false );
      }
      else
      {
        componentItem->setSelected( true );
      }
      if ( wasClick )
      {
        // found an item, and only a click - nothing more to do
        break;
      }
    }
  }

  //update item panel
  const QList<QgsModelComponentGraphicItem *> selectedItemList = scene()->selectedComponentItems();
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

void QgsModelViewToolSelect::wheelEvent( QWheelEvent *event )
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

void QgsModelViewToolSelect::keyPressEvent( QKeyEvent *event )
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
  return mMouseHandles;
}

void QgsModelViewToolSelect::setScene( QgsModelGraphicsScene *scene )
{
  // existing handles are owned by previous layout
  if ( mMouseHandles )
    mMouseHandles->deleteLater();

  //add mouse selection handles to scene, and initially hide
  mMouseHandles = new QgsModelViewMouseHandles( view() );
  mMouseHandles->hide();
  mMouseHandles->setZValue( QgsModelGraphicsScene::MouseHandles );
  scene->addItem( mMouseHandles );
}

void QgsModelViewToolSelect::resetCache()
{
  mHoverEnteredItems.clear();
}
///@endcond
