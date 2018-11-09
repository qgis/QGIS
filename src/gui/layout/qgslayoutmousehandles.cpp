/***************************************************************************
                             qgslayoutmousehandles.cpp
                             ------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutmousehandles.h"
#include "qgis.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgslayout.h"
#include "qgslayoutitem.h"
#include "qgslayoututils.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtoolselect.h"
#include "qgslayoutsnapper.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutundostack.h"
#include <QGraphicsView>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QWidget>
#include <limits>

///@cond PRIVATE

QgsLayoutMouseHandles::QgsLayoutMouseHandles( QgsLayout *layout, QgsLayoutView *view )
  : QObject( nullptr )
  , QGraphicsRectItem( nullptr )
  , mLayout( layout )
  , mView( view )
{
  //listen for selection changes, and update handles accordingly
  connect( mLayout, &QGraphicsScene::selectionChanged, this, &QgsLayoutMouseHandles::selectionChanged );

  //accept hover events, required for changing cursor to resize cursors
  setAcceptHoverEvents( true );

  mHorizontalSnapLine = mView->createSnapLine();
  mHorizontalSnapLine->hide();
  layout->addItem( mHorizontalSnapLine );
  mVerticalSnapLine = mView->createSnapLine();
  mVerticalSnapLine->hide();
  layout->addItem( mVerticalSnapLine );
}

void QgsLayoutMouseHandles::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );

  if ( !mLayout->renderContext().isPreviewRender() )
  {
    //don't draw selection handles in layout outputs
    return;
  }

  if ( mLayout->renderContext().boundingBoxesVisible() )
  {
    //draw resize handles around bounds of entire selection
    double rectHandlerSize = rectHandlerBorderTolerance();
    drawHandles( painter, rectHandlerSize );
  }

  if ( mIsResizing || mIsDragging || mLayout->renderContext().boundingBoxesVisible() )
  {
    //draw dotted boxes around selected items
    drawSelectedItemBounds( painter );
  }
}

void QgsLayoutMouseHandles::drawHandles( QPainter *painter, double rectHandlerSize )
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

void QgsLayoutMouseHandles::drawSelectedItemBounds( QPainter *painter )
{
  //draw dotted border around selected items to give visual feedback which items are selected
  const QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
  if ( selectedItems.isEmpty() )
  {
    return;
  }

  //use difference mode so that they are visible regardless of item colors
  painter->save();
  painter->setCompositionMode( QPainter::CompositionMode_Difference );

  // use a grey dashed pen - in difference mode this should always be visible
  QPen selectedItemPen = QPen( QColor( 144, 144, 144, 255 ) );
  selectedItemPen.setStyle( Qt::DashLine );
  selectedItemPen.setWidth( 0 );
  painter->setPen( selectedItemPen );
  painter->setBrush( Qt::NoBrush );

  QList< QgsLayoutItem * > itemsToDraw;
  collectItems( selectedItems, itemsToDraw );

  for ( QgsLayoutItem *item : qgis::as_const( itemsToDraw ) )
  {
    //get bounds of selected item
    QPolygonF itemBounds;
    if ( mIsDragging && !item->isLocked() )
    {
      //if currently dragging, draw selected item bounds relative to current mouse position
      //first, get bounds of current item in scene coordinates
      QPolygonF itemSceneBounds = item->mapToScene( item->rectWithFrame() );
      //now, translate it by the current movement amount
      //IMPORTANT - this is done in scene coordinates, since we don't want any rotation/non-translation transforms to affect the movement
      itemSceneBounds.translate( transform().dx(), transform().dy() );
      //finally, remap it to the mouse handle item's coordinate system so it's ready for drawing
      itemBounds = mapFromScene( itemSceneBounds );
    }
    else if ( mIsResizing && !item->isLocked() )
    {
      //if currently resizing, calculate relative resize of this item
      if ( selectedItems.size() > 1 )
      {
        //get item bounds in mouse handle item's coordinate system
        QRectF itemRect = mapRectFromItem( item, item->rectWithFrame() );
        //now, resize it relative to the current resized dimensions of the mouse handles
        QgsLayoutUtils::relativeResizeRect( itemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );
        itemBounds = QPolygonF( itemRect );
      }
      else
      {
        //single item selected
        itemBounds = rect();
      }
    }
    else
    {
      //not resizing or moving, so just map from scene bounds
      itemBounds = mapRectFromItem( item, item->rectWithFrame() );
    }

    // drawPolygon causes issues on windows - corners of path may be missing resulting in triangles being drawn
    // instead of rectangles! (Same cause as #13343)
    QPainterPath path;
    path.addPolygon( itemBounds );
    painter->drawPath( path );
  }
  painter->restore();
}

void QgsLayoutMouseHandles::selectionChanged()
{
  //listen out for selected items' size and rotation changed signals
  const QList<QGraphicsItem *> itemList = layout()->items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    if ( !item )
      continue;

    if ( item->isSelected() )
    {
      connect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      connect( item, &QgsLayoutItem::rotationChanged, this, &QgsLayoutMouseHandles::selectedItemRotationChanged );
      connect( item, &QgsLayoutItem::frameChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      connect( item, &QgsLayoutItem::lockChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
    }
    else
    {
      disconnect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      disconnect( item, &QgsLayoutItem::rotationChanged, this, &QgsLayoutMouseHandles::selectedItemRotationChanged );
      disconnect( item, &QgsLayoutItem::frameChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      disconnect( item, &QgsLayoutItem::lockChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
    }
  }

  resetStatusBar();
  updateHandles();
}

void QgsLayoutMouseHandles::selectedItemSizeChanged()
{
  if ( !mIsDragging && !mIsResizing )
  {
    //only required for non-mouse initiated size changes
    updateHandles();
  }
}

void QgsLayoutMouseHandles::selectedItemRotationChanged()
{
  if ( !mIsDragging && !mIsResizing )
  {
    //only required for non-mouse initiated rotation changes
    updateHandles();
  }
}

void QgsLayoutMouseHandles::updateHandles()
{
  //recalculate size and position of handle item

  //first check to see if any items are selected
  QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
  if ( !selectedItems.isEmpty() )
  {
    //one or more items are selected, get bounds of all selected items

    //update rotation of handle object
    double rotation;
    if ( selectionRotation( rotation ) )
    {
      //all items share a common rotation value, so we rotate the mouse handles to match
      setRotation( rotation );
    }
    else
    {
      //items have varying rotation values - we can't rotate the mouse handles to match
      setRotation( 0 );
    }

    //get bounds of all selected items
    QRectF newHandleBounds = selectionBounds();

    //update size and position of handle object
    setRect( 0, 0, newHandleBounds.width(), newHandleBounds.height() );
    setPos( mapToScene( newHandleBounds.topLeft() ) );

    show();
  }
  else
  {
    //no items selected, hide handles
    hide();
  }
  //force redraw
  update();
}

QRectF QgsLayoutMouseHandles::selectionBounds() const
{
  //calculate bounds of all currently selected items in mouse handle coordinate system
  const QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
  auto itemIter = selectedItems.constBegin();

  //start with handle bounds of first selected item
  QRectF bounds = mapFromItem( ( *itemIter ), ( *itemIter )->rectWithFrame() ).boundingRect();

  //iterate through remaining items, expanding the bounds as required
  for ( ++itemIter; itemIter != selectedItems.constEnd(); ++itemIter )
  {
    bounds = bounds.united( mapFromItem( ( *itemIter ), ( *itemIter )->rectWithFrame() ).boundingRect() );
  }

  return bounds;
}

bool QgsLayoutMouseHandles::selectionRotation( double &rotation ) const
{
  //check if all selected items have same rotation
  QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
  auto itemIter = selectedItems.constBegin();

  //start with rotation of first selected item
  double firstItemRotation = ( *itemIter )->rotation();

  //iterate through remaining items, checking if they have same rotation
  for ( ++itemIter; itemIter != selectedItems.constEnd(); ++itemIter )
  {
    if ( !qgsDoubleNear( ( *itemIter )->rotation(), firstItemRotation ) )
    {
      //item has a different rotation, so return false
      return false;
    }
  }

  //all items have the same rotation, so set the rotation variable and return true
  rotation = firstItemRotation;
  return true;
}

double QgsLayoutMouseHandles::rectHandlerBorderTolerance()
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

Qt::CursorShape QgsLayoutMouseHandles::cursorForPosition( QPointF itemCoordPos )
{
  QgsLayoutMouseHandles::MouseAction mouseAction = mouseActionForPosition( itemCoordPos );
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

QgsLayoutMouseHandles::MouseAction QgsLayoutMouseHandles::mouseActionForPosition( QPointF itemCoordPos )
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
    return QgsLayoutMouseHandles::ResizeLeftUp;
  }
  else if ( nearLeftBorder && nearLowerBorder )
  {
    return QgsLayoutMouseHandles::ResizeLeftDown;
  }
  else if ( nearRightBorder && nearUpperBorder )
  {
    return QgsLayoutMouseHandles::ResizeRightUp;
  }
  else if ( nearRightBorder && nearLowerBorder )
  {
    return QgsLayoutMouseHandles::ResizeRightDown;
  }
  else if ( nearLeftBorder && withinHeight )
  {
    return QgsLayoutMouseHandles::ResizeLeft;
  }
  else if ( nearRightBorder && withinHeight )
  {
    return QgsLayoutMouseHandles::ResizeRight;
  }
  else if ( nearUpperBorder && withinWidth )
  {
    return QgsLayoutMouseHandles::ResizeUp;
  }
  else if ( nearLowerBorder && withinWidth )
  {
    return QgsLayoutMouseHandles::ResizeDown;
  }

  //find out if cursor position is over a selected item
  QPointF scenePoint = mapToScene( itemCoordPos );
  const QList<QGraphicsItem *> itemsAtCursorPos = mLayout->items( scenePoint );
  if ( itemsAtCursorPos.isEmpty() )
  {
    //no items at cursor position
    return QgsLayoutMouseHandles::SelectItem;
  }
  for ( QGraphicsItem *graphicsItem : itemsAtCursorPos )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    if ( item && item->isSelected() )
    {
      //cursor is over a selected layout item
      return QgsLayoutMouseHandles::MoveItem;
    }
  }

  //default
  return QgsLayoutMouseHandles::SelectItem;
}

QgsLayoutMouseHandles::MouseAction QgsLayoutMouseHandles::mouseActionForScenePos( QPointF sceneCoordPos )
{
  // convert sceneCoordPos to item coordinates
  QPointF itemPos = mapFromScene( sceneCoordPos );
  return mouseActionForPosition( itemPos );
}

bool QgsLayoutMouseHandles::shouldBlockEvent( QInputEvent * ) const
{
  return mIsDragging || mIsResizing;
}

void QgsLayoutMouseHandles::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
  setViewportCursor( cursorForPosition( event->pos() ) );
}

void QgsLayoutMouseHandles::hoverLeaveEvent( QGraphicsSceneHoverEvent *event )
{
  Q_UNUSED( event );
  setViewportCursor( Qt::ArrowCursor );
}

void QgsLayoutMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  //workaround qt bug #3732 by setting cursor for QGraphicsView viewport,
  //rather then setting it directly here

  if ( dynamic_cast< QgsLayoutViewToolSelect *>( mView->tool() ) )
  {
    mView->viewport()->setCursor( cursor );
  }
}

void QgsLayoutMouseHandles::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
  if ( mIsDragging )
  {
    //currently dragging a selection
    //if shift depressed, constrain movement to horizontal/vertical
    //if control depressed, ignore snapping
    dragMouseMove( event->lastScenePos(), event->modifiers() & Qt::ShiftModifier, event->modifiers() & Qt::ControlModifier );
  }
  else if ( mIsResizing )
  {
    //currently resizing a selection
    //lock aspect ratio if shift depressed
    //resize from center if alt depressed
    resizeMouseMove( event->lastScenePos(), event->modifiers() & Qt::ShiftModifier, event->modifiers() & Qt::AltModifier );
  }

  mLastMouseEventPos = event->lastScenePos();
}

void QgsLayoutMouseHandles::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
  QPointF mouseMoveStopPoint = event->lastScenePos();
  double diffX = mouseMoveStopPoint.x() - mMouseMoveStartPos.x();
  double diffY = mouseMoveStopPoint.y() - mMouseMoveStartPos.y();

  //it was only a click
  if ( std::fabs( diffX ) < std::numeric_limits<double>::min() && std::fabs( diffY ) < std::numeric_limits<double>::min() )
  {
    mIsDragging = false;
    mIsResizing = false;
    update();
    hideAlignItems();
    return;
  }

  if ( mCurrentMouseMoveAction == QgsLayoutMouseHandles::MoveItem )
  {
    //move selected items
    mLayout->undoStack()->beginMacro( tr( "Move Items" ) );

    QPointF mEndHandleMovePos = scenePos();

    double deltaX = mEndHandleMovePos.x() - mBeginHandlePos.x();
    double deltaY = mEndHandleMovePos.y() - mBeginHandlePos.y();

    //move all selected items
    const QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
    for ( QgsLayoutItem *item : selectedItems )
    {
      if ( item->isLocked() || ( item->flags() & QGraphicsItem::ItemIsSelectable ) == 0 || item->isGroupMember() )
      {
        //don't move locked items, or grouped items (group takes care of that)
        continue;
      }

      std::unique_ptr< QgsAbstractLayoutUndoCommand > command( item->createCommand( QString(), 0 ) );
      command->saveBeforeState();

      item->attemptMoveBy( deltaX, deltaY );

      command->saveAfterState();
      mLayout->undoStack()->push( command.release() );
    }
    mLayout->undoStack()->endMacro();
  }
  else if ( mCurrentMouseMoveAction != QgsLayoutMouseHandles::NoAction )
  {
    //resize selected items
    mLayout->undoStack()->beginMacro( tr( "Resize Items" ) );

    //resize all selected items
    const QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
    for ( QgsLayoutItem *item : selectedItems )
    {
      if ( item->isLocked() || ( item->flags() & QGraphicsItem::ItemIsSelectable ) == 0 )
      {
        //don't resize locked items or deselectable items (e.g., items which make up an item group)
        continue;
      }

      std::unique_ptr< QgsAbstractLayoutUndoCommand > command( item->createCommand( QString(), 0 ) );
      command->saveBeforeState();

      QRectF itemRect;
      if ( selectedItems.size() == 1 )
      {
        //only a single item is selected, so set its size to the final resized mouse handle size
        itemRect = mResizeRect;
      }
      else
      {
        //multiple items selected, so each needs to be scaled relatively to the final size of the mouse handles
        itemRect = mapRectFromItem( item, item->rectWithFrame() );
        QgsLayoutUtils::relativeResizeRect( itemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );
      }

      itemRect = itemRect.normalized();
      QPointF newPos = mapToScene( itemRect.topLeft() );

      QgsLayoutSize itemSize = mLayout->convertFromLayoutUnits( itemRect.size(), item->sizeWithUnits().units() );
      item->attemptResize( itemSize, true );

      // translate new position to current item units
      QgsLayoutPoint itemPos = mLayout->convertFromLayoutUnits( newPos, item->positionWithUnits().units() );
      item->attemptMove( itemPos, false, true );

      command->saveAfterState();
      mLayout->undoStack()->push( command.release() );
    }
    mLayout->undoStack()->endMacro();
  }

  hideAlignItems();
  if ( mIsDragging )
  {
    mIsDragging = false;
  }
  if ( mIsResizing )
  {
    mIsResizing = false;
  }

  //reset default action
  mCurrentMouseMoveAction = QgsLayoutMouseHandles::MoveItem;
  setViewportCursor( Qt::ArrowCursor );
  //redraw handles
  resetTransform();
  updateHandles();
  //reset status bar message
  resetStatusBar();
}

void QgsLayoutMouseHandles::resetStatusBar()
{
  if ( !mView )
    return;

  const QList<QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems( false );
  int selectedCount = selectedItems.size();
  if ( selectedCount > 1 )
  {
    //set status bar message to count of selected items
    mView->pushStatusMessage( tr( "%1 items selected" ).arg( selectedCount ) );
  }
  else if ( selectedCount == 1 )
  {
    //set status bar message to count of selected items
    mView->pushStatusMessage( tr( "1 item selected" ) );
  }
  else
  {
    //clear status bar message
    mView->pushStatusMessage( QString() );
  }
}

QPointF QgsLayoutMouseHandles::snapPoint( QPointF originalPoint, QgsLayoutMouseHandles::SnapGuideMode mode, bool snapHorizontal, bool snapVertical )
{
  bool snapped = false;

  const QList< QgsLayoutItem * > selectedItems = mLayout->selectedLayoutItems();
  QList< QgsLayoutItem * > itemsToExclude;
  collectItems( selectedItems, itemsToExclude );

  //depending on the mode, we either snap just the single point, or all the bounds of the selection
  QPointF snappedPoint;
  switch ( mode )
  {
    case Item:
      snappedPoint = mLayout->snapper().snapRect( rect().translated( originalPoint ), mView->transform().m11(), snapped, snapHorizontal ? mHorizontalSnapLine : nullptr,
                     snapVertical ? mVerticalSnapLine : nullptr, &itemsToExclude ).topLeft();
      break;
    case Point:
      snappedPoint = mLayout->snapper().snapPoint( originalPoint, mView->transform().m11(), snapped, snapHorizontal ? mHorizontalSnapLine : nullptr,
                     snapVertical ? mVerticalSnapLine : nullptr, &itemsToExclude );
      break;
  }

  return snapped ? snappedPoint : originalPoint;
}

void QgsLayoutMouseHandles::hideAlignItems()
{
  mHorizontalSnapLine->hide();
  mVerticalSnapLine->hide();
}

void QgsLayoutMouseHandles::collectItems( const QList<QgsLayoutItem *> items, QList<QgsLayoutItem *> &collected )
{
  for ( QgsLayoutItem *item : items )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutGroup )
    {
      // if a group is selected, we don't draw the bounds of the group - instead we draw the bounds of the grouped items
      collectItems( static_cast< QgsLayoutItemGroup * >( item )->items(), collected );
    }
    else
    {
      collected << item;
    }
  }
}

void QgsLayoutMouseHandles::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
  //save current cursor position
  mMouseMoveStartPos = event->lastScenePos();
  mLastMouseEventPos = event->lastScenePos();
  //save current item geometry
  mBeginMouseEventPos = event->lastScenePos();
  mBeginHandlePos = scenePos();
  mBeginHandleWidth = rect().width();
  mBeginHandleHeight = rect().height();
  //type of mouse move action
  mCurrentMouseMoveAction = mouseActionForPosition( event->pos() );

  hideAlignItems();

  if ( mCurrentMouseMoveAction == QgsLayoutMouseHandles::MoveItem )
  {
    //moving items
    mIsDragging = true;
  }
  else if ( mCurrentMouseMoveAction != QgsLayoutMouseHandles::SelectItem &&
            mCurrentMouseMoveAction != QgsLayoutMouseHandles::NoAction )
  {
    //resizing items
    mIsResizing = true;
    mResizeRect = QRectF( 0, 0, mBeginHandleWidth, mBeginHandleHeight );
    mResizeMoveX = 0;
    mResizeMoveY = 0;
    mCursorOffset = calcCursorEdgeOffset( mMouseMoveStartPos );

  }

}

void QgsLayoutMouseHandles::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
  Q_UNUSED( event );
}

QSizeF QgsLayoutMouseHandles::calcCursorEdgeOffset( QPointF cursorPos )
{
  //find offset between cursor position and actual edge of item
  QPointF sceneMousePos = mapFromScene( cursorPos );

  switch ( mCurrentMouseMoveAction )
  {
    //vertical resize
    case QgsLayoutMouseHandles::ResizeUp:
      return QSizeF( 0, sceneMousePos.y() );

    case QgsLayoutMouseHandles::ResizeDown:
      return QSizeF( 0, sceneMousePos.y() - rect().height() );

    //horizontal resize
    case QgsLayoutMouseHandles::ResizeLeft:
      return QSizeF( sceneMousePos.x(), 0 );

    case QgsLayoutMouseHandles::ResizeRight:
      return QSizeF( sceneMousePos.x() - rect().width(), 0 );

    //diagonal resize
    case QgsLayoutMouseHandles::ResizeLeftUp:
      return QSizeF( sceneMousePos.x(), sceneMousePos.y() );

    case QgsLayoutMouseHandles::ResizeRightDown:
      return QSizeF( sceneMousePos.x() - rect().width(), sceneMousePos.y() - rect().height() );

    case QgsLayoutMouseHandles::ResizeRightUp:
      return QSizeF( sceneMousePos.x() - rect().width(), sceneMousePos.y() );

    case QgsLayoutMouseHandles::ResizeLeftDown:
      return QSizeF( sceneMousePos.x(), sceneMousePos.y() - rect().height() );

    case MoveItem:
    case SelectItem:
    case NoAction:
      return QSizeF();
  }

  return QSizeF();
}

void QgsLayoutMouseHandles::dragMouseMove( QPointF currentPosition, bool lockMovement, bool preventSnap )
{
  if ( !mLayout )
  {
    return;
  }

  //calculate total amount of mouse movement since drag began
  double moveX = currentPosition.x() - mBeginMouseEventPos.x();
  double moveY = currentPosition.y() - mBeginMouseEventPos.y();

  //find target position before snapping (in scene coordinates)
  QPointF upperLeftPoint( mBeginHandlePos.x() + moveX, mBeginHandlePos.y() + moveY );

  QPointF snappedLeftPoint;

  //no snapping for rotated items for now
  if ( !preventSnap && qgsDoubleNear( rotation(), 0.0 ) )
  {
    //snap to grid and guides
    snappedLeftPoint = snapPoint( upperLeftPoint, QgsLayoutMouseHandles::Item );

  }
  else
  {
    //no snapping
    snappedLeftPoint = upperLeftPoint;
    hideAlignItems();
  }

  //calculate total shift for item from beginning of drag operation to current position
  double moveRectX = snappedLeftPoint.x() - mBeginHandlePos.x();
  double moveRectY = snappedLeftPoint.y() - mBeginHandlePos.y();

  if ( lockMovement )
  {
    //constrained (shift) moving should lock to horizontal/vertical movement
    //reset the smaller of the x/y movements
    if ( std::fabs( moveRectX ) <= std::fabs( moveRectY ) )
    {
      moveRectX = 0;
    }
    else
    {
      moveRectY = 0;
    }
  }

  //shift handle item to new position
  QTransform moveTransform;
  moveTransform.translate( moveRectX, moveRectY );
  setTransform( moveTransform );

  //show current displacement of selection in status bar
  mView->pushStatusMessage( tr( "dx: %1 mm dy: %2 mm" ).arg( moveRectX ).arg( moveRectY ) );
}

void QgsLayoutMouseHandles::resizeMouseMove( QPointF currentPosition, bool lockRatio, bool fromCenter )
{

  if ( !mLayout )
  {
    return;
  }

  double mx = 0.0, my = 0.0, rx = 0.0, ry = 0.0;

  QPointF beginMousePos;
  QPointF finalPosition;
  if ( qgsDoubleNear( rotation(), 0.0 ) )
  {
    //snapping only occurs if handles are not rotated for now

    bool snapVertical = mCurrentMouseMoveAction == ResizeLeft ||
                        mCurrentMouseMoveAction == ResizeRight ||
                        mCurrentMouseMoveAction == ResizeLeftUp ||
                        mCurrentMouseMoveAction == ResizeRightUp ||
                        mCurrentMouseMoveAction == ResizeLeftDown ||
                        mCurrentMouseMoveAction == ResizeRightDown;

    bool snapHorizontal = mCurrentMouseMoveAction == ResizeUp ||
                          mCurrentMouseMoveAction == ResizeDown ||
                          mCurrentMouseMoveAction == ResizeLeftUp ||
                          mCurrentMouseMoveAction == ResizeRightUp ||
                          mCurrentMouseMoveAction == ResizeLeftDown ||
                          mCurrentMouseMoveAction == ResizeRightDown;

    //subtract cursor edge offset from begin mouse event and current cursor position, so that snapping occurs to edge of mouse handles
    //rather then cursor position
    beginMousePos = mapFromScene( QPointF( mBeginMouseEventPos.x() - mCursorOffset.width(), mBeginMouseEventPos.y() - mCursorOffset.height() ) );
    QPointF snappedPosition = snapPoint( QPointF( currentPosition.x() - mCursorOffset.width(), currentPosition.y() - mCursorOffset.height() ), QgsLayoutMouseHandles::Point, snapHorizontal, snapVertical );
    finalPosition = mapFromScene( snappedPosition );
  }
  else
  {
    //no snapping for rotated items for now
    beginMousePos = mapFromScene( mBeginMouseEventPos );
    finalPosition = mapFromScene( currentPosition );
  }

  double diffX = finalPosition.x() - beginMousePos.x();
  double diffY = finalPosition.y() - beginMousePos.y();

  double ratio = 0;
  if ( lockRatio && !qgsDoubleNear( mBeginHandleHeight, 0.0 ) )
  {
    ratio = mBeginHandleWidth / mBeginHandleHeight;
  }

  switch ( mCurrentMouseMoveAction )
  {
    //vertical resize
    case QgsLayoutMouseHandles::ResizeUp:
    {
      if ( ratio )
      {
        diffX = ( ( mBeginHandleHeight - diffY ) * ratio ) - mBeginHandleWidth;
        mx = -diffX / 2;
        my = diffY;
        rx = diffX;
        ry = -diffY;
      }
      else
      {
        mx = 0;
        my = diffY;
        rx = 0;
        ry = -diffY;
      }
      break;
    }

    case QgsLayoutMouseHandles::ResizeDown:
    {
      if ( ratio )
      {
        diffX = ( ( mBeginHandleHeight + diffY ) * ratio ) - mBeginHandleWidth;
        mx = -diffX / 2;
        my = 0;
        rx = diffX;
        ry = diffY;
      }
      else
      {
        mx = 0;
        my = 0;
        rx = 0;
        ry = diffY;
      }
      break;
    }

    //horizontal resize
    case QgsLayoutMouseHandles::ResizeLeft:
    {
      if ( ratio )
      {
        diffY = ( ( mBeginHandleWidth - diffX ) / ratio ) - mBeginHandleHeight;
        mx = diffX;
        my = -diffY / 2;
        rx = -diffX;
        ry = diffY;
      }
      else
      {
        mx = diffX, my = 0;
        rx = -diffX;
        ry = 0;
      }
      break;
    }

    case QgsLayoutMouseHandles::ResizeRight:
    {
      if ( ratio )
      {
        diffY = ( ( mBeginHandleWidth + diffX ) / ratio ) - mBeginHandleHeight;
        mx = 0;
        my = -diffY / 2;
        rx = diffX;
        ry = diffY;
      }
      else
      {
        mx = 0;
        my = 0;
        rx = diffX, ry = 0;
      }
      break;
    }

    //diagonal resize
    case QgsLayoutMouseHandles::ResizeLeftUp:
    {
      if ( ratio )
      {
        //ratio locked resize
        if ( ( mBeginHandleWidth - diffX ) / ( mBeginHandleHeight - diffY ) > ratio )
        {
          diffX = mBeginHandleWidth - ( ( mBeginHandleHeight - diffY ) * ratio );
        }
        else
        {
          diffY = mBeginHandleHeight - ( ( mBeginHandleWidth - diffX ) / ratio );
        }
      }
      mx = diffX, my = diffY;
      rx = -diffX;
      ry = -diffY;
      break;
    }

    case QgsLayoutMouseHandles::ResizeRightDown:
    {
      if ( ratio )
      {
        //ratio locked resize
        if ( ( mBeginHandleWidth + diffX ) / ( mBeginHandleHeight + diffY ) > ratio )
        {
          diffX = ( ( mBeginHandleHeight + diffY ) * ratio ) - mBeginHandleWidth;
        }
        else
        {
          diffY = ( ( mBeginHandleWidth + diffX ) / ratio ) - mBeginHandleHeight;
        }
      }
      mx = 0;
      my = 0;
      rx = diffX, ry = diffY;
      break;
    }

    case QgsLayoutMouseHandles::ResizeRightUp:
    {
      if ( ratio )
      {
        //ratio locked resize
        if ( ( mBeginHandleWidth + diffX ) / ( mBeginHandleHeight - diffY ) > ratio )
        {
          diffX = ( ( mBeginHandleHeight - diffY ) * ratio ) - mBeginHandleWidth;
        }
        else
        {
          diffY = mBeginHandleHeight - ( ( mBeginHandleWidth + diffX ) / ratio );
        }
      }
      mx = 0;
      my = diffY, rx = diffX, ry = -diffY;
      break;
    }

    case QgsLayoutMouseHandles::ResizeLeftDown:
    {
      if ( ratio )
      {
        //ratio locked resize
        if ( ( mBeginHandleWidth - diffX ) / ( mBeginHandleHeight + diffY ) > ratio )
        {
          diffX = mBeginHandleWidth - ( ( mBeginHandleHeight + diffY ) * ratio );
        }
        else
        {
          diffY = ( ( mBeginHandleWidth - diffX ) / ratio ) - mBeginHandleHeight;
        }
      }
      mx = diffX, my = 0;
      rx = -diffX;
      ry = diffY;
      break;
    }

    case QgsLayoutMouseHandles::MoveItem:
    case QgsLayoutMouseHandles::SelectItem:
    case QgsLayoutMouseHandles::NoAction:
      break;
  }

  //resizing from center of objects?
  if ( fromCenter )
  {
    my = -ry;
    mx = -rx;
    ry = 2 * ry;
    rx = 2 * rx;
  }

  //update selection handle rectangle

  //make sure selection handle size rectangle is normalized (ie, left coord < right coord)
  mResizeMoveX = mBeginHandleWidth + rx > 0 ? mx : mx + mBeginHandleWidth + rx;
  mResizeMoveY = mBeginHandleHeight + ry > 0 ? my : my + mBeginHandleHeight + ry;

  //calculate movement in scene coordinates
  QLineF translateLine = QLineF( 0, 0, mResizeMoveX, mResizeMoveY );
  translateLine.setAngle( translateLine.angle() - rotation() );
  QPointF sceneTranslate = translateLine.p2();

  //move selection handles
  QTransform itemTransform;
  itemTransform.translate( sceneTranslate.x(), sceneTranslate.y() );
  setTransform( itemTransform );

  //handle non-normalised resizes - e.g., dragging the left handle so far to the right that it's past the right handle
  if ( mBeginHandleWidth + rx >= 0 && mBeginHandleHeight + ry >= 0 )
  {
    mResizeRect = QRectF( 0, 0, mBeginHandleWidth + rx, mBeginHandleHeight + ry );
  }
  else if ( mBeginHandleHeight + ry >= 0 )
  {
    mResizeRect = QRectF( QPointF( -( mBeginHandleWidth + rx ), 0 ), QPointF( 0, mBeginHandleHeight + ry ) );
  }
  else if ( mBeginHandleWidth + rx >= 0 )
  {
    mResizeRect = QRectF( QPointF( 0, -( mBeginHandleHeight + ry ) ), QPointF( mBeginHandleWidth + rx, 0 ) );
  }
  else
  {
    mResizeRect = QRectF( QPointF( -( mBeginHandleWidth + rx ), -( mBeginHandleHeight + ry ) ), QPointF( 0, 0 ) );
  }

  setRect( 0, 0, std::fabs( mBeginHandleWidth + rx ), std::fabs( mBeginHandleHeight + ry ) );

  //show current size of selection in status bar
  mView->pushStatusMessage( tr( "width: %1 mm height: %2 mm" ).arg( rect().width() ).arg( rect().height() ) );
}

///@endcond PRIVATE
