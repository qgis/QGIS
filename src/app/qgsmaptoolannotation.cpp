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
#include <QMenu>

#include "qgsmaptoolannotation.h"
#include "moc_qgsmaptoolannotation.cpp"
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
#include "qgsapplication.h"


QgsMapToolAnnotation::QgsMapToolAnnotation( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mCursor = QgsApplication::getThemeCursor( QgsApplication::CapturePoint );
}

QgsMapTool::Flags QgsMapToolAnnotation::flags() const
{
  return QgsMapTool::ShowContextMenu;
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
}

void QgsMapToolAnnotation::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas || !( e->buttons() & Qt::LeftButton ) )
  {
    return;
  }

  mLastMousePosition = e->pos();

  // Check if we clicked on an existing item
  QgsMapCanvasAnnotationItem *item = itemAtPos( e->pos() );
  if ( item )
  {
    mCurrentMoveAction = item->moveActionForPosition( e->pos() );
    if ( !item->isSelected() )
    {
      mCanvas->scene()->clearSelection();
      item->setSelected( true );
    }
    return;
  }

  // Otherwise create new one
  mCanvas->scene()->clearSelection();

  QgsAnnotation *annotation = createItem();
  if ( annotation )
  {
    const QgsPointXY mapPos = transformCanvasToAnnotation( toMapCoordinates( e->pos() ), annotation );
    annotation->setMapPosition( mapPos );
    annotation->setMapPositionCrs( mCanvas->mapSettings().destinationCrs() );
    annotation->setRelativePosition( QPointF(
      static_cast<double>( e->pos().x() ) / mCanvas->width(),
      static_cast<double>( e->pos().y() ) / mCanvas->height()
    ) );
    annotation->setFrameSizeMm( QSizeF( 50, 25 ) );

    QgsProject::instance()->annotationManager()->addAnnotation( annotation );

    // select newly added item
    const auto constItems = mCanvas->items();
    for ( QGraphicsItem *item : constItems )
    {
      if ( QgsMapCanvasAnnotationItem *annotationItem = dynamic_cast<QgsMapCanvasAnnotationItem *>( item ) )
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

void QgsMapToolAnnotation::keyPressEvent( QKeyEvent *e )
{
  QgsMapCanvasAnnotationItem *item = selectedItem();
  if ( item )
  {
    if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
    {
      QgsProject::instance()->annotationManager()->removeAnnotation( item->annotation() );
      if ( mCanvas )
      {
        mCanvas->setCursor( mCursor );
        e->ignore();
      }
    }
  }
}

bool QgsMapToolAnnotation::populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event )
{
  // Display context menu for right click (with edit and delete actions)
  QgsMapCanvasAnnotationItem *existingItem = itemAtPos( event->pos() );
  if ( !existingItem )
  {
    return true;
  }
  menu->addSeparator();
  menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ), tr( "Edit" ), this, [this, existingItem]() {
    QDialog *dialog = createItemEditor( existingItem );
    if ( dialog )
    {
      dialog->exec();
    }
  } );
  menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ), tr( "Delete" ), this, [existingItem]() {
    QgsProject::instance()->annotationManager()->removeAnnotation( existingItem->annotation() );
  } );

  return true;
}

void QgsMapToolAnnotation::canvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsMapCanvasAnnotationItem *item = selectedItem();

  QgsAnnotation *annotation = nullptr;
  if ( item )
    annotation = item->annotation();

  if ( annotation && ( e->buttons() & Qt::LeftButton ) )
  {
    if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::MoveMapPosition )
    {
      const QgsPointXY mapPos = transformCanvasToAnnotation( e->snapPoint(), annotation );
      annotation->setMapPosition( mapPos );
      annotation->setRelativePosition( QPointF( e->pos().x() / mCanvas->width(), e->pos().y() / mCanvas->height() ) );
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
        annotation->setFrameOffsetFromReferencePointMm( QPointF( annotation->frameOffsetFromReferencePointMm().x() + deltaX, annotation->frameOffsetFromReferencePointMm().y() + deltaY ) );
        annotation->setRelativePosition( QPointF( newCanvasPos.x() / mCanvas->width(), newCanvasPos.y() / mCanvas->height() ) );
      }
      else
      {
        const QgsPointXY mapPos = transformCanvasToAnnotation( toMapCoordinates( newCanvasPos.toPoint() ), annotation );
        annotation->setMapPosition( mapPos );
        annotation->setRelativePosition( QPointF( newCanvasPos.x() / mCanvas->width(), newCanvasPos.y() / mCanvas->height() ) );
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

      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRight || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightDown || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightUp )
      {
        xmax += pixelToMmScale * ( e->pos().x() - mLastMousePosition.x() );
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeft || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftDown || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftUp )
      {
        xmin += pixelToMmScale * ( e->pos().x() - mLastMousePosition.x() );
        relPosX = ( relPosX * mCanvas->width() + e->pos().x() - mLastMousePosition.x() ) / static_cast<double>( mCanvas->width() );
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameUp || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftUp || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightUp )
      {
        ymin += pixelToMmScale * ( e->pos().y() - mLastMousePosition.y() );
        relPosY = ( relPosY * mCanvas->height() + e->pos().y() - mLastMousePosition.y() ) / static_cast<double>( mCanvas->height() );
      }
      if ( mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameDown || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameLeftDown || mCurrentMoveAction == QgsMapCanvasAnnotationItem::ResizeFrameRightDown )
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
  else if ( mCanvas )
  {
    if ( ( item = itemAtPos( e->pos() ) ) )
    {
      mCanvas->setCursor( QCursor( item->cursorShapeForAction( item->moveActionForPosition( e->pos() ) ) ) );
    }
    else
    {
      mCanvas->setCursor( mCursor );
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
    // Consider only the topmost item that has a move action for the position
    // (i.e. cursor is over the frame or over the anchor point)
    if ( annotationItem && annotationItem->moveActionForPosition( pos ) != QgsMapCanvasAnnotationItem::NoAction )
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
