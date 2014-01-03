/***************************************************************************
                         qgscomposermousehandles.cpp
                             -------------------
    begin                : September 2013
    copyright            : (C) 2013 by Nyall Dawson, Radim Blazek
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
#include <QPainter>
#include <QWidget>

#include <limits>

#include "qgscomposermousehandles.h"
#include "qgscomposeritem.h"
#include "qgscomposition.h"
#include "qgis.h"
#include "qgslogger.h"

QgsComposerMouseHandles::QgsComposerMouseHandles( QgsComposition *composition ) : QObject( 0 ),
    QGraphicsRectItem( 0 ),
    mComposition( composition ),
    mGraphicsView( 0 ),
    mBeginHandleWidth( 0 ),
    mBeginHandleHeight( 0 ),
    mResizeMoveX( 0 ),
    mResizeMoveY( 0 ),
    mIsDragging( false ),
    mIsResizing( false ),
    mHAlignSnapItem( 0 ),
    mVAlignSnapItem( 0 )
{
  //listen for selection changes, and update handles accordingly
  QObject::connect( mComposition, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );

  //accept hover events, required for changing cursor to resize cursors
  setAcceptsHoverEvents( true );
}

QgsComposerMouseHandles::~QgsComposerMouseHandles()
{

}

QGraphicsView* QgsComposerMouseHandles::graphicsView()
{
  //have we already found the current view?
  if ( mGraphicsView )
  {
    return mGraphicsView;
  }

  //otherwise, try and find current view attached to composition
  if ( scene() )
  {
    QList<QGraphicsView*> viewList = scene()->views();
    if ( viewList.size() > 0 )
    {
      mGraphicsView = viewList.at( 0 );
      return mGraphicsView;
    }
  }

  //no view attached to composition
  return 0;
}

void QgsComposerMouseHandles::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  if ( mComposition->plotStyle() != QgsComposition::Preview )
  {
    //don't draw selection handles in composition outputs
    return;
  }

  //draw resize handles around bounds of entire selection
  double rectHandlerSize = rectHandlerBorderTolerance();
  drawHandles( painter, rectHandlerSize );

  //draw dotted boxes around selected items
  drawSelectedItemBounds( painter );
}

void QgsComposerMouseHandles::drawHandles( QPainter* painter, double rectHandlerSize )
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
  painter->drawRect( QRectF(( rect().width() - rectHandlerSize ) / 2, 0, rectHandlerSize, rectHandlerSize ) );
  //top right
  painter->drawRect( QRectF( rect().width() - rectHandlerSize, 0, rectHandlerSize, rectHandlerSize ) );
  //mid left
  painter->drawRect( QRectF( 0, ( rect().height() - rectHandlerSize ) / 2, rectHandlerSize, rectHandlerSize ) );
  //mid right
  painter->drawRect( QRectF( rect().width() - rectHandlerSize, ( rect().height() - rectHandlerSize ) / 2, rectHandlerSize, rectHandlerSize ) );
  //bottom left
  painter->drawRect( QRectF( 0, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
  //mid bottom
  painter->drawRect( QRectF(( rect().width() - rectHandlerSize ) / 2, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
  //bottom right
  painter->drawRect( QRectF( rect().width() - rectHandlerSize, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
}

void QgsComposerMouseHandles::drawSelectedItemBounds( QPainter* painter )
{
  //draw dotted border around selected items to give visual feedback which items are selected
  QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
  if ( selectedItems.size() == 0 )
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

  QList<QgsComposerItem*>::iterator itemIter = selectedItems.begin();
  for ( ; itemIter != selectedItems.end(); ++itemIter )
  {
    //get bounds of selected item
    QPolygonF itemBounds;
    if ( mIsDragging && !( *itemIter )->positionLock() )
    {
      //if currently dragging, draw selected item bounds relative to current mouse position
      //first, get bounds of current item in scene coordinates
      QPolygonF itemSceneBounds = ( *itemIter )->mapToScene(( *itemIter )->rect() );
      //now, translate it by the current movement amount
      //IMPORTANT - this is done in scene coordinates, since we don't want any rotation/non-translation transforms to affect the movement
      itemSceneBounds.translate( transform().dx(), transform().dy() );
      //finally, remap it to the mouse handle item's coordinate system so it's ready for drawing
      itemBounds = mapFromScene( itemSceneBounds );
    }
    else if ( mIsResizing && !( *itemIter )->positionLock() )
    {
      //if currently resizing, calculate relative resize of this item
      if ( selectedItems.size() > 1 )
      {
        //get item bounds in mouse handle item's coordinate system
        QRectF itemRect = mapRectFromItem(( *itemIter ), ( *itemIter )->rect() );
        //now, resize it relative to the current resized dimensions of the mouse handles
        QgsComposition::relativeResizeRect( itemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );
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
      itemBounds = mapRectFromItem(( *itemIter ), ( *itemIter )->rect() );
    }
    painter->drawPolygon( itemBounds );
  }
  painter->restore();
}

void QgsComposerMouseHandles::selectionChanged()
{
  //listen out for selected items' sizeChanged signal
  QList<QGraphicsItem *> itemList = composition()->items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* item = dynamic_cast<QgsComposerItem *>( *itemIt );
    if ( item )
    {
      if ( item->selected() )
      {
        QObject::connect( item, SIGNAL( sizeChanged() ), this, SLOT( selectedItemSizeChanged() ) );
        QObject::connect( item, SIGNAL( itemRotationChanged( double ) ), this, SLOT( selectedItemRotationChanged() ) );
      }
      else
      {
        QObject::disconnect( item, SIGNAL( sizeChanged() ), this, 0 );
      }
    }
  }

  resetStatusBar();
  updateHandles();
}

void QgsComposerMouseHandles::selectedItemSizeChanged()
{
  if ( !mIsDragging && !mIsResizing )
  {
    //only required for non-mouse initiated size changes
    updateHandles();
  }
}

void QgsComposerMouseHandles::selectedItemRotationChanged()
{
  if ( !mIsDragging && !mIsResizing )
  {
    //only required for non-mouse initiated rotation changes
    updateHandles();
  }
}

void QgsComposerMouseHandles::updateHandles()
{
  //recalculate size and position of handle item

  //first check to see if any items are selected
  QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
  if ( selectedItems.size() > 0 )
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

QRectF QgsComposerMouseHandles::selectionBounds() const
{
  //calculate bounds of all currently selected items in mouse handle coordinate system
  QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIter = selectedItems.begin();

  //start with handle bounds of first selected item
  QRectF bounds = mapFromItem(( *itemIter ), ( *itemIter )->rect() ).boundingRect();

  //iterate through remaining items, expanding the bounds as required
  for ( ++itemIter; itemIter != selectedItems.end(); ++itemIter )
  {
    bounds = bounds.united( mapFromItem(( *itemIter ), ( *itemIter )->rect() ).boundingRect() );
  }

  return bounds;
}

bool QgsComposerMouseHandles::selectionRotation( double & rotation ) const
{
  //check if all selected items have same rotation
  QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIter = selectedItems.begin();

  //start with rotation of first selected item
  double firstItemRotation = ( *itemIter )->itemRotation();

  //iterate through remaining items, checking if they have same rotation
  for ( ++itemIter; itemIter != selectedItems.end(); ++itemIter )
  {
    if (( *itemIter )->itemRotation() != firstItemRotation )
    {
      //item has a different rotation, so return false
      return false;
    }
  }

  //all items have the same rotation, so set the rotation variable and return true
  rotation = firstItemRotation;
  return true;
}

double QgsComposerMouseHandles::rectHandlerBorderTolerance()
{
  //calculate size for resize handles
  //get view scale factor
  double viewScaleFactor = graphicsView()->transform().m11();

  //size of handle boxes depends on zoom level in composer view
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

Qt::CursorShape QgsComposerMouseHandles::cursorForPosition( const QPointF& itemCoordPos )
{
  QgsComposerMouseHandles::MouseAction mouseAction = mouseActionForPosition( itemCoordPos );
  switch ( mouseAction )
  {
    case NoAction:
      return Qt::ForbiddenCursor;
    case MoveItem:
      return Qt::SizeAllCursor;
    case ResizeUp:
    case ResizeDown:
      //account for rotation
      if (( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeVerCursor;
      }
      else if (( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeBDiagCursor;
      }
      else if (( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
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
      if (( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeHorCursor;
      }
      else if (( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeFDiagCursor;
      }
      else if (( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
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
      if (( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeFDiagCursor;
      }
      else if (( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeVerCursor;
      }
      else if (( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
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
      if (( rotation() <= 22.5 || rotation() >= 337.5 ) || ( rotation() >= 157.5 && rotation() <= 202.5 ) )
      {
        return Qt::SizeBDiagCursor;
      }
      else if (( rotation() >= 22.5 && rotation() <= 67.5 ) || ( rotation() >= 202.5 && rotation() <= 247.5 ) )
      {
        return Qt::SizeHorCursor;
      }
      else if (( rotation() >= 67.5 && rotation() <= 112.5 ) || ( rotation() >= 247.5 && rotation() <= 292.5 ) )
      {
        return Qt::SizeFDiagCursor;
      }
      else
      {
        return Qt::SizeVerCursor;
      }
    case SelectItem:
    default:
      return Qt::ArrowCursor;
  }
}

QgsComposerMouseHandles::MouseAction QgsComposerMouseHandles::mouseActionForPosition( const QPointF& itemCoordPos )
{
  bool nearLeftBorder = false;
  bool nearRightBorder = false;
  bool nearLowerBorder = false;
  bool nearUpperBorder = false;

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
    return QgsComposerMouseHandles::ResizeLeftUp;
  }
  else if ( nearLeftBorder && nearLowerBorder )
  {
    return QgsComposerMouseHandles::ResizeLeftDown;
  }
  else if ( nearRightBorder && nearUpperBorder )
  {
    return QgsComposerMouseHandles::ResizeRightUp;
  }
  else if ( nearRightBorder && nearLowerBorder )
  {
    return QgsComposerMouseHandles::ResizeRightDown;
  }
  else if ( nearLeftBorder )
  {
    return QgsComposerMouseHandles::ResizeLeft;
  }
  else if ( nearRightBorder )
  {
    return QgsComposerMouseHandles::ResizeRight;
  }
  else if ( nearUpperBorder )
  {
    return QgsComposerMouseHandles::ResizeUp;
  }
  else if ( nearLowerBorder )
  {
    return QgsComposerMouseHandles::ResizeDown;
  }

  //find out if cursor position is over a selected item
  QPointF scenePoint = mapToScene( itemCoordPos );
  QList<QGraphicsItem *> itemsAtCursorPos = mComposition->items( scenePoint );
  if ( itemsAtCursorPos.size() == 0 )
  {
    //no items at cursor position
    return QgsComposerMouseHandles::SelectItem;
  }
  QList<QGraphicsItem*>::iterator itemIter = itemsAtCursorPos.begin();
  for ( ; itemIter != itemsAtCursorPos.end(); ++itemIter )
  {
    QgsComposerItem* item = dynamic_cast<QgsComposerItem *>(( *itemIter ) );
    if ( item && item->selected() )
    {
      //cursor is over a selected composer item
      return QgsComposerMouseHandles::MoveItem;
    }
  }

  //default
  return QgsComposerMouseHandles::SelectItem;
}

QgsComposerMouseHandles::MouseAction QgsComposerMouseHandles::mouseActionForScenePos( const QPointF& sceneCoordPos )
{
  // convert sceneCoordPos to item coordinates
  QPointF itemPos = mapFromScene( sceneCoordPos );
  return mouseActionForPosition( itemPos );
}

void QgsComposerMouseHandles::hoverMoveEvent( QGraphicsSceneHoverEvent * event )
{
  setViewportCursor( cursorForPosition( event->pos() ) );
}

void QgsComposerMouseHandles::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED( event );
  setViewportCursor( Qt::ArrowCursor );
}

void QgsComposerMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  //workaround qt bug #3732 by setting cursor for QGraphicsView viewport,
  //rather then setting it directly here

  if ( !mComposition->preventCursorChange() )
  {
    graphicsView()->viewport()->setCursor( cursor );
  }
}

void QgsComposerMouseHandles::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
  bool shiftModifier = false;
  bool controlModifier = false;
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //shift key depressed
    shiftModifier = true;
  }
  if ( event->modifiers() & Qt::ControlModifier )
  {
    //shift key depressed
    controlModifier = true;
  }

  if ( mIsDragging )
  {
    //currently dragging a selection
    dragMouseMove( event->lastScenePos(), shiftModifier, controlModifier );
  }
  else if ( mIsResizing )
  {
    //currently resizing a selection
    resizeMouseMove( event->lastScenePos(), shiftModifier, controlModifier );
  }

  mLastMouseEventPos = event->lastScenePos();
}

void QgsComposerMouseHandles::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
  QPointF mouseMoveStopPoint = event->lastScenePos();
  double diffX = mouseMoveStopPoint.x() - mMouseMoveStartPos.x();
  double diffY = mouseMoveStopPoint.y() - mMouseMoveStartPos.y();

  //it was only a click
  if ( qAbs( diffX ) < std::numeric_limits<double>::min() && qAbs( diffY ) < std::numeric_limits<double>::min() )
  {
    mIsDragging = false;
    mIsResizing = false;
    return;
  }

  if ( mCurrentMouseMoveAction == QgsComposerMouseHandles::MoveItem )
  {
    //move selected items
    QUndoCommand* parentCommand = new QUndoCommand( tr( "Change item position" ) );

    QPointF mEndHandleMovePos = scenePos();

    //move all selected items
    QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
    QList<QgsComposerItem*>::iterator itemIter = selectedItems.begin();
    for ( ; itemIter != selectedItems.end(); ++itemIter )
    {
      if (( *itemIter )->positionLock() || (( *itemIter )->flags() & QGraphicsItem::ItemIsSelectable ) == 0 )
      {
        //don't move locked items
        continue;
      }
      QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *itemIter, "", parentCommand );
      subcommand->savePreviousState();
      ( *itemIter )->move( mEndHandleMovePos.x() - mBeginHandlePos.x(), mEndHandleMovePos.y() - mBeginHandlePos.y() );
      subcommand->saveAfterState();
    }
    mComposition->undoStack()->push( parentCommand );

  }
  else if ( mCurrentMouseMoveAction != QgsComposerMouseHandles::NoAction )
  {
    //resize selected items
    QUndoCommand* parentCommand = new QUndoCommand( tr( "Change item size" ) );

    //resize all selected items
    QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
    QList<QgsComposerItem*>::iterator itemIter = selectedItems.begin();
    for ( ; itemIter != selectedItems.end(); ++itemIter )
    {
      if (( *itemIter )->positionLock() || (( *itemIter )->flags() & QGraphicsItem::ItemIsSelectable ) == 0 )
      {
        //don't resize locked items or unselectable items (eg, items which make up an item group)
        continue;
      }
      QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *itemIter, "", parentCommand );
      subcommand->savePreviousState();

      QRectF itemRect;
      if ( selectedItems.size() == 1 )
      {
        //only a single item is selected, so set it's size to the final resized mouse handle size
        itemRect = mResizeRect;
      }
      else
      {
        //multiple items selected, so each needs to be scaled relatively to the final size of the mouse handles
        itemRect = mapRectFromItem(( *itemIter ), ( *itemIter )->rect() );
        QgsComposition::relativeResizeRect( itemRect, QRectF( -mResizeMoveX, -mResizeMoveY, mBeginHandleWidth, mBeginHandleHeight ), mResizeRect );
      }

      itemRect = itemRect.normalized();
      QPointF newPos = mapToScene( itemRect.topLeft() );
      ( *itemIter )->setItemPosition( newPos.x(), newPos.y(), itemRect.width(), itemRect.height() );

      subcommand->saveAfterState();
    }
    mComposition->undoStack()->push( parentCommand );
  }

  deleteAlignItems();

  if ( mIsDragging )
  {
    mIsDragging = false;
  }
  if ( mIsResizing )
  {
    mIsResizing = false;
  }

  //reset default action
  mCurrentMouseMoveAction = QgsComposerMouseHandles::MoveItem;
  setViewportCursor( Qt::ArrowCursor );
  //redraw handles
  resetTransform();
  updateHandles();
  //reset status bar message
  resetStatusBar();
}

void QgsComposerMouseHandles::resetStatusBar()
{
  QList<QgsComposerItem*> selectedItems = mComposition->selectedComposerItems();
  int selectedCount = selectedItems.size();
  if ( selectedCount > 1 )
  {
    //set status bar message to count of selected items
    mComposition->setStatusMessage( QString( tr( "%1 items selected" ) ).arg( selectedCount ) );
  }
  else if ( selectedCount == 1 )
  {
    //set status bar message to count of selected items
    mComposition->setStatusMessage( tr( "1 item selected" ) );
  }
  else
  {
    //clear status bar message
    mComposition->setStatusMessage( QString( "" ) );
  }
}

void QgsComposerMouseHandles::mousePressEvent( QGraphicsSceneMouseEvent* event )
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

  deleteAlignItems();

  if ( mCurrentMouseMoveAction == QgsComposerMouseHandles::MoveItem )
  {
    mIsDragging = true;
  }
  else if ( mCurrentMouseMoveAction != QgsComposerMouseHandles::SelectItem &&
            mCurrentMouseMoveAction != QgsComposerMouseHandles::NoAction )
  {
    mIsResizing = true;
    mResizeRect = QRectF( 0, 0, mBeginHandleWidth, mBeginHandleHeight );
    mResizeMoveX = 0;
    mResizeMoveY = 0;
  }

}

void QgsComposerMouseHandles::dragMouseMove( const QPointF& currentPosition, bool lockMovement, bool preventSnap )
{
  if ( !mComposition )
  {
    return;
  }

  //calculate total amount of mouse movement since drag began
  double moveX = currentPosition.x() - mBeginMouseEventPos.x();
  double moveY = currentPosition.y() - mBeginMouseEventPos.y();

  //find target position before snapping (in scene coordinates)
  QPointF upperLeftPoint( mBeginHandlePos.x() + moveX, mBeginHandlePos.y() + moveY );

  QPointF snappedLeftPoint;
  if ( !preventSnap )
  {
    //snap to grid and guides
    snappedLeftPoint = snapPoint( upperLeftPoint, QgsComposerMouseHandles::Item );
  }
  else
  {
    //no snapping
    snappedLeftPoint = upperLeftPoint;
    deleteAlignItems();
  }

  //calculate total shift for item from beginning of drag operation to current position
  double moveRectX = snappedLeftPoint.x() - mBeginHandlePos.x();
  double moveRectY = snappedLeftPoint.y() - mBeginHandlePos.y();

  if ( lockMovement )
  {
    //constrained (shift) moving should lock to horizontal/vertical movement
    //reset the smaller of the x/y movements
    if ( abs( moveRectX ) <= abs( moveRectY ) )
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
  mComposition->setStatusMessage( QString( tr( "dx: %1 mm dy: %2 mm" ) ).arg( moveRectX ).arg( moveRectY ) );
}

void QgsComposerMouseHandles::resizeMouseMove( const QPointF& currentPosition, bool lockRatio, bool fromCenter )
{

  if ( !mComposition )
  {
    return;
  }

  QPointF beginMousePos = mapFromScene( mBeginMouseEventPos );

  double mx = 0.0, my = 0.0, rx = 0.0, ry = 0.0;
  QPointF snappedPosition = snapPoint( currentPosition, QgsComposerMouseHandles::Point );

  //QPointF currentPosHandle = mapFromScene( currentPosition );
  snappedPosition = mapFromScene( snappedPosition );
  double diffX = 0;
  double diffY = 0;

  diffX = snappedPosition.x() - beginMousePos.x();
  diffY = snappedPosition.y() - beginMousePos.y();

  double ratio = 0;
  if ( lockRatio && mBeginHandleHeight != 0 )
  {
    ratio = mBeginHandleWidth / mBeginHandleHeight;
  }

  switch ( mCurrentMouseMoveAction )
  {
      //vertical resize
    case QgsComposerMouseHandles::ResizeUp:
      //diffY = snappedPosition.y() - beginMousePos.y();
      if ( ratio )
      {
        diffX = (( mBeginHandleHeight - diffY ) * ratio ) - mBeginHandleWidth;
        mx = -diffX / 2; my = diffY; rx = diffX; ry = -diffY;
      }
      else
      {
        mx = 0; my = diffY; rx = 0; ry = -diffY;
      }
      break;

    case QgsComposerMouseHandles::ResizeDown:
      //diffY = snappedPosition.y() - beginMousePos.y();
      if ( ratio )
      {
        diffX = (( mBeginHandleHeight + diffY ) * ratio ) - mBeginHandleWidth;
        mx = -diffX / 2; my = 0; rx = diffX; ry = diffY;
      }
      else
      {
        mx = 0; my = 0; rx = 0; ry = diffY;
      }
      break;

      //horizontal resize
    case QgsComposerMouseHandles::ResizeLeft:
      //diffX = snappedPosition.x() - beginMousePos.x();
      if ( ratio )
      {
        diffY = (( mBeginHandleWidth - diffX ) / ratio ) - mBeginHandleHeight;
        mx = diffX; my = -diffY / 2; rx = -diffX; ry = diffY;
      }
      else
      {
        mx = diffX, my = 0; rx = -diffX; ry = 0;
      }
      break;

    case QgsComposerMouseHandles::ResizeRight:
      //diffX = snappedPosition.x() - ( beginMousePos.x() + mBeginHandleWidth );
      if ( ratio )
      {
        diffY = (( mBeginHandleWidth + diffX ) / ratio ) - mBeginHandleHeight;
        mx = 0; my = -diffY / 2; rx = diffX; ry = diffY;
      }
      else
      {
        mx = 0; my = 0; rx = diffX, ry = 0;
      }
      break;

      //diagonal resize
    case QgsComposerMouseHandles::ResizeLeftUp:
      //diffX = snappedPosition.x() - beginMousePos.x();
      diffY = snappedPosition.y() - beginMousePos.y();
      if ( ratio )
      {
        //ratio locked resize
        if (( mBeginHandleWidth - diffX ) / ( mBeginHandleHeight - diffY ) > ratio )
        {
          diffX = mBeginHandleWidth - (( mBeginHandleHeight - diffY ) * ratio );
        }
        else
        {
          diffY = mBeginHandleHeight - (( mBeginHandleWidth - diffX ) / ratio );
        }
      }
      mx = diffX, my = diffY; rx = -diffX; ry = -diffY;
      break;

    case QgsComposerMouseHandles::ResizeRightDown:
      //diffX = snappedPosition.x() - ( beginMousePos.x() + mBeginHandleWidth );
      //diffY = snappedPosition.y() - ( beginMousePos.y() + mBeginHandleHeight );
      if ( ratio )
      {
        //ratio locked resize
        if (( mBeginHandleWidth + diffX ) / ( mBeginHandleHeight + diffY ) > ratio )
        {
          diffX = (( mBeginHandleHeight + diffY ) * ratio ) - mBeginHandleWidth;
        }
        else
        {
          diffY = (( mBeginHandleWidth + diffX ) / ratio ) - mBeginHandleHeight;
        }
      }
      mx = 0; my = 0; rx = diffX, ry = diffY;
      break;

    case QgsComposerMouseHandles::ResizeRightUp:
      //diffX = snappedPosition.x() - ( beginMousePos.x() + mBeginHandleWidth );
      //diffY = snappedPosition.y() - beginMousePos.y();
      if ( ratio )
      {
        //ratio locked resize
        if (( mBeginHandleWidth + diffX ) / ( mBeginHandleHeight - diffY ) > ratio )
        {
          diffX = (( mBeginHandleHeight - diffY ) * ratio ) - mBeginHandleWidth;
        }
        else
        {
          diffY = mBeginHandleHeight - (( mBeginHandleWidth + diffX ) / ratio );
        }
      }
      mx = 0; my = diffY, rx = diffX, ry = -diffY;
      break;

    case QgsComposerMouseHandles::ResizeLeftDown:
      //diffX = snappedPosition.x() - beginMousePos.x();
      //diffY = snappedPosition.y() - ( beginMousePos.y() + mBeginHandleHeight );
      if ( ratio )
      {
        //ratio locked resize
        if (( mBeginHandleWidth - diffX ) / ( mBeginHandleHeight + diffY ) > ratio )
        {
          diffX = mBeginHandleWidth - (( mBeginHandleHeight + diffY ) * ratio );
        }
        else
        {
          diffY = (( mBeginHandleWidth - diffX ) / ratio ) - mBeginHandleHeight;
        }
      }
      mx = diffX, my = 0; rx = -diffX; ry = diffY;
      break;

    case QgsComposerMouseHandles::MoveItem:
    case QgsComposerMouseHandles::SelectItem:
    case QgsComposerMouseHandles::NoAction:
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

  //handle non-normalised resizes - eg, dragging the left handle so far to the right that it's past the right handle
  if ( mBeginHandleWidth + rx >= 0 && mBeginHandleHeight + ry >= 0 )
  {
    mResizeRect = QRectF( 0, 0,  mBeginHandleWidth + rx, mBeginHandleHeight + ry );
  }
  else if ( mBeginHandleHeight + ry >= 0 )
  {
    mResizeRect = QRectF( QPointF( -( mBeginHandleWidth + rx ), 0 ),  QPointF( 0, mBeginHandleHeight + ry ) );
  }
  else if ( mBeginHandleWidth + rx >= 0 )
  {
    mResizeRect = QRectF( QPointF( 0, -( mBeginHandleHeight + ry ) ),  QPointF( mBeginHandleWidth + rx, 0 ) );
  }
  else
  {
    mResizeRect = QRectF( QPointF( -( mBeginHandleWidth + rx ), -( mBeginHandleHeight + ry ) ),  QPointF( 0, 0 ) );
  }

  setRect( 0, 0, fabs( mBeginHandleWidth + rx ), fabs( mBeginHandleHeight + ry ) );

  //show current size of selection in status bar
  mComposition->setStatusMessage( QString( tr( "width: %1 mm height: %2 mm" ) ).arg( rect().width() ).arg( rect().height() ) );
}

QPointF QgsComposerMouseHandles::snapPoint( const QPointF& point, QgsComposerMouseHandles::SnapGuideMode mode )
{
  //snap to grid
  QPointF snappedPoint = mComposition->snapPointToGrid( point );

  if ( snappedPoint != point ) //don't do align snap if grid snap has been done
  {
    deleteAlignItems();
    return snappedPoint;
  }

  //align item
  if ( !mComposition->alignmentSnap() && !mComposition->smartGuidesEnabled() )
  {
    return point;
  }

  double alignX = 0;
  double alignY = 0;

  //depending on the mode, we either snap just the single point, or all the bounds of the selection
  switch ( mode )
  {
    case QgsComposerMouseHandles::Item:
      snappedPoint = alignItem( alignX, alignY, point.x(), point.y() );
      break;
    case QgsComposerMouseHandles::Point:
      snappedPoint = alignPos( point, alignX, alignY );
      break;
  }

  if ( alignX != -1 )
  {
    QGraphicsLineItem* item = hAlignSnapItem();
    int numPages = mComposition->numPages();
    double yLineCoord = 300; //default in case there is no single page
    if ( numPages > 0 )
    {
      yLineCoord = mComposition->paperHeight() * numPages + mComposition->spaceBetweenPages() * ( numPages - 1 );
    }
    item->setLine( QLineF( alignX, 0, alignX,  yLineCoord ) );
    item->show();
  }
  else
  {
    deleteHAlignSnapItem();
  }
  if ( alignY != -1 )
  {
    QGraphicsLineItem* item = vAlignSnapItem();
    item->setLine( QLineF( 0, alignY, mComposition->paperWidth(), alignY ) );
    item->show();
  }
  else
  {
    deleteVAlignSnapItem();
  }
  return snappedPoint;
}

QGraphicsLineItem* QgsComposerMouseHandles::hAlignSnapItem()
{
  if ( !mHAlignSnapItem )
  {
    mHAlignSnapItem = new QGraphicsLineItem( 0 );
    mHAlignSnapItem->setPen( QPen( QColor( Qt::red ) ) );
    scene()->addItem( mHAlignSnapItem );
    mHAlignSnapItem->setZValue( 90 );
  }
  return mHAlignSnapItem;
}

QGraphicsLineItem* QgsComposerMouseHandles::vAlignSnapItem()
{
  if ( !mVAlignSnapItem )
  {
    mVAlignSnapItem = new QGraphicsLineItem( 0 );
    mVAlignSnapItem->setPen( QPen( QColor( Qt::red ) ) );
    scene()->addItem( mVAlignSnapItem );
    mVAlignSnapItem->setZValue( 90 );
  }
  return mVAlignSnapItem;
}

void QgsComposerMouseHandles::deleteHAlignSnapItem()
{
  if ( mHAlignSnapItem )
  {
    scene()->removeItem( mHAlignSnapItem );
    delete mHAlignSnapItem;
    mHAlignSnapItem = 0;
  }
}

void QgsComposerMouseHandles::deleteVAlignSnapItem()
{
  if ( mVAlignSnapItem )
  {
    scene()->removeItem( mVAlignSnapItem );
    delete mVAlignSnapItem;
    mVAlignSnapItem = 0;
  }
}

void QgsComposerMouseHandles::deleteAlignItems()
{
  deleteHAlignSnapItem();
  deleteVAlignSnapItem();
}

QPointF QgsComposerMouseHandles::alignItem( double& alignX, double& alignY, double unalignedX, double unalignedY )
{
  double left = unalignedX;
  double right = left + rect().width();
  double midH = ( left + right ) / 2.0;
  double top = unalignedY;
  double bottom = top + rect().height();
  double midV = ( top + bottom ) / 2.0;

  QMap<double, const QgsComposerItem* > xAlignCoordinates;
  QMap<double, const QgsComposerItem* > yAlignCoordinates;
  collectAlignCoordinates( xAlignCoordinates, yAlignCoordinates );

  //find nearest matches x
  double xItemLeft = left; //new left coordinate of the item
  double xAlignCoord = 0;
  double smallestDiffX = DBL_MAX;

  checkNearestItem( left, xAlignCoordinates, smallestDiffX, 0, xItemLeft, xAlignCoord );
  checkNearestItem( midH, xAlignCoordinates, smallestDiffX, ( left - right ) / 2.0, xItemLeft, xAlignCoord );
  checkNearestItem( right, xAlignCoordinates, smallestDiffX, left - right, xItemLeft, xAlignCoord );

  //find nearest matches y
  double yItemTop = top; //new top coordinate of the item
  double yAlignCoord = 0;
  double smallestDiffY = DBL_MAX;

  checkNearestItem( top, yAlignCoordinates, smallestDiffY, 0, yItemTop, yAlignCoord );
  checkNearestItem( midV, yAlignCoordinates, smallestDiffY, ( top - bottom ) / 2.0, yItemTop, yAlignCoord );
  checkNearestItem( bottom, yAlignCoordinates, smallestDiffY, top - bottom, yItemTop, yAlignCoord );

  double xCoord = ( smallestDiffX < 5 ) ? xItemLeft : unalignedX;
  alignX = ( smallestDiffX < 5 ) ? xAlignCoord : -1;
  double yCoord = ( smallestDiffY < 5 ) ? yItemTop : unalignedY;
  alignY = ( smallestDiffY < 5 ) ? yAlignCoord : -1;
  return QPointF( xCoord, yCoord );
}

QPointF QgsComposerMouseHandles::alignPos( const QPointF& pos, double& alignX, double& alignY )
{
  QMap<double, const QgsComposerItem* > xAlignCoordinates;
  QMap<double, const QgsComposerItem* > yAlignCoordinates;
  collectAlignCoordinates( xAlignCoordinates, yAlignCoordinates );

  double nearestX = pos.x();
  double nearestY = pos.y();
  if ( !nearestItem( xAlignCoordinates, pos.x(), nearestX )
       || !nearestItem( yAlignCoordinates, pos.y(), nearestY ) )
  {
    alignX = -1;
    alignY = -1;
    return pos;
  }

  QPointF result( pos.x(), pos.y() );
  if ( abs( nearestX - pos.x() ) < mComposition->alignmentSnapTolerance() )
  {
    result.setX( nearestX );
    alignX = nearestX;
  }
  else
  {
    alignX = -1;
  }

  if ( abs( nearestY - pos.y() ) < mComposition->alignmentSnapTolerance() )
  {
    result.setY( nearestY );
    alignY = nearestY;
  }
  else
  {
    alignY = -1;
  }
  return result;
}

void QgsComposerMouseHandles::collectAlignCoordinates( QMap< double, const QgsComposerItem* >& alignCoordsX, QMap< double, const QgsComposerItem* >& alignCoordsY )
{
  alignCoordsX.clear();
  alignCoordsY.clear();

  if ( mComposition->smartGuidesEnabled() )
  {
    QList<QGraphicsItem *> itemList = mComposition->items();
    QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
    for ( ; itemIt != itemList.end(); ++itemIt )
    {
      const QgsComposerItem* currentItem = dynamic_cast<const QgsComposerItem *>( *itemIt );
      //don't snap to selected items, since they're the ones that will be snapping to something else
      if ( !currentItem || currentItem->selected() )
      {
        continue;
      }
      QRectF itemRect = currentItem->sceneBoundingRect();
      alignCoordsX.insert( itemRect.left(), currentItem );
      alignCoordsX.insert( itemRect.right(), currentItem );
      alignCoordsX.insert( itemRect.center().x(), currentItem );
      alignCoordsY.insert( itemRect.top(), currentItem );
      alignCoordsY.insert( itemRect.center().y(), currentItem );
      alignCoordsY.insert( itemRect.bottom(), currentItem );
    }
  }

  //arbitrary snap lines
  if ( mComposition->alignmentSnap() )
  {
    QList< QGraphicsLineItem* >::const_iterator sIt = mComposition->snapLines()->constBegin();
    for ( ; sIt != mComposition->snapLines()->constEnd(); ++sIt )
    {
      double x = ( *sIt )->line().x1();
      double y = ( *sIt )->line().y1();
      if ( qgsDoubleNear( y, 0.0 ) )
      {
        alignCoordsX.insert( x, 0 );
      }
      else
      {
        alignCoordsY.insert( y, 0 );
      }
    }
  }
}

void QgsComposerMouseHandles::checkNearestItem( double checkCoord, const QMap< double, const QgsComposerItem* >& alignCoords, double& smallestDiff, double itemCoordOffset, double& itemCoord, double& alignCoord ) const
{
  double currentCoord = 0;
  if ( !nearestItem( alignCoords, checkCoord, currentCoord ) )
  {
    return;
  }

  double currentDiff = abs( checkCoord - currentCoord );
  if ( currentDiff < mComposition->alignmentSnapTolerance() )
  {
    itemCoord = currentCoord + itemCoordOffset;
    alignCoord = currentCoord;
    smallestDiff = currentDiff;
  }
}

bool QgsComposerMouseHandles::nearestItem( const QMap< double, const QgsComposerItem* >& coords, double value, double& nearestValue ) const
{
  if ( coords.size() < 1 )
  {
    return false;
  }

  QMap< double, const QgsComposerItem* >::const_iterator it = coords.lowerBound( value );
  if ( it == coords.constBegin() ) //value smaller than first map value
  {
    nearestValue = it.key();
    return true;
  }
  else if ( it == coords.constEnd() ) //value larger than last map value
  {
    --it;
    nearestValue = it.key();
    return true;
  }
  else
  {
    //get smaller value and larger value and return the closer one
    double upperVal = it.key();
    --it;
    double lowerVal = it.key();

    double lowerDiff = value - lowerVal;
    double upperDiff = upperVal - value;
    if ( lowerDiff < upperDiff )
    {
      nearestValue = lowerVal;
      return true;
    }
    else
    {
      nearestValue = upperVal;
      return true;
    }
  }
}
