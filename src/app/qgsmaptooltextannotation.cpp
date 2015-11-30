/***************************************************************************
                              qgsmaptooltextannotation.cpp
                              -------------------------------
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

#include "qgsmaptooltextannotation.h"
#include "qgstextannotationitem.h"
#include "qgsproject.h"
#include <QMouseEvent>

QgsMapToolTextAnnotation::QgsMapToolTextAnnotation( QgsMapCanvas* canvas ): QgsMapToolAnnotation( canvas )
{

}

QgsMapToolTextAnnotation::~QgsMapToolTextAnnotation()
{

}

QgsAnnotationItem* QgsMapToolTextAnnotation::createItem( QMouseEvent* e )
{
  QgsPoint mapCoord = toMapCoordinates( e->pos() );
  QgsTextAnnotationItem* textItem = new QgsTextAnnotationItem( mCanvas );
  textItem->setMapPosition( toMapCoordinates( e->pos() ) );
  textItem->setFrameSize( QSizeF( 200, 100 ) );
  textItem->setSelected( true );
  QgsProject::instance()->setDirty( true );
  return textItem;
}

