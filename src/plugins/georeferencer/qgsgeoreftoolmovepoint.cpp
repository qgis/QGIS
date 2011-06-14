/***************************************************************************
     qgsgeoreftoolmovepoint.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapcanvas.h"

#include "qgsgeoreftoolmovepoint.h"

QgsGeorefToolMovePoint::QgsGeorefToolMovePoint( QgsMapCanvas *canvas )
    : QgsMapTool( canvas )
    , mRubberBand( 0 )
{
}

void QgsGeorefToolMovePoint::canvasPressEvent( QMouseEvent *e )
{
  if ( e->button() & Qt::LeftButton )
  {
    mStartPointMapCoords = e->pos();
    emit pointPressed( e->pos() );
  }
}

bool QgsGeorefToolMovePoint::isCanvas( QgsMapCanvas *canvas )
{
  return ( mCanvas == canvas );
}

void QgsGeorefToolMovePoint::canvasMoveEvent( QMouseEvent *e )
{
  emit pointMoved( e->pos() );
}

void QgsGeorefToolMovePoint::canvasReleaseEvent( QMouseEvent *e )
{
  emit pointReleased( e->pos() );
}
