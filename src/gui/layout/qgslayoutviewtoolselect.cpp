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
#include "moc_qgslayoutviewtoolselect.cpp"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitempage.h"
#include "qgslayoutmousehandles.h"
#include "qgslayoutitemgroup.h"


const double QgsLayoutViewToolSelect::sSearchToleranceInMillimeters = 2.0;


QgsLayoutViewToolSelect::QgsLayoutViewToolSelect( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::ArrowCursor );

  mRubberBand.reset( new QgsLayoutViewRectangularRubberBand( view ) );
  mRubberBand->setBrush( QBrush( QColor( 224, 178, 76, 63 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 254, 58, 29, 100 ) ), 0, Qt::DotLine ) );
}

QgsLayoutViewToolSelect::~QgsLayoutViewToolSelect()
{
  if ( mMouseHandles )
  {
    // want to force them to be removed from the scene
    if ( mMouseHandles->scene() )
      mMouseHandles->scene()->removeItem( mMouseHandles );
    mMouseHandles->deleteLater();
  }
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
    Qgis::MouseHandlesAction mouseAction = mMouseHandles->mouseActionForScenePos( event->layoutPoint() );

    if ( mouseAction != Qgis::MouseHandlesAction::MoveItem
         && mouseAction != Qgis::MouseHandlesAction::NoAction
         && mouseAction != Qgis::MouseHandlesAction::SelectItem )
    {
      //mouse is over a resize handle, so propagate event onward
      event->ignore();
      return;
    }
  }

  if ( event->button() != Qt::LeftButton )
  {
    if ( mIsSelecting )
    {
      mIsSelecting = false;
      mRubberBand->finish();
    }
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
    selectedItem = layout()->layoutItemAt( event->layoutPoint(), previousSelectedItem, true, searchToleranceInLayoutUnits() );

    //if we didn't find a lower item we'll use the top-most as fall-back
    //this duplicates mapinfo/illustrator/etc behavior where ctrl-clicks are "cyclic"
    if ( !selectedItem )
    {
      selectedItem = layout()->layoutItemAt( event->layoutPoint(), true, searchToleranceInLayoutUnits() );
    }
  }
  else
  {
    //select topmost item at position of event
    selectedItem = layout()->layoutItemAt( event->layoutPoint(), true, searchToleranceInLayoutUnits() );
  }

  // if selected item is in a group, we actually get the top-level group it's part of
  QgsLayoutItemGroup *group = selectedItem ? selectedItem->parentGroup() : nullptr;
  while ( group && group->parentGroup() )
  {
    group = group->parentGroup();
  }
  if ( group )
    selectedItem = group;

  if ( !selectedItem )
  {
    //not clicking over an item, so start marquee selection
    mIsSelecting = true;
    mMousePressStartPos = event->pos();
    mRubberBand->start( event->layoutPoint(), Qt::KeyboardModifiers() );
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
    if ( ( !selectedItem->isSelected() ) &&            //keep selection if an already selected item pressed
         !( event->modifiers() & Qt::ShiftModifier ) ) //keep selection if shift key pressed
    {
      layout()->setSelectedItem( selectedItem ); // clears existing selection
    }
    else
    {
      selectedItem->setSelected( true );
    }

    // Due to the selection tolerance, items can be selected while the mouse is not over them,
    // so we cannot just forward the mouse press event by calling event->ignore().
    // We call startMove to simulate a mouse press event on the handles
    mMouseHandles->startMove( view()->mapToScene( event->pos() ) );

    emit itemFocused( selectedItem );
  }
}

void QgsLayoutViewToolSelect::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( mIsSelecting )
  {
    mRubberBand->update( event->layoutPoint(), Qt::KeyboardModifiers() );
  }
  else
  {
    if ( !mMouseHandles->isDragging() && !mMouseHandles->isResizing() )
    {
      if ( layout()->layoutItemAt( event->layoutPoint(), true, searchToleranceInLayoutUnits() ) )
      {
        view()->viewport()->setCursor( Qt::SizeAllCursor );
      }
      else
      {
        view()->viewport()->setCursor( Qt::ArrowCursor );
      }
    }
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

  // important -- we don't pass the event modifiers here, because we use them for a different meaning!
  // (modifying how the selection interacts with the items, rather than modifying the selection shape)
  QRectF rect = mRubberBand->finish( event->layoutPoint() );

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
  {
    const double tolerance = searchToleranceInLayoutUnits();
    itemList = layout()->items( QRectF( rect.center().x() - tolerance, rect.center().y() - tolerance, 2 * tolerance, 2 * tolerance ), selectionMode );
  }
  else
    itemList = layout()->items( rect, selectionMode );

  QgsLayoutItemPage *focusedPaperItem = nullptr;
  for ( QGraphicsItem *item : std::as_const( itemList ) )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item );
    QgsLayoutItemPage *paperItem = dynamic_cast<QgsLayoutItemPage *>( item );
    if ( paperItem )
      focusedPaperItem = paperItem;

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
  else if ( focusedPaperItem )
  {
    emit itemFocused( focusedPaperItem );
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
  if ( mMouseHandles )
    mMouseHandles->deleteLater();

  //add mouse selection handles to layout, and initially hide
  mMouseHandles = new QgsLayoutMouseHandles( layout, view() );
  mMouseHandles->hide();
  mMouseHandles->setZValue( QgsLayout::ZMouseHandles );
  layout->addItem( mMouseHandles );
}
double QgsLayoutViewToolSelect::searchToleranceInLayoutUnits()
{
  const double pixelsPerMm = view()->physicalDpiX() / 25.4;
  return sSearchToleranceInMillimeters * pixelsPerMm / view()->transform().m11();
}
///@endcond
