/***************************************************************************
                         qgsrasterrendererwidget.cpp
                         ---------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererwidget.h"
#include "moc_qgsrasterrendererwidget.cpp"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsmapcanvas.h"

void QgsRasterRendererWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

QgsMapCanvas *QgsRasterRendererWidget::mapCanvas()
{
  return mCanvas;
}
