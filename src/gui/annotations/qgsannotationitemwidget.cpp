/***************************************************************************
                             qgsannotationitemwidget.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitemwidget.h"
#include "moc_qgsannotationitemwidget.cpp"
#include "qgsmapcanvas.h"
#include "qgsrendereditemdetails.h"
#include "qgsrendereditemresults.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgsannotationlayer.h"

QgsAnnotationItemBaseWidget::QgsAnnotationItemBaseWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
}

bool QgsAnnotationItemBaseWidget::setItem( QgsAnnotationItem *item )
{
  return setNewItem( item );
}

void QgsAnnotationItemBaseWidget::setLayer( QgsAnnotationLayer *layer )
{
  mLayer = layer;
}

QgsAnnotationLayer *QgsAnnotationItemBaseWidget::layer()
{
  return mLayer;
}

void QgsAnnotationItemBaseWidget::setItemId( const QString &id )
{
  mItemId = id;
}

QString QgsAnnotationItemBaseWidget::itemId() const
{
  return mItemId;
}

void QgsAnnotationItemBaseWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsAnnotationItemBaseWidget::context() const
{
  return mContext;
}

void QgsAnnotationItemBaseWidget::focusDefaultWidget()
{
}

bool QgsAnnotationItemBaseWidget::setNewItem( QgsAnnotationItem * )
{
  return false;
}

const QgsRenderedAnnotationItemDetails *QgsAnnotationItemBaseWidget::renderedItemDetails()
{
  QgsMapCanvas *canvas = mContext.mapCanvas();
  if ( !canvas )
    return nullptr;

  QString layerId;
  if ( const QgsAnnotationLayer *layer = mLayer.data() )
  {
    layerId = layer->id();
  }
  if ( layerId.isEmpty() )
    return nullptr;

  const QgsRenderedItemResults *renderedItemResults = canvas->renderedItemResults( false );
  if ( !renderedItemResults )
  {
    return nullptr;
  }

  const QList<QgsRenderedItemDetails *> items = renderedItemResults->renderedItems();
  const QString annotationId = mItemId;
  auto it = std::find_if( items.begin(), items.end(), [layerId, annotationId]( const QgsRenderedItemDetails *item ) {
    if ( const QgsRenderedAnnotationItemDetails *annotationItem = dynamic_cast<const QgsRenderedAnnotationItemDetails *>( item ) )
    {
      if ( annotationItem->itemId() == annotationId && annotationItem->layerId() == layerId )
        return true;
    }
    return false;
  } );
  if ( it != items.end() )
  {
    return dynamic_cast<const QgsRenderedAnnotationItemDetails *>( *it );
  }
  return nullptr;
}
