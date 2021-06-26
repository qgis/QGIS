/***************************************************************************
                         qgsrasterrendererwidget.cpp
                         ---------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererwidget.h"
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

