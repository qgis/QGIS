/***************************************************************************
                    qgsmaptoolselectannotationmousehandles.cpp
                             -----------------------
    begin                : November 2025
    copyright            : (C) 2025 by Mathieu Pellrin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolselectannotationmousehandles.h"
#include "moc_qgsmaptoolselectannotationmousehandles.cpp"
#include "qgis.h"
#include "qgsgui.h"
#include "qgsmaptoolselectannotation.h"

#include <QGraphicsView>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QWidget>
#include <limits>

///@cond PRIVATE

QgsMapToolSelectAnnotationMouseHandles::QgsMapToolSelectAnnotationMouseHandles( QgsMapToolSelectAnnotation *mapTool, QgsMapCanvas *canvas )
  : QgsGraphicsViewMouseHandles( canvas )
  , mMapTool( mapTool )
  , mCanvas( canvas )
{
  setZValue( 100 );

  connect( mMapTool, &QgsMapToolSelectAnnotation::selectedItemsChanged, this, &QgsMapToolSelectAnnotationMouseHandles::selectionChanged );
  connect( mCanvas, &QgsMapCanvas::extentsChanged, this, [this] { updateHandles(); } );
  mCanvas->scene()->addItem( this );

  setRotationEnabled( true );
}

void QgsMapToolSelectAnnotationMouseHandles::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  paintInternal( painter, true, true, true, option, widget );
}

void QgsMapToolSelectAnnotationMouseHandles::selectionChanged()
{
  updateHandles();
}

void QgsMapToolSelectAnnotationMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  if ( qobject_cast<QgsMapToolSelectAnnotation *>( mCanvas->mapTool() ) )
  {
    mCanvas->viewport()->setCursor( cursor );
  }
}

QList<QGraphicsItem *> QgsMapToolSelectAnnotationMouseHandles::sceneItemsAtPoint( QPointF scenePoint )
{
  QList<QGraphicsItem *> graphicsItems;
  const QList<QgsAnnotationItemRubberBand *> items = mMapTool->selectedItems();
  for ( QgsAnnotationItemRubberBand *item : items )
  {
    if ( item->sceneBoundingRect().contains( scenePoint ) )
    {
      graphicsItems << item;
    }
  }
  return graphicsItems;
}

QList<QGraphicsItem *> QgsMapToolSelectAnnotationMouseHandles::selectedSceneItems( bool ) const
{
  QList<QGraphicsItem *> graphicsItems;
  const QList<QgsAnnotationItemRubberBand *> items = mMapTool->selectedItems();
  for ( QgsAnnotationItemRubberBand *item : items )
  {
    graphicsItems << item;
  }
  return graphicsItems;
}

QRectF QgsMapToolSelectAnnotationMouseHandles::itemRect( QGraphicsItem *item ) const
{
  return item->boundingRect();
}

void QgsMapToolSelectAnnotationMouseHandles::moveItem( QGraphicsItem *item, double deltaX, double deltaY )
{
  qgis::down_cast<QgsAnnotationItemRubberBand *>( item )->attemptMoveBy( deltaX, deltaY );
}

void QgsMapToolSelectAnnotationMouseHandles::rotateItem( QGraphicsItem *item, double deltaDegree, double deltaCenterX, double deltaCenterY )
{
  QgsAnnotationItemRubberBand *annotationItem = qgis::down_cast<QgsAnnotationItemRubberBand *>( item );
  annotationItem->attemptMoveBy( deltaCenterX, deltaCenterY );
  annotationItem->attemptRotateBy( deltaDegree );
}

void QgsMapToolSelectAnnotationMouseHandles::setItemRect( QGraphicsItem *item, QRectF rect )
{
  qgis::down_cast<QgsAnnotationItemRubberBand *>( item )->attemptSetSceneRect( rect );
}

///@endcond PRIVATE
