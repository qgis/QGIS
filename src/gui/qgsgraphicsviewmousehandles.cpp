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
#include "qgsrendercontext.h"
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

void QgsGraphicsViewMouseHandles::paintInternal( QPainter *painter, bool showHandles, bool showStaticBoundingBoxes, bool showTemporaryBoundingBoxes, const QStyleOptionGraphicsItem *, QWidget * )
{
  if ( !showHandles )
  {
    return;
  }

  if ( showStaticBoundingBoxes )
  {
    //draw resize handles around bounds of entire selection
    double rectHandlerSize = rectHandlerBorderTolerance();
    drawHandles( painter, rectHandlerSize );
  }

  if ( showTemporaryBoundingBoxes && ( mIsResizing || mIsDragging || showStaticBoundingBoxes ) )
  {
    //draw dotted boxes around selected items
    drawSelectedItemBounds( painter );
  }
}

QRectF QgsGraphicsViewMouseHandles::storedItemRect( QGraphicsItem *item ) const
{
  return itemRect( item );
}

void QgsGraphicsViewMouseHandles::previewItemMove( QGraphicsItem *, double, double )
{

}

QRectF QgsGraphicsViewMouseHandles::previewSetItemRect( QGraphicsItem *, QRectF )
{
  return QRectF();
}

void QgsGraphicsViewMouseHandles::startMacroCommand( const QString & )
{

}

void QgsGraphicsViewMouseHandles::endMacroCommand()
{

}

void QgsGraphicsViewMouseHandles::endItemCommand( QGraphicsItem * )
{

}

void QgsGraphicsViewMouseHandles::createItemCommand( QGraphicsItem * )
{

}

QPointF QgsGraphicsViewMouseHandles::snapPoint( QPointF originalPoint, QgsGraphicsViewMouseHandles::SnapGuideMode, bool, bool )
{
  return originalPoint;
}

void QgsGraphicsViewMouseHandles::expandItemList( const QList<QGraphicsItem *> &items, QList<QGraphicsItem *> &collected ) const
{
  collected = items;
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

void QgsGraphicsViewMouseHandles::drawSelectedItemBounds( QPainter *painter )
{
  //draw dotted border around selected items to give visual feedback which items are selected
  const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
  if ( selectedItems.isEmpty() )
  {
    return;
  }

  //use difference mode so that they are visible regardless of item colors
  QgsScopedQPainterState painterState( painter );
  painter->setCompositionMode( QPainter::CompositionMode_Difference );

  // use a grey dashed pen - in difference mode this should always be visible
  QPen selectedItemPen = QPen( QColor( 144, 144, 144, 255 ) );
  selectedItemPen.setStyle( Qt::DashLine );
  selectedItemPen.setWidth( 0 );
  painter->setPen( selectedItemPen );
  painter->setBrush( Qt::NoBrush );

  QList< QGraphicsItem * > itemsToDraw;
  expandItemList( selectedItems, itemsToDraw );

  for ( QGraphicsItem *item : std::as_const( itemsToDraw ) )
  {
    //get bounds of selected item
    QPolygonF itemBounds;
    if ( isDragging() && !itemIsLocked( item ) )
    {
      //if currently dragging, draw selected item bounds relative to current mouse position
      //first, get bounds of current item in scene coordinates
      QPolygonF itemSceneBounds = item->mapToScene( itemRect( item ) );
      //now, translate it by the current movement amount
      //IMPORTANT - this is done in scene coordinates, since we don't want any rotation/non-translation transforms to affect the movement
      itemSceneBounds.translate( transform().dx(), transform().dy() );
      //finally, remap it to the mouse handle item's coordinate system so it's ready for drawing
      itemBounds = mapFromScene( itemSceneBounds );
    }
    else if ( isResizing() && !itemIsLocked( item ) )
    {
      //if currently resizing, calculate relative resize of this item
      if ( selectedItems.size() > 1 )
      {
        //get item bounds in mouse handle item's coordinate system
        QRectF thisItemRect = mapRectFromItem( item, itemRect( item ) );
        //now, resize it relative to the current resized dimensions of the mouse handles
        relativeResizeRect( thisItemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );
        itemBounds = QPolygonF( thisItemRect );
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
      itemBounds = mapRectFromItem( item, itemRect( item ) );
    }

    // drawPolygon causes issues on windows - corners of path may be missing resulting in triangles being drawn
    // instead of rectangles! (Same cause as #13343)
    QPainterPath path;
    path.addPolygon( itemBounds );
    painter->drawPath( path );
  }
}

double QgsGraphicsViewMouseHandles::rectHandlerBorderTolerance()
{
  if ( !mView )
    return 0;

  //calculate size for resize handles
  //get view scale factor
  double viewScaleFactor = mView->transform().m11();

  //size of handle boxes depends on zoom level in layout view
  double rectHandlerSize = mHandleSize / viewScaleFactor;

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

void QgsGraphicsViewMouseHandles::selectedItemSizeChanged()
{
  if ( !isDragging() && !isResizing() )
  {
    //only required for non-mouse initiated size changes
    updateHandles();
  }
}

void QgsGraphicsViewMouseHandles::selectedItemRotationChanged()
{
  if ( !isDragging() && !isResizing() )
  {
    //only required for non-mouse initiated rotation changes
    updateHandles();
  }
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

void QgsGraphicsViewMouseHandles::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

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

  if ( mCurrentMouseMoveAction == MoveItem )
  {
    //moving items
    mIsDragging = true;
  }
  else if ( mCurrentMouseMoveAction != SelectItem &&
            mCurrentMouseMoveAction != NoAction )
  {
    //resizing items
    mIsResizing = true;
    mResizeRect = QRectF( 0, 0, mBeginHandleWidth, mBeginHandleHeight );
    mResizeMoveX = 0;
    mResizeMoveY = 0;
    mCursorOffset = calcCursorEdgeOffset( mMouseMoveStartPos );

  }
}

void QgsGraphicsViewMouseHandles::resetStatusBar()
{
  const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
  int selectedCount = selectedItems.size();
  if ( selectedCount )
  {
    //set status bar message to count of selected items
    showStatusMessage( tr( "%n item(s) selected", nullptr, selectedCount ) );
  }
  else
  {
    //clear status bar message
    showStatusMessage( QString() );
  }
}

void QgsGraphicsViewMouseHandles::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
  if ( isDragging() )
  {
    //currently dragging a selection
    //if shift depressed, constrain movement to horizontal/vertical
    //if control depressed, ignore snapping
    dragMouseMove( event->lastScenePos(), event->modifiers() & Qt::ShiftModifier, event->modifiers() & Qt::ControlModifier );
  }
  else if ( isResizing() )
  {
    //currently resizing a selection
    //lock aspect ratio if shift depressed
    //resize from center if alt depressed
    resizeMouseMove( event->lastScenePos(), event->modifiers() & Qt::ShiftModifier, event->modifiers() & Qt::AltModifier );
  }

  mLastMouseEventPos = event->lastScenePos();
}

void QgsGraphicsViewMouseHandles::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

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

  if ( mCurrentMouseMoveAction == MoveItem )
  {
    //move selected items
    startMacroCommand( tr( "Move Items" ) );

    QPointF mEndHandleMovePos = scenePos();

    double deltaX = mEndHandleMovePos.x() - mBeginHandlePos.x();
    double deltaY = mEndHandleMovePos.y() - mBeginHandlePos.y();

    //move all selected items
    const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
    for ( QGraphicsItem *item : selectedItems )
    {
      if ( itemIsLocked( item ) || ( item->flags() & QGraphicsItem::ItemIsSelectable ) == 0 || itemIsGroupMember( item ) )
      {
        //don't move locked items, or grouped items (group takes care of that)
        continue;
      }

      createItemCommand( item );
      moveItem( item, deltaX, deltaY );
      endItemCommand( item );
    }
    endMacroCommand();
  }
  else if ( mCurrentMouseMoveAction != NoAction )
  {
    //resize selected items
    startMacroCommand( tr( "Resize Items" ) );

    //resize all selected items
    const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
    for ( QGraphicsItem *item : selectedItems )
    {
      if ( itemIsLocked( item ) || ( item->flags() & QGraphicsItem::ItemIsSelectable ) == 0 )
      {
        //don't resize locked items or deselectable items (e.g., items which make up an item group)
        continue;
      }
      createItemCommand( item );

      QRectF thisItemRect;
      if ( selectedItems.size() == 1 )
      {
        //only a single item is selected, so set its size to the final resized mouse handle size
        thisItemRect = mResizeRect;
      }
      else
      {
        //multiple items selected, so each needs to be scaled relatively to the final size of the mouse handles
        thisItemRect = mapRectFromItem( item, itemRect( item ) );
        relativeResizeRect( thisItemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );
      }

      thisItemRect = thisItemRect.normalized();
      QPointF newPos = mapToScene( thisItemRect.topLeft() );
      thisItemRect.moveTopLeft( newPos );
      setItemRect( item, thisItemRect );

      endItemCommand( item );
    }
    endMacroCommand();
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
  mCurrentMouseMoveAction = MoveItem;
  setViewportCursor( Qt::ArrowCursor );
  //redraw handles
  resetTransform();
  updateHandles();
  //reset status bar message
  resetStatusBar();
}

bool QgsGraphicsViewMouseHandles::selectionRotation( double &rotation ) const
{
  //check if all selected items have same rotation
  QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
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

void QgsGraphicsViewMouseHandles::updateHandles()
{
  //recalculate size and position of handle item

  //first check to see if any items are selected
  QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
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

void QgsGraphicsViewMouseHandles::dragMouseMove( QPointF currentPosition, bool lockMovement, bool preventSnap )
{
  if ( !scene() )
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
    snappedLeftPoint = snapPoint( upperLeftPoint, Item );
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

  const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
  for ( QGraphicsItem *item : selectedItems )
  {
    previewItemMove( item, moveRectX, moveRectY );
  }
  //show current displacement of selection in status bar
  showStatusMessage( tr( "dx: %1 mm dy: %2 mm" ).arg( moveRectX ).arg( moveRectY ) );
}

void QgsGraphicsViewMouseHandles::resizeMouseMove( QPointF currentPosition, bool lockRatio, bool fromCenter )
{
  if ( !scene() )
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
    QPointF snappedPosition = snapPoint( QPointF( currentPosition.x() - mCursorOffset.width(), currentPosition.y() - mCursorOffset.height() ), Point, snapHorizontal, snapVertical );
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
    case ResizeUp:
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

    case ResizeDown:
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
    case ResizeLeft:
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

    case ResizeRight:
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
    case ResizeLeftUp:
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

    case ResizeRightDown:
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

    case ResizeRightUp:
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

    case ResizeLeftDown:
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

    case MoveItem:
    case SelectItem:
    case NoAction:
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

  const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
  QRectF newHandleBounds;
  for ( QGraphicsItem *item : selectedItems )
  {
    //get stored item bounds in mouse handle item's coordinate system
    QRectF thisItemRect = mapRectFromScene( storedItemRect( item ) );
    //now, resize it relative to the current resized dimensions of the mouse handles
    relativeResizeRect( thisItemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );

    thisItemRect = mapRectFromScene( previewSetItemRect( item, mapRectToScene( thisItemRect ) ) );
    newHandleBounds = newHandleBounds.isValid() ? newHandleBounds.united( thisItemRect ) : thisItemRect;
  }

  setRect( newHandleBounds.isValid() ? newHandleBounds : QRectF( 0, 0, std::fabs( mBeginHandleWidth + rx ), std::fabs( mBeginHandleHeight + ry ) ) );

  //show current size of selection in status bar
  showStatusMessage( tr( "width: %1 mm height: %2 mm" ).arg( rect().width() ).arg( rect().height() ) );

}

void QgsGraphicsViewMouseHandles::setHandleSize( double size )
{
  mHandleSize = size;
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

QRectF QgsGraphicsViewMouseHandles::selectionBounds() const
{
  //calculate bounds of all currently selected items in mouse handle coordinate system
  const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
  auto itemIter = selectedItems.constBegin();

  //start with handle bounds of first selected item
  QRectF bounds = mapFromItem( ( *itemIter ), itemRect( *itemIter ) ).boundingRect();

  //iterate through remaining items, expanding the bounds as required
  for ( ++itemIter; itemIter != selectedItems.constEnd(); ++itemIter )
  {
    bounds = bounds.united( mapFromItem( ( *itemIter ), itemRect( *itemIter ) ).boundingRect() );
  }

  return bounds;
}

void QgsGraphicsViewMouseHandles::relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter )
{
  //linearly scale rectToResize relative to the scaling from boundsBefore to boundsAfter
  double left = relativePosition( rectToResize.left(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double right = relativePosition( rectToResize.right(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double top = relativePosition( rectToResize.top(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );
  double bottom = relativePosition( rectToResize.bottom(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );

  rectToResize.setRect( left, top, right - left, bottom - top );
}

double QgsGraphicsViewMouseHandles::relativePosition( double position, double beforeMin, double beforeMax, double afterMin, double afterMax )
{
  //calculate parameters for linear scale between before and after ranges
  double m = ( afterMax - afterMin ) / ( beforeMax - beforeMin );
  double c = afterMin - ( beforeMin * m );

  //return linearly scaled position
  return m * position + c;
}

///@endcond PRIVATE
