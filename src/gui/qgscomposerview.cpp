/***************************************************************************
                         qgscomposerview.cpp
                         -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QMainWindow>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>
#include <QGridLayout>
#include <QScrollBar>
#include <QDesktopWidget>

#include "qgsapplication.h"
#include "qgscomposerview.h"
#include "qgscomposerarrow.h"
#include "qgscomposerframe.h"
#include "qgscomposerhtml.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposermousehandles.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposerpicture.h"
#include "qgscomposerruler.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgscomposerattributetablev2.h"
#include "qgslogger.h"
#include "qgsaddremovemultiframecommand.h"
#include "qgspaperitem.h"
#include "qgsmapcanvas.h" //for QgsMapCanvas::WheelAction
#include "qgscursors.h"
#include "qgscomposerutils.h"

QgsComposerView::QgsComposerView( QWidget* parent, const char* name, Qt::WindowFlags f )
    : QGraphicsView( parent )
    , mRubberBandItem( 0 )
    , mRubberBandLineItem( 0 )
    , mMoveContentItem( 0 )
    , mMarqueeSelect( false )
    , mMarqueeZoom( false )
    , mTemporaryZoomStatus( QgsComposerView::Inactive )
    , mPaintingEnabled( true )
    , mHorizontalRuler( 0 )
    , mVerticalRuler( 0 )
    , mToolPanning( false )
    , mMousePanning( false )
    , mKeyPanning( false )
    , mMovingItemContent( false )
    , mPreviewEffect( 0 )
{
  Q_UNUSED( f );
  Q_UNUSED( name );

  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );
  setFrameShape( QFrame::NoFrame );

  mPreviewEffect = new QgsPreviewEffect( this );
  viewport()->setGraphicsEffect( mPreviewEffect );
}

void QgsComposerView::setCurrentTool( QgsComposerView::Tool t )
{
  mCurrentTool = t;

  //update mouse cursor for current tool
  if ( !composition() )
  {
    return;
  }
  switch ( t )
  {
    case QgsComposerView::Pan:
    {
      //lock cursor to prevent composer items changing it
      composition()->setPreventCursorChange( true );
      viewport()->setCursor( defaultCursorForTool( Pan ) );
      break;
    }
    case QgsComposerView::Zoom:
    {
      //lock cursor to prevent composer items changing it
      composition()->setPreventCursorChange( true );
      //set the cursor to zoom in
      viewport()->setCursor( defaultCursorForTool( Zoom ) );
      break;
    }
    case QgsComposerView::AddArrow:
    case QgsComposerView::AddHtml:
    case QgsComposerView::AddMap:
    case QgsComposerView::AddLegend:
    case QgsComposerView::AddLabel:
    case QgsComposerView::AddScalebar:
    case QgsComposerView::AddPicture:
    case QgsComposerView::AddRectangle:
    case QgsComposerView::AddEllipse:
    case QgsComposerView::AddTriangle:
    case QgsComposerView::AddTable:
    case QgsComposerView::AddAttributeTable:
    {
      //using a drawing tool
      //lock cursor to prevent composer items changing it
      composition()->setPreventCursorChange( true );
      viewport()->setCursor( defaultCursorForTool( mCurrentTool ) );
      break;
    }
    default:
    {
      //not using pan tool, composer items can change cursor
      composition()->setPreventCursorChange( false );
      viewport()->setCursor( Qt::ArrowCursor );
    }
  }
}

void QgsComposerView::mousePressEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  if ( mRubberBandItem || mRubberBandLineItem || mKeyPanning || mMousePanning || mToolPanning || mMovingItemContent )
  {
    //ignore clicks during certain operations
    return;
  }

  if ( composition()->selectionHandles()->isDragging() || composition()->selectionHandles()->isResizing() )
  {
    //ignore clicks while dragging/resizing items
    return;
  }

  QPointF scenePoint = mapToScene( e->pos() );
  QPointF snappedScenePoint = composition()->snapPointToGrid( scenePoint );
  mMousePressStartPos = e->pos();

  if ( e->button() == Qt::RightButton )
  {
    //ignore right clicks for now
    //TODO - show context menu
    return;
  }
  else if ( e->button() == Qt::MidButton )
  {
    //pan composer with middle button
    mMousePanning = true;
    mMouseLastXY = e->pos();
    if ( composition() )
    {
      //lock cursor to closed hand cursor
      composition()->setPreventCursorChange( true );
    }
    viewport()->setCursor( Qt::ClosedHandCursor );
    return;
  }

  switch ( mCurrentTool )
  {
      //select/deselect items and pass mouse event further
    case Select:
    {
      //check if we are clicking on a selection handle
      if ( composition()->selectionHandles()->isVisible() )
      {
        //selection handles are being shown, get mouse action for current cursor position
        QgsComposerMouseHandles::MouseAction mouseAction = composition()->selectionHandles()->mouseActionForScenePos( scenePoint );

        if ( mouseAction != QgsComposerMouseHandles::MoveItem && mouseAction != QgsComposerMouseHandles::NoAction && mouseAction != QgsComposerMouseHandles::SelectItem )
        {
          //mouse is over a resize handle, so propagate event onward
          QGraphicsView::mousePressEvent( e );
          return;
        }
      }

      QgsComposerItem* selectedItem = 0;
      QgsComposerItem* previousSelectedItem = 0;

      if ( e->modifiers() & Qt::ControlModifier )
      {
        //CTRL modifier, so we are trying to select the next item below the current one
        //first, find currently selected item
        QList<QgsComposerItem*> selectedItems = composition()->selectedComposerItems();
        if ( selectedItems.size() > 0 )
        {
          previousSelectedItem = selectedItems.at( 0 );
        }
      }

      if ( previousSelectedItem )
      {
        //select highest item just below previously selected item at position of event
        selectedItem = composition()->composerItemAt( scenePoint, previousSelectedItem, true );

        //if we didn't find a lower item we'll use the top-most as fall-back
        //this duplicates mapinfo/illustrator/etc behaviour where ctrl-clicks are "cyclic"
        if ( !selectedItem )
        {
          selectedItem = composition()->composerItemAt( scenePoint, true );
        }
      }
      else
      {
        //select topmost item at position of event
        selectedItem = composition()->composerItemAt( scenePoint, true );
      }

      if ( !selectedItem )
      {
        //not clicking over an item, so start marquee selection
        startMarqueeSelect( scenePoint );
        break;
      }

      if (( !selectedItem->selected() ) &&        //keep selection if an already selected item pressed
          !( e->modifiers() & Qt::ShiftModifier ) ) //keep selection if shift key pressed
      {
        composition()->setAllUnselected();
      }

      if (( e->modifiers() & Qt::ShiftModifier ) && ( selectedItem->selected() ) )
      {
        //SHIFT-clicking a selected item deselects it
        selectedItem->setSelected( false );

        //Check if we have any remaining selected items, and if so, update the item panel
        QList<QgsComposerItem*> selectedItems = composition()->selectedComposerItems();
        if ( selectedItems.size() > 0 )
        {
          emit selectedItemChanged( selectedItems.at( 0 ) );
        }
      }
      else
      {
        selectedItem->setSelected( true );
        QGraphicsView::mousePressEvent( e );
        emit selectedItemChanged( selectedItem );
      }
      break;
    }

    case Zoom:
    {
      if ( !( e->modifiers() & Qt::ShiftModifier ) )
      {
        //zoom in action
        startMarqueeZoom( scenePoint );
      }
      else
      {
        //zoom out action, so zoom out and recenter on clicked point
        double scaleFactor = 2;
        //get current visible part of scene
        QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
        QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

        //transform the mouse pos to scene coordinates
        QPointF scenePoint = mapToScene( e->pos() );

        visibleRect.scale( scaleFactor, scenePoint.x(), scenePoint.y() );
        QRectF boundsRect = visibleRect.toRectF();

        //zoom view to fit desired bounds
        fitInView( boundsRect, Qt::KeepAspectRatio );
      }
      break;
    }

    case Pan:
    {
      //pan action
      mToolPanning = true;
      mMouseLastXY = e->pos();
      viewport()->setCursor( Qt::ClosedHandCursor );
      break;
    }

    case MoveItemContent:
    {
      //get a list of items at clicked position
      QList<QGraphicsItem *> itemsAtCursorPos = items( e->pos() );
      if ( itemsAtCursorPos.size() == 0 )
      {
        //no items at clicked position
        return;
      }

      //find highest non-locked QgsComposerItem at clicked position
      //(other graphics items may be higher, eg selection handles)
      QList<QGraphicsItem*>::iterator itemIter = itemsAtCursorPos.begin();
      for ( ; itemIter != itemsAtCursorPos.end(); ++itemIter )
      {
        QgsComposerItem* item = dynamic_cast<QgsComposerItem *>(( *itemIter ) );
        if ( item && !item->positionLock() )
        {
          //we've found the highest QgsComposerItem
          mMoveContentStartPos = scenePoint;
          mMoveContentItem = item;
          mMovingItemContent = true;
          break;
        }
      }

      //no QgsComposerItem at clicked position
      return;
    }

    //create rubber band for adding line items
    case AddArrow:
    {
      mRubberBandStartPos = QPointF( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandLineItem = new QGraphicsLineItem( snappedScenePoint.x(), snappedScenePoint.y(), snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandLineItem->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
      mRubberBandLineItem->setZValue( 1000 );
      scene()->addItem( mRubberBandLineItem );
      scene()->update();
      break;
    }

    //create rubber band for adding rectangular items
    case AddMap:
    case AddRectangle:
    case AddTriangle:
    case AddEllipse:
    case AddHtml:
    case AddPicture:
    case AddLabel:
    case AddLegend:
    case AddTable:
    case AddAttributeTable:
    {
      QTransform t;
      mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
      mRubberBandItem->setBrush( Qt::NoBrush );
      mRubberBandItem->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
      mRubberBandStartPos = QPointF( snappedScenePoint.x(), snappedScenePoint.y() );
      t.translate( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandItem->setTransform( t );
      mRubberBandItem->setZValue( 1000 );
      scene()->addItem( mRubberBandItem );
      scene()->update();
    }
    break;

    case AddScalebar:
      if ( composition() )
      {
        QgsComposerScaleBar* newScaleBar = new QgsComposerScaleBar( composition() );
        newScaleBar->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 20, 20 ) );
        composition()->addComposerScaleBar( newScaleBar );
        QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
        if ( mapItemList.size() > 0 )
        {
          newScaleBar->setComposerMap( mapItemList.at( 0 ) );
        }
        newScaleBar->applyDefaultSize(); //4 segments, 1/5 of composer map width

        composition()->setAllUnselected();
        newScaleBar->setSelected( true );
        emit selectedItemChanged( newScaleBar );

        emit actionFinished();
        composition()->pushAddRemoveCommand( newScaleBar, tr( "Scale bar added" ) );
      }
      break;

    default:
      break;
  }
}

QCursor QgsComposerView::defaultCursorForTool( Tool currentTool )
{
  switch ( currentTool )
  {
    case Select:
      return Qt::ArrowCursor;

    case Zoom:
    {
      QPixmap myZoomQPixmap = QPixmap(( const char ** )( zoom_in ) );
      return  QCursor( myZoomQPixmap, 7, 7 );
    }

    case Pan:
      return Qt::OpenHandCursor;

    case MoveItemContent:
      return Qt::ArrowCursor;

    case AddArrow:
    case AddMap:
    case AddRectangle:
    case AddTriangle:
    case AddEllipse:
    case AddHtml:
    case AddLabel:
    case AddScalebar:
    case AddLegend:
    case AddPicture:
    case AddTable:
    case AddAttributeTable:
    {
      QPixmap myCrosshairQPixmap = QPixmap(( const char ** )( cross_hair_cursor ) );
      return QCursor( myCrosshairQPixmap, 8, 8 );
    }
  }
  return Qt::ArrowCursor;
}

void QgsComposerView::addShape( Tool currentTool )
{
  QgsComposerShape::Shape shape = QgsComposerShape::Ellipse;

  if ( currentTool == AddRectangle )
    shape = QgsComposerShape::Rectangle;
  else if ( currentTool == AddTriangle )
    shape = QgsComposerShape::Triangle;

  if ( !mRubberBandItem || ( mRubberBandItem->rect().width() < 0.1 && mRubberBandItem->rect().height() < 0.1 ) )
  {
    removeRubberBand();
    return;
  }
  if ( composition() )
  {
    QgsComposerShape* composerShape = new QgsComposerShape( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height(), composition() );
    composerShape->setShapeType( shape );
    //new shapes use symbol v2 by default
    composerShape->setUseSymbolV2( true );
    composition()->addComposerShape( composerShape );
    removeRubberBand();

    composition()->setAllUnselected();
    composerShape->setSelected( true );
    emit selectedItemChanged( composerShape );

    emit actionFinished();
    composition()->pushAddRemoveCommand( composerShape, tr( "Shape added" ) );
  }
}

void QgsComposerView::updateRulers()
{
  if ( mHorizontalRuler )
  {
    mHorizontalRuler->setSceneTransform( viewportTransform() );
  }
  if ( mVerticalRuler )
  {
    mVerticalRuler->setSceneTransform( viewportTransform() );
  }
}

void QgsComposerView::removeRubberBand()
{
  if ( mRubberBandItem )
  {
    scene()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = 0;
  }
}

void QgsComposerView::startMarqueeSelect( QPointF & scenePoint )
{
  mMarqueeSelect = true;

  QTransform t;
  mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
  mRubberBandItem->setBrush( QBrush( QColor( 224, 178, 76, 63 ) ) );
  mRubberBandItem->setPen( QPen( QBrush( QColor( 254, 58, 29, 100 ) ), 0, Qt::DotLine ) );
  mRubberBandStartPos = QPointF( scenePoint.x(), scenePoint.y() );
  t.translate( scenePoint.x(), scenePoint.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( 1000 );
  scene()->addItem( mRubberBandItem );
  scene()->update();
}

void QgsComposerView::endMarqueeSelect( QMouseEvent* e )
{
  mMarqueeSelect = false;

  bool subtractingSelection = false;
  if ( e->modifiers() & Qt::ShiftModifier )
  {
    //shift modifer means adding to selection, nothing required here
  }
  else if ( e->modifiers() & Qt::ControlModifier )
  {
    //control modifier means subtract from current selection
    subtractingSelection = true;
  }
  else
  {
    //not adding to or removing from selection, so clear current selection
    composition()->setAllUnselected();
  }

  if ( !mRubberBandItem || ( mRubberBandItem->rect().width() < 0.1 && mRubberBandItem->rect().height() < 0.1 ) )
  {
    //just a click, do nothing
    removeRubberBand();
    return;
  }

  QRectF boundsRect = QRectF( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(),
                              mRubberBandItem->rect().width(), mRubberBandItem->rect().height() );

  //determine item selection mode, default to intersection
  Qt::ItemSelectionMode selectionMode = Qt::IntersectsItemShape;
  if ( e->modifiers() & Qt::AltModifier )
  {
    //alt modifier switches to contains selection mode
    selectionMode = Qt::ContainsItemShape;
  }

  //find all items in rubber band
  QList<QGraphicsItem *> itemList = composition()->items( boundsRect, selectionMode );
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* mypItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    QgsPaperItem* paperItem = dynamic_cast<QgsPaperItem*>( *itemIt );
    if ( mypItem && !paperItem )
    {
      if ( !mypItem->positionLock() )
      {
        if ( subtractingSelection )
        {
          mypItem->setSelected( false );
        }
        else
        {
          mypItem->setSelected( true );
        }
      }
    }
  }
  removeRubberBand();

  //update item panel
  QList<QgsComposerItem*> selectedItemList = composition()->selectedComposerItems();
  if ( selectedItemList.size() > 0 )
  {
    emit selectedItemChanged( selectedItemList[0] );
  }
}

void QgsComposerView::startMarqueeZoom( QPointF & scenePoint )
{
  mMarqueeZoom = true;

  QTransform t;
  mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
  mRubberBandItem->setBrush( QBrush( QColor( 70, 50, 255, 25 ) ) );
  mRubberBandItem->setPen( QPen( QColor( 70, 50, 255, 100 ) ) );
  mRubberBandStartPos = QPointF( scenePoint.x(), scenePoint.y() );
  t.translate( scenePoint.x(), scenePoint.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( 1000 );
  scene()->addItem( mRubberBandItem );
  scene()->update();
}

void QgsComposerView::endMarqueeZoom( QMouseEvent* e )
{
  mMarqueeZoom = false;

  QRectF boundsRect;

  if ( !mRubberBandItem || ( mRubberBandItem->rect().width() < 0.1 && mRubberBandItem->rect().height() < 0.1 ) )
  {
    //just a click, so zoom to clicked point and recenter
    double scaleFactor = 0.5;
    //get current visible part of scene
    QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

    //transform the mouse pos to scene coordinates
    QPointF scenePoint = mapToScene( e->pos() );

    visibleRect.scale( scaleFactor, scenePoint.x(), scenePoint.y() );
    boundsRect = visibleRect.toRectF();
  }
  else
  {
    //marquee zoom
    //zoom bounds are size marquee object
    boundsRect = QRectF( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(),
                         mRubberBandItem->rect().width(), mRubberBandItem->rect().height() );
  }

  removeRubberBand();
  //zoom view to fit desired bounds
  fitInView( boundsRect, Qt::KeepAspectRatio );

  if ( mTemporaryZoomStatus == QgsComposerView::ActiveUntilMouseRelease )
  {
    //user was using the temporary keyboard activated zoom tool
    //and the control or space key was released before mouse button, so end temporary zoom
    mTemporaryZoomStatus = QgsComposerView::Inactive;
    setCurrentTool( mPreviousTool );
  }
}

void QgsComposerView::mouseReleaseEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  if ( e->button() != Qt::LeftButton &&
       ( composition()->selectionHandles()->isDragging() || composition()->selectionHandles()->isResizing() ) )
  {
    //ignore clicks while dragging/resizing items
    return;
  }

  QPoint mousePressStopPoint = e->pos();
  int diffX = mousePressStopPoint.x() - mMousePressStartPos.x();
  int diffY = mousePressStopPoint.y() - mMousePressStartPos.y();

  //was this just a click? or a click and drag?
  bool clickOnly = false;
  if ( qAbs( diffX ) < 2 && qAbs( diffY ) < 2 )
  {
    clickOnly = true;
  }

  QPointF scenePoint = mapToScene( e->pos() );

  if ( mMousePanning || mToolPanning )
  {
    mMousePanning = false;
    mToolPanning = false;

    if ( clickOnly && e->button() == Qt::MidButton )
    {
      //middle mouse button click = recenter on point

      //get current visible part of scene
      QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
      QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );
      visibleRect.scale( 1, scenePoint.x(), scenePoint.y() );
      QRectF boundsRect = visibleRect.toRectF();

      //zoom view to fit desired bounds
      fitInView( boundsRect, Qt::KeepAspectRatio );
    }

    //set new cursor
    if ( mCurrentTool != Pan )
    {
      if ( composition() )
      {
        //allow composer items to change cursor
        composition()->setPreventCursorChange( false );
      }
    }
    viewport()->setCursor( defaultCursorForTool( mCurrentTool ) );
  }

  //for every other tool, ignore clicks of non-left button
  if ( e->button() != Qt::LeftButton )
  {
    return;
  }

  if ( mMarqueeSelect )
  {
    endMarqueeSelect( e );
    return;
  }

  switch ( mCurrentTool )
  {
    case Select:
    {
      QGraphicsView::mouseReleaseEvent( e );
      break;
    }

    case Zoom:
    {
      if ( mMarqueeZoom )
      {
        endMarqueeZoom( e );
      }
      break;
    }

    case MoveItemContent:
    {
      if ( mMoveContentItem )
      {
        //update map preview if composer map
        QgsComposerMap* composerMap = dynamic_cast<QgsComposerMap *>( mMoveContentItem );
        if ( composerMap )
        {
          composerMap->setOffset( 0, 0 );
        }

        double moveX = scenePoint.x() - mMoveContentStartPos.x();
        double moveY = scenePoint.y() - mMoveContentStartPos.y();

        composition()->beginCommand( mMoveContentItem, tr( "Move item content" ) );
        mMoveContentItem->moveContent( -moveX, -moveY );
        composition()->endCommand();
        mMoveContentItem = 0;
        mMovingItemContent = false;
      }
      break;
    }
    case AddArrow:
      if ( !composition() || !mRubberBandLineItem )
      {
        scene()->removeItem( mRubberBandLineItem );
        delete mRubberBandLineItem;
        mRubberBandLineItem = 0;
        return;
      }
      else
      {
        QgsComposerArrow* composerArrow = new QgsComposerArrow( mRubberBandLineItem->line().p1(), mRubberBandLineItem->line().p2(), composition() );
        composition()->addComposerArrow( composerArrow );

        composition()->setAllUnselected();
        composerArrow->setSelected( true );
        emit selectedItemChanged( composerArrow );

        scene()->removeItem( mRubberBandLineItem );
        delete mRubberBandLineItem;
        mRubberBandLineItem = 0;
        emit actionFinished();
        composition()->pushAddRemoveCommand( composerArrow, tr( "Arrow added" ) );
      }
      break;

    case AddRectangle:
    case AddTriangle:
    case AddEllipse:
      addShape( mCurrentTool );
      break;

    case AddMap:
      if ( !composition() || !mRubberBandItem || ( mRubberBandItem->rect().width() < 0.1 && mRubberBandItem->rect().height() < 0.1 ) )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerMap* composerMap = new QgsComposerMap( composition(), mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() );
        composition()->addComposerMap( composerMap );

        composition()->setAllUnselected();
        composerMap->setSelected( true );
        emit selectedItemChanged( composerMap );

        removeRubberBand();
        emit actionFinished();
        composition()->pushAddRemoveCommand( composerMap, tr( "Map added" ) );
      }
      break;

    case AddPicture:
      if ( !composition() || !mRubberBandItem || ( mRubberBandItem->rect().width() < 0.1 && mRubberBandItem->rect().height() < 0.1 ) )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerPicture* newPicture = new QgsComposerPicture( composition() );
        newPicture->setSceneRect( QRectF( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() ) );
        composition()->addComposerPicture( newPicture );

        composition()->setAllUnselected();
        newPicture->setSelected( true );
        emit selectedItemChanged( newPicture );

        removeRubberBand();
        emit actionFinished();
        composition()->pushAddRemoveCommand( newPicture, tr( "Picture added" ) );
      }
      break;

    case AddLabel:
      if ( !composition() || !mRubberBandItem )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerLabel* newLabelItem = new QgsComposerLabel( composition() );
        newLabelItem->setText( tr( "QGIS" ) );
        newLabelItem->adjustSizeToText();

        //make sure label size is sufficient to fit text
        double labelWidth = qMax( mRubberBandItem->rect().width(), newLabelItem->rect().width() );
        double labelHeight = qMax( mRubberBandItem->rect().height(), newLabelItem->rect().height() );
        newLabelItem->setSceneRect( QRectF( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), labelWidth, labelHeight ) );

        composition()->addComposerLabel( newLabelItem );

        composition()->setAllUnselected();
        newLabelItem->setSelected( true );
        emit selectedItemChanged( newLabelItem );

        removeRubberBand();
        emit actionFinished();
        composition()->pushAddRemoveCommand( newLabelItem, tr( "Label added" ) );
      }
      break;

    case AddLegend:
      if ( !composition() || !mRubberBandItem )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerLegend* newLegend = new QgsComposerLegend( composition() );
        QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
        if ( mapItemList.size() > 0 )
        {
          newLegend->setComposerMap( mapItemList.at( 0 ) );
        }
        newLegend->setSceneRect( QRectF( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() ) );
        composition()->addComposerLegend( newLegend );
        newLegend->updateLegend();

        composition()->setAllUnselected();
        newLegend->setSelected( true );
        emit selectedItemChanged( newLegend );

        removeRubberBand();
        emit actionFinished();
        composition()->pushAddRemoveCommand( newLegend, tr( "Legend added" ) );
      }
      break;

    case AddTable:
      if ( !composition() || !mRubberBandItem )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerAttributeTable* newTable = new QgsComposerAttributeTable( composition() );
        QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
        if ( mapItemList.size() > 0 )
        {
          newTable->setComposerMap( mapItemList.at( 0 ) );
        }
        newTable->setSceneRect( QRectF( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() ) );

        composition()->addComposerTable( newTable );

        composition()->setAllUnselected();
        newTable->setSelected( true );
        emit selectedItemChanged( newTable );

        removeRubberBand();
        emit actionFinished();
        composition()->pushAddRemoveCommand( newTable, tr( "Table added" ) );
      }
      break;

    case AddAttributeTable:
      if ( !composition() || !mRubberBandItem )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerAttributeTableV2* newTable = new QgsComposerAttributeTableV2( composition(), true );
        QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
        if ( mapItemList.size() > 0 )
        {
          newTable->setComposerMap( mapItemList.at( 0 ) );
        }
        QgsAddRemoveMultiFrameCommand* command = new QgsAddRemoveMultiFrameCommand( QgsAddRemoveMultiFrameCommand::Added,
            newTable, composition(), tr( "Attribute table added" ) );
        composition()->undoStack()->push( command );
        QgsComposerFrame* frame = new QgsComposerFrame( composition(), newTable, mRubberBandItem->transform().dx(),
            mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(),
            mRubberBandItem->rect().height() );
        composition()->beginMultiFrameCommand( newTable, tr( "Attribute table frame added" ) );
        newTable->addFrame( frame );
        composition()->endMultiFrameCommand();

        composition()->setAllUnselected();
        frame->setSelected( true );
        emit selectedItemChanged( frame );

        removeRubberBand();
        emit actionFinished();
      }
      break;

    case AddHtml:
      if ( !composition() || !mRubberBandItem || ( mRubberBandItem->rect().width() < 0.1 && mRubberBandItem->rect().height() < 0.1 ) )
      {
        removeRubberBand();
        return;
      }
      else
      {
        QgsComposerHtml* composerHtml = new QgsComposerHtml( composition(), true );
        QgsAddRemoveMultiFrameCommand* command = new QgsAddRemoveMultiFrameCommand( QgsAddRemoveMultiFrameCommand::Added,
            composerHtml, composition(), tr( "Html item added" ) );
        composition()->undoStack()->push( command );
        QgsComposerFrame* frame = new QgsComposerFrame( composition(), composerHtml, mRubberBandItem->transform().dx(),
            mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(),
            mRubberBandItem->rect().height() );
        composition()->beginMultiFrameCommand( composerHtml, tr( "Html frame added" ) );
        composerHtml->addFrame( frame );
        composition()->endMultiFrameCommand();

        composition()->setAllUnselected();
        frame->setSelected( true );
        emit selectedItemChanged( frame );

        removeRubberBand();
        emit actionFinished();
      }
    default:
      break;
  }
}

void QgsComposerView::mouseMoveEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  bool shiftModifier = false;
  bool altModifier = false;
  if ( e->modifiers() & Qt::ShiftModifier )
  {
    //shift key depressed
    shiftModifier = true;
  }
  if ( e->modifiers() & Qt::AltModifier )
  {
    //alt key depressed
    altModifier = true;
  }

  mMouseCurrentXY = e->pos();
  //update cursor position in composer status bar
  emit cursorPosChanged( mapToScene( e->pos() ) );

  updateRulers();
  if ( mHorizontalRuler )
  {
    mHorizontalRuler->updateMarker( e->posF() );
  }
  if ( mVerticalRuler )
  {
    mVerticalRuler->updateMarker( e->posF() );
  }

  if ( mToolPanning || mMousePanning || mKeyPanning )
  {
    //panning, so scroll view
    horizontalScrollBar()->setValue( horizontalScrollBar()->value() - ( e->x() - mMouseLastXY.x() ) );
    verticalScrollBar()->setValue( verticalScrollBar()->value() - ( e->y() - mMouseLastXY.y() ) );
    mMouseLastXY = e->pos();
    return;
  }
  else if ( e->buttons() == Qt::NoButton )
  {
    if ( mCurrentTool == Select )
    {
      QGraphicsView::mouseMoveEvent( e );
    }
  }
  else
  {
    QPointF scenePoint = mapToScene( e->pos() );

    if ( mMarqueeSelect || mMarqueeZoom )
    {
      updateRubberBandRect( scenePoint );
      return;
    }

    switch ( mCurrentTool )
    {
      case Select:
        QGraphicsView::mouseMoveEvent( e );
        break;

      case AddArrow:
      {
        updateRubberBandLine( scenePoint, shiftModifier );
        break;
      }

      case AddMap:
      case AddRectangle:
      case AddTriangle:
      case AddEllipse:
      case AddHtml:
      case AddPicture:
      case AddLabel:
      case AddLegend:
      case AddTable:
      case AddAttributeTable:
        //adjust rubber band item
      {
        updateRubberBandRect( scenePoint, shiftModifier, altModifier );
        break;
      }

      case MoveItemContent:
      {
        //update map preview if composer map
        QgsComposerMap* composerMap = dynamic_cast<QgsComposerMap *>( mMoveContentItem );
        if ( composerMap )
        {
          composerMap->setOffset( scenePoint.x() - mMoveContentStartPos.x(), scenePoint.y() - mMoveContentStartPos.y() );
          composerMap->update();
        }
        break;
      }
      default:
        break;
    }
  }
}

void QgsComposerView::updateRubberBandRect( QPointF & pos, const bool constrainSquare, const bool fromCenter )
{
  if ( !mRubberBandItem )
  {
    return;
  }

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  double dx = pos.x() - mRubberBandStartPos.x();
  double dy = pos.y() - mRubberBandStartPos.y();

  if ( constrainSquare )
  {
    if ( fabs( dx ) > fabs( dy ) )
    {
      width = fabs( dx );
      height = width;
    }
    else
    {
      height = fabs( dy );
      width = height;
    }

    x = mRubberBandStartPos.x() - (( dx < 0 ) ? width : 0 );
    y = mRubberBandStartPos.y() - (( dy < 0 ) ? height : 0 );
  }
  else
  {
    //not constraining
    if ( dx < 0 )
    {
      x = pos.x();
      width = -dx;
    }
    else
    {
      x = mRubberBandStartPos.x();
      width = dx;
    }

    if ( dy < 0 )
    {
      y = pos.y();
      height = -dy;
    }
    else
    {
      y = mRubberBandStartPos.y();
      height = dy;
    }
  }

  if ( fromCenter )
  {
    x = mRubberBandStartPos.x() - width;
    y = mRubberBandStartPos.y() - height;
    width *= 2.0;
    height *= 2.0;
  }

  mRubberBandItem->setRect( 0, 0, width, height );
  QTransform t;
  t.translate( x, y );
  mRubberBandItem->setTransform( t );
}

void QgsComposerView::updateRubberBandLine( const QPointF &pos, const bool constrainAngles )
{
  if ( !mRubberBandLineItem )
  {
    return;
  }

  //snap to grid
  QPointF snappedScenePoint = composition()->snapPointToGrid( pos );

  QLineF newLine = QLineF( mRubberBandStartPos, snappedScenePoint );

  if ( constrainAngles )
  {
    //movement is contrained to 45 degree angles
    double angle = QgsComposerUtils::snappedAngle( newLine.angle() );
    newLine.setAngle( angle );
  }

  mRubberBandLineItem->setLine( newLine );
}

void QgsComposerView::mouseDoubleClickEvent( QMouseEvent* e )
{
  e->ignore();
}

void QgsComposerView::copyItems( ClipboardMode mode )
{
  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> composerItemList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIt = composerItemList.begin();

  QDomDocument doc;
  QDomElement documentElement = doc.createElement( "ComposerItemClipboard" );
  for ( ; itemIt != composerItemList.end(); ++itemIt )
  {
    // copy each item in a group
    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup*>( *itemIt );
    if ( itemGroup && composition() )
    {
      QSet<QgsComposerItem*> groupedItems = itemGroup->items();
      QSet<QgsComposerItem*>::iterator it = groupedItems.begin();
      for ( ; it != groupedItems.end(); ++it )
      {
        ( *it )->writeXML( documentElement, doc );
      }
    }
    ( *itemIt )->writeXML( documentElement, doc );
    if ( mode == ClipboardModeCut )
    {
      composition()->removeComposerItem( *itemIt );
    }
  }
  doc.appendChild( documentElement );

  //if it's a copy, we have to remove the UUIDs since we don't want any duplicate UUID
  if ( mode == ClipboardModeCopy )
  {
    // remove all uuid attributes
    QDomNodeList composerItemsNodes = doc.elementsByTagName( "ComposerItem" );
    for ( int i = 0; i < composerItemsNodes.count(); ++i )
    {
      QDomNode composerItemNode = composerItemsNodes.at( i );
      if ( composerItemNode.isElement() )
      {
        composerItemNode.toElement().removeAttribute( "uuid" );
      }
    }
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( "text/xml", doc.toByteArray() );
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setMimeData( mimeData );
}

void QgsComposerView::pasteItems( PasteMode mode )
{
  if ( !composition() )
  {
    return;
  }

  QDomDocument doc;
  QClipboard *clipboard = QApplication::clipboard();
  if ( doc.setContent( clipboard->mimeData()->data( "text/xml" ) ) )
  {
    QDomElement docElem = doc.documentElement();
    if ( docElem.tagName() == "ComposerItemClipboard" )
    {
      if ( composition() )
      {
        QPointF pt;
        if ( mode == QgsComposerView::PasteModeCursor || mode == QgsComposerView::PasteModeInPlace )
        {
          // place items at cursor position
          pt = mapToScene( mapFromGlobal( QCursor::pos() ) );
        }
        else
        {
          // place items in center of viewport
          pt = mapToScene( viewport()->rect().center() );
        }
        bool pasteInPlace = ( mode == PasteModeInPlace );
        composition()->addItemsFromXML( docElem, doc, 0, true, &pt, pasteInPlace );
      }
    }
  }

  //switch back to select tool so that pasted items can be moved/resized (#8958)
  setCurrentTool( QgsComposerView::Select );
}

void QgsComposerView::deleteSelectedItems()
{
  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> composerItemList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIt = composerItemList.begin();

  //delete selected items
  for ( ; itemIt != composerItemList.end(); ++itemIt )
  {
    if ( composition() )
    {
      composition()->removeComposerItem( *itemIt );
    }
  }
}

void QgsComposerView::selectAll()
{
  if ( !composition() )
  {
    return;
  }

  //select all items in composer
  QList<QGraphicsItem *> itemList = composition()->items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* mypItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    QgsPaperItem* paperItem = dynamic_cast<QgsPaperItem*>( *itemIt );
    if ( mypItem && !paperItem )
    {
      if ( !mypItem->positionLock() )
      {
        mypItem->setSelected( true );
      }
      else
      {
        //deselect all locked items
        mypItem->setSelected( false );
      }
      emit selectedItemChanged( mypItem );
    }
  }
}

void QgsComposerView::selectNone()
{
  if ( !composition() )
  {
    return;
  }

  composition()->setAllUnselected();
}

void QgsComposerView::selectInvert()
{
  if ( !composition() )
  {
    return;
  }

  //check all items in composer
  QList<QGraphicsItem *> itemList = composition()->items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* mypItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    QgsPaperItem* paperItem = dynamic_cast<QgsPaperItem*>( *itemIt );
    if ( mypItem && !paperItem )
    {
      //flip selected state for items (and deselect any locked items)
      if ( mypItem->selected() || mypItem->positionLock() )
      {

        mypItem->setSelected( false );
      }
      else
      {
        mypItem->setSelected( true );
        emit selectedItemChanged( mypItem );
      }
    }
  }
}

void QgsComposerView::keyPressEvent( QKeyEvent * e )
{
  if ( !composition() )
  {
    return;
  }

  if ( mKeyPanning || mMousePanning || mToolPanning || mMovingItemContent ||
       composition()->selectionHandles()->isDragging() || composition()->selectionHandles()->isResizing() )
  {
    return;
  }

  if ( mTemporaryZoomStatus != QgsComposerView::Inactive )
  {
    //temporary keyboard based zoom is active
    if ( e->isAutoRepeat() )
    {
      return;
    }

    //respond to changes in ctrl key status
    if ( !( e->modifiers() & Qt::ControlModifier ) && !mMarqueeZoom )
    {
      //space pressed, but control key was released, end of temporary zoom tool
      mTemporaryZoomStatus = QgsComposerView::Inactive;
      setCurrentTool( mPreviousTool );
    }
    else if ( !( e->modifiers() & Qt::ControlModifier ) && mMarqueeZoom )
    {
      //control key released, but user is mid-way through a marquee zoom
      //so end temporary zoom when user releases the mouse button
      mTemporaryZoomStatus = QgsComposerView::ActiveUntilMouseRelease;
    }
    else
    {
      //both control and space pressed
      //set cursor to zoom in/out depending on shift key status
      QPixmap myZoomQPixmap = QPixmap(( const char ** )( e->modifiers() & Qt::ShiftModifier ? zoom_out : zoom_in ) );
      QCursor zoomCursor = QCursor( myZoomQPixmap, 7, 7 );
      viewport()->setCursor( zoomCursor );
    }
    return;
  }

  if ( mCurrentTool != QgsComposerView::Zoom && ( mRubberBandItem || mRubberBandLineItem ) )
  {
    //disable keystrokes while drawing a box
    return;
  }

  if ( e->key() == Qt::Key_Space && ! e->isAutoRepeat() )
  {
    if ( !( e->modifiers() & Qt::ControlModifier ) )
    {
      // Pan composer with space bar
      mKeyPanning = true;
      mMouseLastXY = mMouseCurrentXY;
      if ( composition() )
      {
        //prevent cursor changes while panning
        composition()->setPreventCursorChange( true );
      }
      viewport()->setCursor( Qt::ClosedHandCursor );
      return;
    }
    else
    {
      //ctrl+space pressed, so switch to temporary keyboard based zoom tool
      mTemporaryZoomStatus = QgsComposerView::Active;
      mPreviousTool = mCurrentTool;
      setCurrentTool( Zoom );
      //set cursor to zoom in/out depending on shift key status
      QPixmap myZoomQPixmap = QPixmap(( const char ** )( e->modifiers() & Qt::ShiftModifier ? zoom_out : zoom_in ) );
      QCursor zoomCursor = QCursor( myZoomQPixmap, 7, 7 );
      viewport()->setCursor( zoomCursor );
      return;
    }
  }

  if ( mCurrentTool == QgsComposerView::Zoom )
  {
    //using the zoom tool, respond to changes in shift key status and update mouse cursor accordingly
    if ( ! e->isAutoRepeat() )
    {
      QPixmap myZoomQPixmap = QPixmap(( const char ** )( e->modifiers() & Qt::ShiftModifier ? zoom_out : zoom_in ) );
      QCursor zoomCursor = QCursor( myZoomQPixmap, 7, 7 );
      viewport()->setCursor( zoomCursor );
    }
    return;
  }

  QList<QgsComposerItem*> composerItemList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIt = composerItemList.begin();

  // increment used for cursor key item movement
  double increment = 1.0;
  if ( e->modifiers() & Qt::ShiftModifier )
  {
    //holding shift while pressing cursor keys results in a big step
    increment = 10.0;
  }
  else if ( e->modifiers() & Qt::AltModifier )
  {
    //holding alt while pressing cursor keys results in a 1 pixel step
    double viewScale = transform().m11();
    if ( viewScale > 0 )
    {
      increment = 1 / viewScale;
    }
  }

  if ( e->key() == Qt::Key_Left )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->beginCommand( tr( "Item moved" ), QgsComposerMergeCommand::ItemMove );
      ( *itemIt )->move( -1 * increment, 0.0 );
      ( *itemIt )->endCommand();
    }
  }
  else if ( e->key() == Qt::Key_Right )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->beginCommand( tr( "Item moved" ), QgsComposerMergeCommand::ItemMove );
      ( *itemIt )->move( increment, 0.0 );
      ( *itemIt )->endCommand();
    }
  }
  else if ( e->key() == Qt::Key_Down )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->beginCommand( tr( "Item moved" ), QgsComposerMergeCommand::ItemMove );
      ( *itemIt )->move( 0.0, increment );
      ( *itemIt )->endCommand();
    }
  }
  else if ( e->key() == Qt::Key_Up )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->beginCommand( tr( "Item moved" ), QgsComposerMergeCommand::ItemMove );
      ( *itemIt )->move( 0.0, -1 * increment );
      ( *itemIt )->endCommand();
    }
  }
}

void QgsComposerView::keyReleaseEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Space && !e->isAutoRepeat() && mKeyPanning )
  {
    //end of panning with space key
    mKeyPanning = false;

    //reset cursor
    if ( mCurrentTool != Pan )
    {
      if ( composition() )
      {
        //allow cursor changes again
        composition()->setPreventCursorChange( false );
      }
    }
    viewport()->setCursor( defaultCursorForTool( mCurrentTool ) );
    return;
  }
  else if ( e->key() == Qt::Key_Space && !e->isAutoRepeat() && mTemporaryZoomStatus != QgsComposerView::Inactive )
  {
    //temporary keyboard-based zoom tool is active and space key has been released
    if ( mMarqueeZoom )
    {
      //currently in the middle of a marquee operation, so don't switch tool back immediately
      //instead, wait until mouse button has been released before switching tool back
      mTemporaryZoomStatus = QgsComposerView::ActiveUntilMouseRelease;
    }
    else
    {
      //switch tool back
      mTemporaryZoomStatus = QgsComposerView::Inactive;
      setCurrentTool( mPreviousTool );
    }
  }
  else if ( mCurrentTool == QgsComposerView::Zoom )
  {
    //if zoom tool is active, respond to changes in the shift key status and update cursor accordingly
    if ( ! e->isAutoRepeat() )
    {
      QPixmap myZoomQPixmap = QPixmap(( const char ** )( e->modifiers() & Qt::ShiftModifier ? zoom_out : zoom_in ) );
      QCursor zoomCursor = QCursor( myZoomQPixmap, 7, 7 );
      viewport()->setCursor( zoomCursor );
    }
    return;
  }
}

void QgsComposerView::wheelEvent( QWheelEvent* event )
{
  if ( mRubberBandItem || mRubberBandLineItem )
  {
    //ignore wheel events while marquee operations are active (eg, creating new item)
    return;
  }

  if ( composition()->selectionHandles()->isDragging() || composition()->selectionHandles()->isResizing() )
  {
    //ignore wheel events while dragging/resizing items
    return;
  }

  if ( currentTool() == MoveItemContent )
  {
    //move item content tool, so scroll events get handled by the selected composer item

    QPointF scenePoint = mapToScene( event->pos() );
    //select topmost item at position of event
    QgsComposerItem* theItem = composition()->composerItemAt( scenePoint, true );
    if ( theItem )
    {
      if ( theItem->isSelected() )
      {
        QSettings settings;
        //read zoom mode
        QgsComposerItem::ZoomMode zoomMode = ( QgsComposerItem::ZoomMode )settings.value( "/qgis/wheel_action", 2 ).toInt();
        if ( zoomMode == QgsComposerItem::NoZoom )
        {
          //do nothing
          return;
        }

        double zoomFactor = settings.value( "/qgis/zoom_factor", 2.0 ).toDouble();
        if ( event->modifiers() & Qt::ControlModifier )
        {
          //holding ctrl while wheel zooming results in a finer zoom
          zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
        }
        zoomFactor = event->delta() > 0 ? zoomFactor : 1 / zoomFactor;

        QPointF itemPoint = theItem->mapFromScene( scenePoint );
        theItem->beginCommand( tr( "Zoom item content" ), QgsComposerMergeCommand::ItemZoomContent );
        theItem->zoomContent( zoomFactor, itemPoint, zoomMode );
        theItem->endCommand();
      }
    }
  }
  else
  {
    //not using move item content tool, so zoom whole composition
    wheelZoom( event );
  }
}

void QgsComposerView::wheelZoom( QWheelEvent * event )
{
  //get mouse wheel zoom behaviour settings
  QSettings mySettings;
  int wheelAction = mySettings.value( "/qgis/wheel_action", 2 ).toInt();
  double zoomFactor = mySettings.value( "/qgis/zoom_factor", 2 ).toDouble();

  if (( QgsMapCanvas::WheelAction )wheelAction == QgsMapCanvas::WheelNothing )
  {
    return;
  }

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 10.0;
  }

  //caculate zoom scale factor
  bool zoomIn = event->delta() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
  QPointF scenePoint = mapToScene( event->pos() );

  //adjust view center according to wheel action setting
  switch (( QgsMapCanvas::WheelAction )wheelAction )
  {
    case QgsMapCanvas::WheelZoomAndRecenter:
    {
      centerOn( scenePoint.x(), scenePoint.y() );
      break;
    }

    case QgsMapCanvas::WheelZoomToMouseCursor:
    {
      QgsPoint oldCenter( visibleRect.center() );
      QgsPoint newCenter( scenePoint.x() + (( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                          scenePoint.y() + (( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );
      centerOn( newCenter.x(), newCenter.y() );
      break;
    }

    default:
      break;
  }

  //zoom composition
  if ( zoomIn )
  {
    scale( zoomFactor, zoomFactor );
  }
  else
  {
    scale( 1 / zoomFactor, 1 / zoomFactor );
  }

  //update composition for new zoom
  emit zoomLevelChanged();
  updateRulers();
  update();
  //redraw cached map items
  QList<QGraphicsItem *> itemList = composition()->items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerMap* mypItem = dynamic_cast<QgsComposerMap *>( *itemIt );
    if (( mypItem ) && ( mypItem->previewMode() == QgsComposerMap::Render ) )
    {
      mypItem->updateCachedImage();
    }
  }
}

void QgsComposerView::setZoomLevel( double zoomLevel )
{
  double dpi = QgsApplication::desktop()->logicalDpiX();
  //monitor dpi is not always correct - so make sure the value is sane
  if (( dpi < 60 ) || ( dpi > 250 ) )
    dpi = 72;

  //desired pixel width for 1mm on screen
  double scale = zoomLevel * dpi / 25.4;
  setTransform( QTransform::fromScale( scale, scale ) );

  updateRulers();
  update();
  emit zoomLevelChanged();
}

void QgsComposerView::setPreviewModeEnabled( bool enabled )
{
  if ( !mPreviewEffect )
  {
    return;
  }

  mPreviewEffect->setEnabled( enabled );
}

void QgsComposerView::setPreviewMode( QgsPreviewEffect::PreviewMode mode )
{
  if ( !mPreviewEffect )
  {
    return;
  }

  mPreviewEffect->setMode( mode );
}

void QgsComposerView::paintEvent( QPaintEvent* event )
{
  if ( mPaintingEnabled )
  {
    QGraphicsView::paintEvent( event );
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QgsComposerView::hideEvent( QHideEvent* e )
{
  emit( composerViewHide( this ) );
  e->ignore();
}

void QgsComposerView::showEvent( QShowEvent* e )
{
  emit( composerViewShow( this ) );
  e->ignore();
}

void QgsComposerView::resizeEvent( QResizeEvent* event )
{
  QGraphicsView::resizeEvent( event );
  emit zoomLevelChanged();
  updateRulers();
}

void QgsComposerView::scrollContentsBy( int dx, int dy )
{
  QGraphicsView::scrollContentsBy( dx, dy );
  updateRulers();
}

void QgsComposerView::setComposition( QgsComposition* c )
{
  setScene( c );
  if ( mHorizontalRuler )
  {
    mHorizontalRuler->setComposition( c );
  }
  if ( mVerticalRuler )
  {
    mVerticalRuler->setComposition( c );
  }

  //emit compositionSet, so that composer windows can update for the new composition
  emit compositionSet( c );
}

QgsComposition* QgsComposerView::composition()
{
  if ( scene() )
  {
    QgsComposition* c = dynamic_cast<QgsComposition *>( scene() );
    if ( c )
    {
      return c;
    }
  }
  return 0;
}

void QgsComposerView::groupItems()
{
  if ( !composition() )
  {
    return;
  }

  //group selected items
  QList<QgsComposerItem*> selectionList = composition()->selectedComposerItems();
  QgsComposerItemGroup* itemGroup = composition()->groupItems( selectionList );

  if ( !itemGroup )
  {
    //group could not be created
    return;
  }

  itemGroup->setSelected( true );
  emit selectedItemChanged( itemGroup );
}

void QgsComposerView::ungroupItems()
{
  if ( !composition() )
  {
    return;
  }

  //hunt through selection for any groups, and ungroup them
  QList<QgsComposerItem*> selectionList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIter = selectionList.begin();
  for ( ; itemIter != selectionList.end(); ++itemIter )
  {
    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup *>( *itemIter );
    if ( itemGroup )
    {
      composition()->ungroupItems( itemGroup );
    }
  }
}

QMainWindow* QgsComposerView::composerWindow()
{
  QMainWindow* composerObject = 0;
  QObject* currentObject = parent();
  if ( !currentObject )
  {
    return qobject_cast<QMainWindow *>( currentObject );
  }

  while ( true )
  {
    composerObject = qobject_cast<QMainWindow*>( currentObject );
    if ( composerObject || currentObject->parent() == 0 )
    {
      return composerObject;
    }
    currentObject = currentObject->parent();
  }

  return 0;
}
