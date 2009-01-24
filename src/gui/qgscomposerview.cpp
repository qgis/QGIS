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

#include <QMouseEvent>
#include <QKeyEvent>

#include "qgscomposerview.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"

QgsComposerView::QgsComposerView( QWidget* parent, const char* name, Qt::WFlags f ) :
    QGraphicsView( parent ), mShiftKeyPressed( false ), mRubberBandItem( 0 ), mMoveContentItem( 0 )
{
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

  QPointF scenePoint = mapToScene( e->pos());
  QPointF snappedScenePoint = composition()->snapPointToGrid(scenePoint);

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
      if ( selectedItem )
      {
        selectedItem->setSelected( true );
      }


      QGraphicsView::mousePressEvent( e );
      emit selectedItemChanged( selectedItem );
      break;
    }

    case MoveItemContent:
    {
      //store item as member if it is selected and cursor is over item
      QgsComposerItem* item = dynamic_cast<QgsComposerItem*>( itemAt( e->pos() ) );
      if ( item )
      {
        mMoveContentStartPos = scenePoint;
      }
      mMoveContentItem = item;
      break;
    }

    //create rubber band
    case AddMap:
    {
      QTransform t;
      mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
      t.translate( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandItem->setTransform( t );
      mRubberBandItem->setZValue( 100 );
      scene()->addItem( mRubberBandItem );
      scene()->update();
    }
    break;

    case AddLabel:
    {
      QgsComposerLabel* newLabelItem = new QgsComposerLabel( composition() );
      newLabelItem->setText( "Quantum GIS" );
      newLabelItem->adjustSizeToText();
      scene()->addItem( newLabelItem );
      emit composerLabelAdded( newLabelItem );
      scene()->clearSelection();
      newLabelItem->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), newLabelItem->rect().width(), newLabelItem->rect().height() ) );
      newLabelItem->setSelected( true );
      emit selectedItemChanged( newLabelItem );
      emit actionFinished();
    }
    break;

    case AddScalebar:
    {
      QgsComposerScaleBar* newScaleBar = new QgsComposerScaleBar( composition() );

      //take first available map...
      QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
      if ( mapItemList.size() > 0 )
      {
        newScaleBar->setComposerMap( mapItemList.at( 0 ) );
      }

      newScaleBar->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 20, 20 ) );
      newScaleBar->applyDefaultSettings(); //4 segments, 1/5 of composer map width
      scene()->addItem( newScaleBar );
      emit composerScaleBarAdded( newScaleBar );
      scene()->clearSelection();
      newScaleBar->setSelected( true );
      emit selectedItemChanged( newScaleBar );
      emit actionFinished();
    }
    break;

    case AddLegend:
    {
      QgsComposerLegend* newLegend = new QgsComposerLegend( composition() );
      scene()->addItem( newLegend );
      emit composerLegendAdded( newLegend );
      scene()->clearSelection();
      newLegend->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), newLegend->rect().width(), newLegend->rect().height() ) );
      newLegend->setSelected( true );
      emit selectedItemChanged( newLegend );
      emit actionFinished();
      break;
    }
    case AddPicture:
    {
      QgsComposerPicture* newPicture = new QgsComposerPicture( composition() );
      scene()->addItem( newPicture );
      emit composerPictureAdded( newPicture );
      scene()->clearSelection();
      newPicture->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 30, 30 ) );
      newPicture->setSelected( true );
      emit selectedItemChanged( newPicture );
      emit actionFinished();
    }

    default:
      break;
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
        QgsComposerMap* composerMap = dynamic_cast<QgsComposerMap*>( mMoveContentItem );
        if ( composerMap )
        {
          composerMap->setOffset( 0, 0 );
        }

        double moveX = scenePoint.x() - mMoveContentStartPos.x();
        double moveY = scenePoint.y() - mMoveContentStartPos.y();
        mMoveContentItem->moveContent( -moveX, -moveY );
        mMoveContentItem = 0;
      }
      break;
    }

    case AddMap:
    {
      if ( !mRubberBandItem || mRubberBandItem->rect().width() < 0.1 || mRubberBandItem->rect().width() < 0.1 )
      {
        scene()->removeItem( mRubberBandItem );
        delete mRubberBandItem;
        return;
      }

      QgsComposerMap* composerMap = new QgsComposerMap( composition(), mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() );
      composerMap->setPreviewMode( QgsComposerMap::Rectangle );

      emit composerMapAdded( composerMap );
      scene()->addItem( composerMap );
      scene()->clearSelection();

      scene()->removeItem( mRubberBandItem );
      delete mRubberBandItem;

      composerMap->setSelected( true );
      emit selectedItemChanged( composerMap );
      emit actionFinished();
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
    double newWidth, newHeight; //for rubber band

    switch ( mCurrentTool )
    {
      case Select:
        QGraphicsView::mouseMoveEvent( e );
        break;

      case AddMap:
        //adjust rubber band item
        newWidth = scenePoint.x() - mRubberBandItem->transform().dx();
        newHeight = scenePoint.y() - mRubberBandItem->transform().dy();
        mRubberBandItem->setRect( 0, 0, newWidth, newHeight );
        break;

      case MoveItemContent:
      {
        //update map preview if composer map
        QgsComposerMap* composerMap = dynamic_cast<QgsComposerMap*>( mMoveContentItem );
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

  //delete selected items
  if ( e->key() == Qt::Key_Delete )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      composition()->removeItem( *itemIt );
      delete( *itemIt );
      emit itemRemoved( *itemIt );
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
      theItem->zoomContent( event->delta(), itemPoint.x(), itemPoint.y() );
    }
  }
}

void QgsComposerView::setComposition( QgsComposition* c )
{
  setScene( c );
}

QgsComposition* QgsComposerView::composition()
{
  if ( scene() )
  {
    QgsComposition* c = dynamic_cast<QgsComposition*>( scene() );
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
  //connect signal/slot to let item group tell if child items get removed
  connect( itemGroup, SIGNAL( childItemDeleted( QgsComposerItem* ) ), this, SLOT( sendItemRemovedSignal( QgsComposerItem* ) ) );

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
    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup*>( *itemIter );
    if ( itemGroup )
    {
      itemGroup->removeItems();
      composition()->removeItem( *itemIter );
      delete( *itemIter );
      emit itemRemoved( *itemIter );
    }
  }

}

void QgsComposerView::sendItemRemovedSignal( QgsComposerItem* item )
{
  emit itemRemoved( item );
}

