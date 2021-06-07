/***************************************************************************
    qgsmaptoolemitpoint.cpp  -  map tool that emits a signal on click
    ---------------------
    begin                : June 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/


#include "qgsmaptoolemitpoint.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"



QgsMapToolEmitPoint::QgsMapToolEmitPoint( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
}

void QgsMapToolEmitPoint::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapToolEmitPoint::canvasPressEvent( QgsMapMouseEvent *e )
{
  QgsPointXY pnt = toMapCoordinates( e->pos() );
  emit canvasClicked( pnt, e->button() );
}

void QgsMapToolEmitPoint::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}
