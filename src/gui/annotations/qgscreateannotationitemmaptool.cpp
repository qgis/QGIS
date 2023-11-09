/***************************************************************************
                             qgscreateannotationitemmaptool.cpp
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

#include "qgscreateannotationitemmaptool.h"
#include "qgsmapcanvas.h"
#include "qgsannotationlayer.h"
#include "qgsannotationitem.h"

QgsCreateAnnotationItemMapToolHandler::QgsCreateAnnotationItemMapToolHandler( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, QObject *parent )
  : QObject( parent )
  , mMapCanvas( canvas )
  , mCadDockWidget( cadDockWidget )
{

}

QgsAnnotationItem *QgsCreateAnnotationItemMapToolHandler::takeCreatedItem()
{
  return mCreatedItem.release();
}

QgsCreateAnnotationItemMapToolHandler::~QgsCreateAnnotationItemMapToolHandler() = default;

QgsAnnotationLayer *QgsCreateAnnotationItemMapToolHandler::targetLayer()
{
  if ( QgsAnnotationLayer *res = qobject_cast< QgsAnnotationLayer * >( mMapCanvas->currentLayer() ) )
    return res;
  else
    return QgsProject::instance()->mainAnnotationLayer();
}

void QgsCreateAnnotationItemMapToolHandler::pushCreatedItem( QgsAnnotationItem *item )
{
  mCreatedItem.reset( item );
  emit itemCreated();
}
