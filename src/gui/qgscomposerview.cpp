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
#include "qgscomposerattributetable.h"
#include "qgslogger.h"
#include "qgsaddremovemultiframecommand.h"
#include "qgspaperitem.h"
#include "qgsmapcanvas.h" //for QgsMapCanvas::WheelAction

QgsComposerView::QgsComposerView( QWidget* parent, const char* name, Qt::WFlags f )
    : QGraphicsView( parent )
    , mRubberBandItem( 0 )
    , mRubberBandLineItem( 0 )
    , mMoveContentItem( 0 )
    , mPaintingEnabled( true )
    , mHorizontalRuler( 0 )
    , mVerticalRuler( 0 )
{
  Q_UNUSED( f );
  Q_UNUSED( name );

  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );
  setFrameShape( QFrame::NoFrame );
}

void QgsComposerView::mousePressEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  QPointF scenePoint = mapToScene( e->pos() );
  QPointF snappedScenePoint = composition()->snapPointToGrid( scenePoint );

  //lock/unlock position of item with right click
  if ( e->button() == Qt::RightButton )
  {
    QgsComposerItem* selectedItem = composition()->composerItemAt( scenePoint );
    if ( selectedItem )
    {
      bool lock = selectedItem->positionLock() ? false : true;
      selectedItem->setPositionLock( lock );
      selectedItem->update();
    }
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
        selectedItem = composition()->composerItemAt( scenePoint, previousSelectedItem );

        //if we didn't find a lower item we'll use the top-most as fall-back
        //this duplicates mapinfo/illustrator/etc behaviour where ctrl-clicks are "cyclic"
        if ( !selectedItem )
        {
          selectedItem = composition()->composerItemAt( scenePoint );
        }
      }
      else
      {
        //select topmost item at position of event
        selectedItem = composition()->composerItemAt( scenePoint );
      }

      if ( !selectedItem )
      {
        composition()->clearSelection();
        break;
      }

      if (( !selectedItem->selected() ) &&        //keep selection if an already selected item pressed
          !( e->modifiers() & Qt::ShiftModifier ) ) //keep selection if shift key pressed
      {
        composition()->clearSelection();
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

    case MoveItemContent:
    {
      //get a list of items at clicked position
      QList<QGraphicsItem *> itemsAtCursorPos = items( e->pos() );
      if ( itemsAtCursorPos.size() == 0 )
      {
        //no items at clicked position
        return;
      }

      //find highest QgsComposerItem at clicked position
      //(other graphics items may be higher, eg selection handles)
      QList<QGraphicsItem*>::iterator itemIter = itemsAtCursorPos.begin();
      for ( ; itemIter != itemsAtCursorPos.end(); ++itemIter )
      {
        QgsComposerItem* item = dynamic_cast<QgsComposerItem *>(( *itemIter ) );
        if ( item )
        {
          //we've found the highest QgsComposerItem
          mMoveContentStartPos = scenePoint;
          mMoveContentItem = item;
          break;
        }
      }

      //no QgsComposerItem at clicked position
      return;
    }

    case AddArrow:
    {
      mRubberBandStartPos = QPointF( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandLineItem = new QGraphicsLineItem( snappedScenePoint.x(), snappedScenePoint.y(), snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandLineItem->setZValue( 100 );
      scene()->addItem( mRubberBandLineItem );
      scene()->update();
      break;
    }

    //create rubber band for map and ellipse items
    case AddMap:
    case AddRectangle:
    case AddTriangle:
    case AddEllipse:
    case AddHtml:
    {
      QTransform t;
      mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
      mRubberBandStartPos = QPointF( snappedScenePoint.x(), snappedScenePoint.y() );
      t.translate( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandItem->setTransform( t );
      mRubberBandItem->setZValue( 100 );
      scene()->addItem( mRubberBandItem );
      scene()->update();
    }
    break;

    case AddLabel:
      if ( composition() )
      {
        QgsComposerLabel* newLabelItem = new QgsComposerLabel( composition() );
        newLabelItem->setText( tr( "QGIS" ) );
        newLabelItem->adjustSizeToText();
        newLabelItem->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), newLabelItem->rect().width(), newLabelItem->rect().height() ) );
        composition()->addComposerLabel( newLabelItem );
        emit actionFinished();
        composition()->pushAddRemoveCommand( newLabelItem, tr( "Label added" ) );
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
        emit actionFinished();
        composition()->pushAddRemoveCommand( newScaleBar, tr( "Scale bar added" ) );
      }
      break;

    case AddLegend:
    {
      if ( composition() )
      {
        QgsComposerLegend* newLegend = new QgsComposerLegend( composition() );
        newLegend->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), newLegend->rect().width(), newLegend->rect().height() ) );
        composition()->addComposerLegend( newLegend );
        newLegend->updateLegend();
        emit actionFinished();
        composition()->pushAddRemoveCommand( newLegend, tr( "Legend added" ) );
      }
      break;
    }
    case AddPicture:
      if ( composition() )
      {
        QgsComposerPicture* newPicture = new QgsComposerPicture( composition() );
        newPicture->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 30, 30 ) );
        composition()->addComposerPicture( newPicture );
        emit actionFinished();
        composition()->pushAddRemoveCommand( newPicture, tr( "Picture added" ) );
      }
      break;
    case AddTable:
      if ( composition() )
      {
        QgsComposerAttributeTable* newTable = new QgsComposerAttributeTable( composition() );
        newTable->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 50, 50 ) );
        composition()->addComposerTable( newTable );
        emit actionFinished();
        composition()->pushAddRemoveCommand( newTable, tr( "Table added" ) );
      }
      break;
    default:
      break;
  }
}

void QgsComposerView::addShape( Tool currentTool )
{
  QgsComposerShape::Shape shape = QgsComposerShape::Ellipse;

  if ( currentTool == AddRectangle )
    shape = QgsComposerShape::Rectangle;
  else if ( currentTool == AddTriangle )
    shape = QgsComposerShape::Triangle;

  if ( !mRubberBandItem || mRubberBandItem->rect().width() < 0.1 || mRubberBandItem->rect().width() < 0.1 )
  {
    scene()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = 0;
    return;
  }
  if ( composition() )
  {
    QgsComposerShape* composerShape = new QgsComposerShape( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height(), composition() );
    composerShape->setShapeType( shape );
    composition()->addComposerShape( composerShape );
    scene()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = 0;
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

void QgsComposerView::mouseReleaseEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  QPointF scenePoint = mapToScene( e->pos() );

  switch ( mCurrentTool )
  {
    case Select:
    {
      QGraphicsView::mouseReleaseEvent( e );
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
      }
      break;
    }
    case AddArrow:
      if ( composition() )
      {
        QPointF scenePoint = mapToScene( e->pos() );
        QPointF snappedScenePoint = composition()->snapPointToGrid( scenePoint );
        QgsComposerArrow* composerArrow = new QgsComposerArrow( mRubberBandStartPos, QPointF( snappedScenePoint.x(), snappedScenePoint.y() ), composition() );
        composition()->addComposerArrow( composerArrow );
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
      if ( !mRubberBandItem || mRubberBandItem->rect().width() < 0.1 || mRubberBandItem->rect().width() < 0.1 )
      {
        if ( mRubberBandItem )
        {
          scene()->removeItem( mRubberBandItem );
          delete mRubberBandItem;
          mRubberBandItem = 0;
        }
        return;
      }
      if ( composition() )
      {
        QgsComposerMap* composerMap = new QgsComposerMap( composition(), mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() );
        composition()->addComposerMap( composerMap );
        scene()->removeItem( mRubberBandItem );
        delete mRubberBandItem;
        mRubberBandItem = 0;
        emit actionFinished();
        composition()->pushAddRemoveCommand( composerMap, tr( "Map added" ) );
      }
      break;

    case AddHtml:
      if ( composition() )
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
        scene()->removeItem( mRubberBandItem );
        delete mRubberBandItem;
        mRubberBandItem = 0;
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

  updateRulers();
  if ( mHorizontalRuler )
  {
    mHorizontalRuler->updateMarker( e->posF() );
  }
  if ( mVerticalRuler )
  {
    mVerticalRuler->updateMarker( e->posF() );
  }

  if ( e->buttons() == Qt::NoButton )
  {
    if ( mCurrentTool == Select )
    {
      QGraphicsView::mouseMoveEvent( e );
    }
  }
  else
  {
    QPointF scenePoint = mapToScene( e->pos() );

    switch ( mCurrentTool )
    {
      case Select:
        QGraphicsView::mouseMoveEvent( e );
        break;

      case AddArrow:
      {
        if ( mRubberBandLineItem )
        {
          mRubberBandLineItem->setLine( mRubberBandStartPos.x(), mRubberBandStartPos.y(),  scenePoint.x(),  scenePoint.y() );
        }
        break;
      }

      case AddMap:
      case AddRectangle:
      case AddTriangle:
      case AddEllipse:
      case AddHtml:
        //adjust rubber band item
      {
        double x = 0;
        double y = 0;
        double width = 0;
        double height = 0;

        double dx = scenePoint.x() - mRubberBandStartPos.x();
        double dy = scenePoint.y() - mRubberBandStartPos.y();

        if ( dx < 0 )
        {
          x = scenePoint.x();
          width = -dx;
        }
        else
        {
          x = mRubberBandStartPos.x();
          width = dx;
        }

        if ( dy < 0 )
        {
          y = scenePoint.y();
          height = -dy;
        }
        else
        {
          y = mRubberBandStartPos.y();
          height = dy;
        }

        if ( mRubberBandItem )
        {
          mRubberBandItem->setRect( 0, 0, width, height );
          QTransform t;
          t.translate( x, y );
          mRubberBandItem->setTransform( t );
        }
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
        if ( mode == PasteModeCursor )
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

  composition()->clearSelection();
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

  QList<QgsComposerItem*> composerItemList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIt = composerItemList.begin();

  // increment used for cursor key item movement
  double increment = 1.0;
  if ( e->modifiers() & Qt::ShiftModifier )
  {
    //holding shift while pressing cursor keys results in a big step
    increment = 10.0;
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

void QgsComposerView::wheelEvent( QWheelEvent* event )
{
  if ( currentTool() == MoveItemContent )
  {
    //move item content tool, so scroll events get handled by the selected composer item

    QPointF scenePoint = mapToScene( event->pos() );
    //select topmost item at position of event
    QgsComposerItem* theItem = composition()->composerItemAt( scenePoint );
    if ( theItem )
    {
      if ( theItem->isSelected() )
      {
        QPointF itemPoint = theItem->mapFromScene( scenePoint );
        theItem->beginCommand( tr( "Zoom item content" ) );
        theItem->zoomContent( event->delta(), itemPoint.x(), itemPoint.y() );
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

  //caculate zoom scale factor
  bool zoomIn = event->delta() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
  QPointF scenePoint = mapToScene( event->pos() );

  //zoom composition, respecting wheel action setting
  switch (( QgsMapCanvas::WheelAction )wheelAction )
  {
    case QgsMapCanvas::WheelZoom:
      // zoom without changing extent
      if ( zoomIn )
      {
        scale( zoomFactor, zoomFactor );
      }
      else
      {
        scale( 1 / zoomFactor, 1 / zoomFactor );
      }
      break;

    case QgsMapCanvas::WheelZoomAndRecenter:
    {
      visibleRect.scale( scaleFactor, scenePoint.x(), scenePoint.y() );
      fitInView( visibleRect.toRectF(), Qt::KeepAspectRatio );
      break;
    }

    case QgsMapCanvas::WheelZoomToMouseCursor:
    {
      QgsPoint oldCenter( visibleRect.center() );
      QgsPoint newCenter( scenePoint.x() + (( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                          scenePoint.y() + (( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );

      visibleRect.scale( scaleFactor, newCenter.x(), newCenter.y() );
      fitInView( visibleRect.toRectF(), Qt::KeepAspectRatio );
      break;
    }

    case QgsMapCanvas::WheelNothing:
      return;
  }

  //update composition for new zoom
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

  QList<QgsComposerItem*> selectionList = composition()->selectedComposerItems();
  if ( selectionList.size() < 2 )
  {
    return; //not enough items for a group
  }
  QgsComposerItemGroup* itemGroup = new QgsComposerItemGroup( composition() );

  QList<QgsComposerItem*>::iterator itemIter = selectionList.begin();
  for ( ; itemIter != selectionList.end(); ++itemIter )
  {
    itemGroup->addItem( *itemIter );
  }

  composition()->addItem( itemGroup );
  itemGroup->setSelected( true );
  emit selectedItemChanged( itemGroup );
}

void QgsComposerView::ungroupItems()
{
  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> selectionList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIter = selectionList.begin();
  for ( ; itemIter != selectionList.end(); ++itemIter )
  {
    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup *>( *itemIter );
    if ( itemGroup )
    {
      itemGroup->removeItems();
      composition()->removeItem( *itemIter );
      delete( *itemIter );
      emit itemRemoved( *itemIter );
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
