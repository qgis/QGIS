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

#include <QDialog>

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
#include "qgsexception.h"
#include "qgsannotationmanager.h"
#include "qgsmapmouseevent.h"


QgsMapToolAnnotation::QgsMapToolAnnotation( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mCursor = QCursor( Qt::ArrowCursor );
}

QDialog *QgsMapToolAnnotation::createItemEditor( QgsMapCanvasAnnotationItem *item )
{
  if ( !item || !item->annotation() )
  {
    return nullptr;
  }

  QgsAnnotation *annotation = item->annotation();

  if ( qobject_cast<QgsTextAnnotation *>( annotation ) )
  {
    return new QgsTextAnnotationDialog( item );
  }
  else if ( qobject_cast<QgsFormAnnotation *>( annotation ) )
  {
    return new QgsFormAnnotationDialog( item );
  }
  else if ( qobject_cast<QgsHtmlAnnotation *>( annotation ) )
  {
    return new QgsHtmlAnnotationDialog( item );
  }
  else if ( qobject_cast<QgsSvgAnnotation *>( annotation ) )
  {
    return new QgsSvgAnnotationDialog( item );
  }

  return nullptr;
}

void QgsMapToolAnnotation::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )

  mCurrentMoveAction = QgsMapCanvasAnnotationItem::NoAction;
  mCanvas->setCursor( mCursor );
}

void QgsMapToolAnnotation::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas || !( e->buttons() & Qt::LeftButton ) )
  {
    return;
  }

  mLastMousePosition = e->pos();

  QgsMapCanvasAnnotationItem *item = selectedItem();
  if ( item )
  {
    mCurrentMoveAction = item->moveActionForPosition( e->pos() );
    if ( mCurrentMoveAction != QgsMapCanvasAnnotationItem::NoAction )
    {
      return;
    }
  }

  if ( !item || mCurrentMoveAction == QgsMapCanvasAnnotationItem::NoAction )
  {
    //select a new item if there is one at this position
    mCanvas->scene()->clearSelection();
    QgsMapCanvasAnnotationItem *existingItem = itemAtPos( e->pos() );
    if ( existingItem )
    {
      existingItem->setSelected( true );
    }
    else
    {
      //otherwise create new one
      QgsAnnotation *annotation = createItem();
      if ( annotation )
      {
        const QgsPointXY mapPos = transformCanvasToAnnotation( toMapCoordinates( e->pos() ), annotation );
        annotation->setMapPosition( mapPos );
        annotation->setMapPositionCrs( mCanvas->mapSettings().destinationCrs() );
        annotation->setRelativePosition( QPointF( e->pos().x() / mCanvas->width(),
                                         e->pos().y() / mCanvas->height() ) );
        annotation->setFrameSizeMm( QSizeF( 50, 25 ) );

        QgsProject::instance()->annotationManager()->addAnnotation( annotation );

        // select newly added item
        const auto constItems = mCanvas->items();
        for ( QGraphicsItem *item : constItems )
        {
          if ( QgsMapCanvasAnnotationItem *annotationItem = dynamic_cast< QgsMapCanvasAnnotationItem * >( item ) )
          {
            if ( annotationItem->annotation() == annotation )
            {
              annotationItem->setSelected( true );
              break;
            }
          }
        }
      }
    }
  }
}

void QgsMapToolAnnotation::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_T && e->modifiers() == Qt::ControlModifier )
  {
    toggleTextItemVisibilities();
  }

  QgsMapCanvasAnnotationItem *item = selectedItem();
  if ( item )
  {
    if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
    {
      const QCursor neutralCursor( item->cursorShapeForAction( QgsMapCanvasAnnotationItem::NoAction ) );
      QgsProject::instance()->annotationManager()->removeAnnotation( item->annotation() );
      if ( mCanvas )
      {
        mCanvas->setCursor( neutralCursor );
        e->ignore();
      }
    }
  }
}

void QgsMapToolAnnotation::canvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsMapCanvasAnnotationItem *item = selectedItem();
  if ( !item )
    return;

  QgsAnnotation *annotation = item->annotation();
  if ( !annotation )
    return;

  if ( e->buttons() & Qt::LeftButton )
  {
    if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::MoveMapPosition )
    {
      const QgsPointXY mapPos = transformCanvasToAnnotation( e->snapPoint(), annotation );
      annotation->setMapPosition( mapPos );
      annotation->setRelativePosition( QPointF( e->pos().x() / mCanvas->width(),
                                       e->pos().y() / mCanvas->height() ) );
      item->update();
      QgsProject::instance()->setDirty( true );
    }
    else if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::MoveFramePosition )
    {
      const QPointF newCanvasPos = item->pos() + ( e->pos() - mLastMousePosition );
      if ( annotation->hasFixedMapPosition() )
      {
        const double pixelToMmScale = 25.4 / mCanvas->logicalDpiX();
        const double deltaX = pixelToMmScale * ( e->pos().x() - mLastMousePosition.x() );
        const double deltaY = pixelToMmScale * ( e->pos().y() - mLastMousePosition.y() );
        annotation->setFrameOffsetFromReferencePointMm( QPointF( annotation->frameOffsetFromReferencePointMm().x() + deltaX,
            annotation->frameOffsetFromReferencePointMm().y() + deltaY ) );
        annotation->setRelativePosition( QPointF( newCanvasPos.x() / mCanvas->width(),
                                         newCanvasPos.y() / mCanvas->height() ) );
      }
      else
      {
        const QgsPointXY mapPos = transformCanvasToAnnotation( toMapCoordinates( newCanvasPos.toPoint() ), annotation );
        annotation->setMapPosition( mapPos );
        annotation->setRelativePosition( QPointF( newCanvasPos.x() / mCanvas->width(),
                                         newCanvasPos.y() / mCanvas->height() ) );
      }
      item->update();
      QgsProject::instance()->setDirty( true );
    }
    else if ( mCurrentMoveAction != QgsMapCanvasAnnotationItem::NoAction )
    {
      //handle the frame resize actions

      const double pixelToMmScale = 25.4 / mCanvas->logicalDpiX();

      const QSizeF size = annotation->frameSizeMm();
      double xmin = annotation->frameOffsetFromReferencePointMm().x();
      double ymin = annotation->frameOffsetFromReferencePointMm().y();
      double xmax = xmin + size.width();
      double ymax = ymin + size.height();
      double relPosX = annotation->relativePosition().x();
      double relPosY = annotation->relativePosition().y();

      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRight ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightUp )
      {
        xmax += pixelToMmScale * ( e->pos().x() - mLastMousePosition.x() );
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeft ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftUp )
      {
        xmin += pixelToMmScale * ( e->pos().x() - mLastMousePosition.x() );
        relPosX = ( relPosX * mCanvas->width() + e->pos().x() - mLastMousePosition.x() ) / static_cast<double>( mCanvas->width() );
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameUp ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftUp ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightUp )
      {
        ymin += pixelToMmScale * ( e->pos().y() - mLastMousePosition.y() );
        relPosY = ( relPosY * mCanvas->height() + e->pos().y() - mLastMousePosition.y() ) / static_cast<double>( mCanvas->height() );
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftDown ||
           mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightDown )
      {
        ymax += pixelToMmScale * ( e->pos().y() - mLastMousePosition.y() );
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

      annotation->setFrameOffsetFromReferencePointMm( QPointF( xmin, ymin ) );
      annotation->setFrameSizeMm( QSizeF( xmax - xmin, ymax - ymin ) );
      annotation->setRelativePosition( QPointF( relPosX, relPosY ) );
      item->update();
      QgsProject::instance()->setDirty( true );
    }
  }
  else if ( item )
  {
    const QgsMapCanvasAnnotationItem::MouseMoveAction moveAction = item->moveActionForPosition( e->pos() );
    if ( mCanvas )
    {
      mCanvas->setCursor( QCursor( item->cursorShapeForAction( moveAction ) ) );
    }
  }
  mLastMousePosition = e->pos();
}

void QgsMapToolAnnotation::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  QgsMapCanvasAnnotationItem *item = itemAtPos( e->pos() );
  if ( !item )
  {
    return;
  }
  QDialog *itemEditor = createItemEditor( item );
  if ( itemEditor )
  {
    if ( itemEditor->exec() )
      QgsProject::instance()->setDirty( true );
    delete itemEditor;
  }
}

QgsMapCanvasAnnotationItem *QgsMapToolAnnotation::itemAtPos( QPointF pos ) const
{
  if ( !mCanvas )
  {
    return nullptr;
  }

  QList<QGraphicsItem *> graphicItems = mCanvas->items( pos.toPoint() );
  QList<QGraphicsItem *>::iterator gIt = graphicItems.begin();
  for ( ; gIt != graphicItems.end(); ++gIt )
  {
    QgsMapCanvasAnnotationItem *annotationItem = dynamic_cast<QgsMapCanvasAnnotationItem *>( *gIt );
    if ( annotationItem )
    {
      return annotationItem;
    }
  }
  return nullptr;
}

QgsMapCanvasAnnotationItem *QgsMapToolAnnotation::selectedItem() const
{
  if ( !mCanvas || !mCanvas->scene() )
  {
    return nullptr;
  }
  QList<QGraphicsItem *> gItemList = mCanvas->scene()->selectedItems();
  QList<QGraphicsItem *>::iterator it = gItemList.begin();
  for ( ; it != gItemList.end(); ++it )
  {
    QgsMapCanvasAnnotationItem *aItem = dynamic_cast<QgsMapCanvasAnnotationItem *>( *it );
    if ( aItem )
    {
      return aItem;
    }
  }
  return nullptr;
}

QList<QgsMapCanvasAnnotationItem *> QgsMapToolAnnotation::annotationItems() const
{
  if ( !mCanvas )
  {
    return QList<QgsMapCanvasAnnotationItem *>();
  }
  else
  {
    return mCanvas->annotationItems();
  }
}

void QgsMapToolAnnotation::toggleTextItemVisibilities()
{
  const QList<QgsMapCanvasAnnotationItem *> itemList = annotationItems();
  const auto constItemList = itemList;
  for ( QgsMapCanvasAnnotationItem *item : constItemList )
  {
    QgsTextAnnotation *textItem = qobject_cast<QgsTextAnnotation *>( item->annotation() );
    if ( textItem )
    {
      textItem->setVisible( !textItem->isVisible() );
    }
  }
}

QgsPointXY QgsMapToolAnnotation::transformCanvasToAnnotation( QgsPointXY p, QgsAnnotation *annotation ) const
{
  if ( annotation->mapPositionCrs() != mCanvas->mapSettings().destinationCrs() )
  {
    const QgsCoordinateTransform transform( mCanvas->mapSettings().destinationCrs(), annotation->mapPositionCrs(), QgsProject::instance() );
    try
    {
      p = transform.transform( p );
    }
    catch ( const QgsCsException & )
    {
      // ignore
    }
  }
  return p;
}
