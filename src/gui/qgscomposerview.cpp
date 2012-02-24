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

#include "qgscomposerview.h"
#include "qgscomposerarrow.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgscomposerattributetable.h"
#include "qgslogger.h"

QgsComposerView::QgsComposerView( QWidget* parent, const char* name, Qt::WFlags f )
    : QGraphicsView( parent )
    , mShiftKeyPressed( false )
    , mRubberBandItem( 0 )
    , mRubberBandLineItem( 0 )
    , mMoveContentItem( 0 )
    , mPaintingEnabled( true )
{
  Q_UNUSED( f );
  Q_UNUSED( name );

  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );
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
      //make sure the new cursor is correct
      QPointF itemPoint = selectedItem->mapFromScene( scenePoint );
      selectedItem->updateCursor( itemPoint );
    }
    return;
  }

  switch ( mCurrentTool )
  {
      //select/deselect items and pass mouse event further
    case Select:
    {
      if ( !mShiftKeyPressed ) //keep selection if shift key pressed
      {
        composition()->clearSelection();
      }

      //select topmost item at position of event
      QgsComposerItem* selectedItem = composition()->composerItemAt( scenePoint );
      if ( !selectedItem )
      {
        break;
      }

      selectedItem->setSelected( true );
      QGraphicsView::mousePressEvent( e );
      emit selectedItemChanged( selectedItem );
      break;
    }

    case MoveItemContent:
    {
      //store item as member if it is selected and cursor is over item
      QgsComposerItem* item = dynamic_cast<QgsComposerItem *>( itemAt( e->pos() ) );
      if ( item )
      {
        mMoveContentStartPos = scenePoint;
      }
      mMoveContentItem = item;
      break;
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
        newLabelItem->setText( tr( "Quantum GIS" ) );
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

void QgsComposerView::keyPressEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Shift )
  {
    mShiftKeyPressed = true;
  }

  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> composerItemList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIt = composerItemList.begin();

  if ( e->matches( QKeySequence::Copy ) || e->matches( QKeySequence::Cut ) )
  {
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
      if ( e->matches( QKeySequence::Cut ) )
      {
        composition()->removeComposerItem( *itemIt );
      }
    }
    doc.appendChild( documentElement );
    QMimeData *mimeData = new QMimeData;
    mimeData->setData( "text/xml", doc.toByteArray() );
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData( mimeData );
  }

  if ( e->matches( QKeySequence::Paste ) )
  {
    QDomDocument doc;
    QClipboard *clipboard = QApplication::clipboard();
    if ( doc.setContent( clipboard->mimeData()->data( "text/xml" ) ) )
    {
      QDomElement docElem = doc.documentElement();
      if ( docElem.tagName() == "ComposerItemClipboard" )
      {
        if ( composition() )
        {
          QPointF pt = mapToScene( mapFromGlobal( QCursor::pos() ) );
          composition()->addItemsFromXML( docElem, doc, 0, true, &pt );
        }
      }
    }
  }

  //delete selected items
  if ( e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      if ( composition() )
      {
        composition()->removeComposerItem( *itemIt );
      }
    }
  }

  else if ( e->key() == Qt::Key_Left )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( -1.0, 0.0 );
    }
  }
  else if ( e->key() == Qt::Key_Right )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( 1.0, 0.0 );
    }
  }
  else if ( e->key() == Qt::Key_Down )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( 0.0, 1.0 );
    }
  }
  else if ( e->key() == Qt::Key_Up )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( 0.0, -1.0 );
    }
  }
}

void QgsComposerView::keyReleaseEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Shift )
  {
    mShiftKeyPressed = false;
  }
}

void QgsComposerView::wheelEvent( QWheelEvent* event )
{
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

void QgsComposerView::setComposition( QgsComposition* c )
{
  setScene( c );
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
