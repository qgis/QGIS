/***************************************************************************
                              qgsmaptoolannotation.cpp
                              ----------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolannotation.h"
#include "qgsannotation.h"
#include "qgsformannotationdialog.h"
#include "qgsformannotation.h"
#include "qgshtmlannotation.h"
#include "qgshtmlannotationdialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgstextannotationdialog.h"
#include "qgstextannotation.h"
#include "qgssvgannotationdialog.h"
#include "qgssvgannotation.h"
#include "qgsproject.h"
#include <QDialog>
#include <QMouseEvent>

QgsMapToolAnnotation::QgsMapToolAnnotation( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
  mCursor = QCursor( Qt::ArrowCursor );
}

QgsMapToolAnnotation::~QgsMapToolAnnotation()
{
}

QDialog* QgsMapToolAnnotation::createItemEditor( QgsMapCanvasAnnotationItem *item )
{
  if ( !item || !item->annotation() )
  {
    return nullptr;
  }

  QgsAnnotation* annotation = item->annotation();

  QgsTextAnnotation* tItem = dynamic_cast<QgsTextAnnotation*>( annotation );
  if ( tItem )
  {
    return new QgsTextAnnotationDialog( item );
  }

  QgsFormAnnotation* fItem = dynamic_cast<QgsFormAnnotation*>( annotation );
  if ( fItem )
  {
    return new QgsFormAnnotationDialog( item );
  }

  QgsHtmlAnnotation* hItem = dynamic_cast<QgsHtmlAnnotation*>( annotation );
  if ( hItem )
  {
    return new QgsHtmlAnnotationDialog( item );
  }

  QgsSvgAnnotation* sItem = dynamic_cast<QgsSvgAnnotation*>( annotation );
  if ( sItem )
  {
    return new QgsSvgAnnotationDialog( item );
  }

  return nullptr;
}

void QgsMapToolAnnotation::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );

  mCurrentMoveAction = QgsMapCanvasAnnotationItem::NoAction;
  mCanvas->setCursor( mCursor );
}

void QgsMapToolAnnotation::canvasPressEvent( QgsMapMouseEvent* e )
{
  if ( !mCanvas )
  {
    return;
  }

  mLastMousePosition = e->posF();

  QgsMapCanvasAnnotationItem* item = selectedItem();
  if ( item )
  {
    mCurrentMoveAction = item->moveActionForPosition( e->posF() );
    if ( mCurrentMoveAction != QgsMapCanvasAnnotationItem::NoAction )
    {
      return;
    }
  }

  if ( !item || mCurrentMoveAction == QgsMapCanvasAnnotationItem::NoAction )
  {
    //select a new item if there is one at this position
    mCanvas->scene()->clearSelection();
    QgsMapCanvasAnnotationItem* existingItem = itemAtPos( e->posF() );
    if ( existingItem )
    {
      existingItem->setSelected( true );
    }
    else
    {
      //otherwise create new one
      QgsAnnotation* annotation = createItem();
      if ( annotation )
      {
        annotation->setMapPosition( toMapCoordinates( e->pos() ) );
        annotation->setRelativePosition( QPointF( e->posF().x() / mCanvas->width(),
                                         e->posF().y() / mCanvas->height() ) );
        annotation->setFrameSize( QSizeF( 200, 100 ) );

        QgsMapCanvasAnnotationItem* canvasItem = new QgsMapCanvasAnnotationItem( annotation, mCanvas );
        canvasItem->setSelected( true );
        QgsProject::instance()->setDirty( true );
      }
    }
  }
}

void QgsMapToolAnnotation::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_T && e->modifiers() == Qt::ControlModifier )
  {
    toggleTextItemVisibilities();
  }

  QgsMapCanvasAnnotationItem* item = selectedItem();
  if ( item )
  {
    if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
    {
      if ( mCanvas && mCanvas->scene() )
      {
        QCursor neutralCursor( item->cursorShapeForAction( QgsMapCanvasAnnotationItem::NoAction ) );
        mCanvas->scene()->removeItem( item );
        delete item;
        mCanvas->setCursor( neutralCursor );
        QgsProject::instance()->setDirty( true ); // TODO QGIS3: Rework the whole annotation code to be MVC compliant, see PR #2506

        // Override default shortcut management in MapCanvas
        e->ignore();
      }
    }
  }
}

void QgsMapToolAnnotation::canvasMoveEvent( QgsMapMouseEvent* e )
{
  QgsMapCanvasAnnotationItem* item = selectedItem();
  if ( !item || !item->annotation() )
    return;

  QgsAnnotation* annotation = item->annotation();

  if ( e->buttons() & Qt::LeftButton )
  {
    if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::MoveMapPosition )
    {
      annotation->setMapPosition( toMapCoordinates( e->pos() ) );
      annotation->setRelativePosition( QPointF( e->posF().x() / mCanvas->width(),
                                       e->posF().y() / mCanvas->height() ) );
      item->update();
      QgsProject::instance()->setDirty( true );
    }
    else if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::MoveFramePosition )
    {
      QPointF newCanvasPos = item->pos() + ( e->posF() - mLastMousePosition );
      if ( annotation->hasFixedMapPosition() )
      {
        annotation->setFrameOffsetFromReferencePoint( annotation->frameOffsetFromReferencePoint() + ( e->posF() - mLastMousePosition ) );
        annotation->setRelativePosition( QPointF( newCanvasPos.x() / mCanvas->width(),
                                         newCanvasPos.y() / mCanvas->height() ) );
      }
      else
      {
        annotation->setMapPosition( toMapCoordinates( newCanvasPos.toPoint() ) );
        annotation->setRelativePosition( QPointF( newCanvasPos.x() / mCanvas->width(),
                                         newCanvasPos.y() / mCanvas->height() ) );
      }
      item->update();
      QgsProject::instance()->setDirty( true );
    }
    else if ( mCurrentMoveAction != QgsMapCanvasAnnotationItem::NoAction )
    {
      //handle the frame resize actions
      QSizeF size = annotation->frameSize();
      double xmin = annotation->frameOffsetFromReferencePoint().x();
      double ymin = annotation->frameOffsetFromReferencePoint().y();
      double xmax = xmin + size.width();
      double ymax = ymin + size.height();
      double relPosX = annotation->relativePosition().x();
      double relPosY = annotation->relativePosition().y();

      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRight ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightUp )
      {
        xmax += e->posF().x() - mLastMousePosition.x();
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeft ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftUp )
      {
        xmin += e->posF().x() - mLastMousePosition.x();
        relPosX = ( relPosX * mCanvas->width() + e->posF().x() - mLastMousePosition.x( ) ) / ( double )mCanvas->width();
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameUp ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftUp ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightUp )
      {
        ymin += e->posF().y() - mLastMousePosition.y();
        relPosY = ( relPosY * mCanvas->height() + e->posF().y() - mLastMousePosition.y( ) ) / ( double )mCanvas->height();
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightDown )
      {
        ymax += e->posF().y() - mLastMousePosition.y();
      }

      //switch min / max if necessary
      double tmp;
      if ( xmax < xmin )
      {
        tmp = xmax;
        xmax = xmin;
        xmin = tmp;
      }
      if ( ymax < ymin )
      {
        tmp = ymax;
        ymax = ymin;
        ymin = tmp;
      }

      annotation->setFrameOffsetFromReferencePoint( QPointF( xmin, ymin ) );
      annotation->setFrameSize( QSizeF( xmax - xmin, ymax - ymin ) );
      annotation->setRelativePosition( QPointF( relPosX, relPosY ) );
      item->update();
      QgsProject::instance()->setDirty( true );
    }
  }
  else if ( item )
  {
    QgsMapCanvasAnnotationItem::MouseMoveAction moveAction = item->moveActionForPosition( e->posF() );
    if ( mCanvas )
    {
      mCanvas->setCursor( QCursor( item->cursorShapeForAction( moveAction ) ) );
    }
  }
  mLastMousePosition = e->posF();
}

void QgsMapToolAnnotation::canvasDoubleClickEvent( QgsMapMouseEvent* e )
{
  QgsMapCanvasAnnotationItem* item = itemAtPos( e->posF() );
  if ( !item )
  {
    return;
  }
  QDialog* itemEditor = createItemEditor( item );
  if ( itemEditor )
  {
    if ( itemEditor->exec() )
      QgsProject::instance()->setDirty( true );
    delete itemEditor;
  }
}

QgsMapCanvasAnnotationItem* QgsMapToolAnnotation::itemAtPos( QPointF pos ) const
{
  if ( !mCanvas )
  {
    return nullptr;
  }

  QList<QGraphicsItem *> graphicItems = mCanvas->items( pos.toPoint() );
  QList<QGraphicsItem *>::iterator gIt = graphicItems.begin();
  for ( ; gIt != graphicItems.end(); ++gIt )
  {
    QgsMapCanvasAnnotationItem* annotationItem = dynamic_cast<QgsMapCanvasAnnotationItem*>( *gIt );
    if ( annotationItem )
    {
      return annotationItem;
    }
  }
  return nullptr;
}

QgsMapCanvasAnnotationItem* QgsMapToolAnnotation::selectedItem() const
{
  if ( !mCanvas || !mCanvas->scene() )
  {
    return nullptr;
  }
  QList<QGraphicsItem *> gItemList = mCanvas->scene()->selectedItems();
  QList<QGraphicsItem *>::iterator it = gItemList.begin();
  for ( ; it != gItemList.end(); ++it )
  {
    QgsMapCanvasAnnotationItem* aItem = dynamic_cast<QgsMapCanvasAnnotationItem*>( *it );
    if ( aItem )
    {
      return aItem;
    }
  }
  return nullptr;
}

QList<QgsMapCanvasAnnotationItem*> QgsMapToolAnnotation::annotationItems() const
{
  QList<QgsMapCanvasAnnotationItem*> annotationItemList;
  if ( !mCanvas || !mCanvas->scene() )
  {
    return annotationItemList;
  }

  QList<QGraphicsItem *>  itemList = mCanvas->scene()->items();
  QList<QGraphicsItem *>::iterator it = itemList.begin();
  for ( ; it != itemList.end(); ++it )
  {
    QgsMapCanvasAnnotationItem* aItem = dynamic_cast<QgsMapCanvasAnnotationItem*>( *it );
    if ( aItem )
    {
      annotationItemList.push_back( aItem );
    }
  }

  return annotationItemList;
}

void QgsMapToolAnnotation::toggleTextItemVisibilities()
{
  QList<QgsMapCanvasAnnotationItem*> itemList = annotationItems();
  Q_FOREACH ( QgsMapCanvasAnnotationItem* item, itemList )
  {
    QgsTextAnnotation* textItem = dynamic_cast<QgsTextAnnotation*>( item->annotation() );
    if ( textItem )
    {
      textItem->setVisible( !textItem->isVisible() );
    }
  }
}
