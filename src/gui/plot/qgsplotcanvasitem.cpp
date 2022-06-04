/***************************************************************************
                          qgsplotcanvasitem.cpp
                          ------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsplotcanvasitem.h"
#include "qgsplotcanvas.h"

QgsPlotCanvasItem::QgsPlotCanvasItem( QgsPlotCanvas *canvas )
  : mCanvas( canvas )
{
  Q_ASSERT( mCanvas && mCanvas->scene() );
  mCanvas->scene()->addItem( this );
}

QgsPlotCanvasItem::~QgsPlotCanvasItem() = default;

void QgsPlotCanvasItem::paint( QPainter *painter,
                               const QStyleOptionGraphicsItem *,
                               QWidget * )
{
  paint( painter );
}
