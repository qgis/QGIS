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
#include "qgsmapmouseevent.h"
#include "qgsgeoreftoolmovepoint.h"
#include "qgssnappingutils.h"
#include "moc_qgsgeoreftoolmovepoint.cpp"

QgsGeorefToolMovePoint::QgsGeorefToolMovePoint( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );
}

void QgsGeorefToolMovePoint::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() & Qt::LeftButton )
  {
    if ( mStartPointMapCoords.isEmpty() )
    {
      mStartPointMapCoords = toMapCoordinates( e->pos() );
      emit pointPressed( mStartPointMapCoords );
    }
    else
    {
      mStartPointMapCoords = QgsPointXY();
    }
  }
}

bool QgsGeorefToolMovePoint::isCanvas( QgsMapCanvas *canvas )
{
  return ( mCanvas == canvas );
}

void QgsGeorefToolMovePoint::canvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointLocator::Match match;
  if ( !mStartPointMapCoords.isEmpty() )
  {
    const QgsPointXY pnt = toMapCoordinates( e->pos() );
    match = canvas()->snappingUtils()->snapToMap( pnt );
    mSnapIndicator->setMatch( match );
  }
  emit pointMoved( match.isValid() ? match.point() : toMapCoordinates( e->pos() ) );
}

void QgsGeorefToolMovePoint::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( mStartPointMapCoords.isEmpty() )
  {
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
    emit pointReleased( toMapCoordinates( e->pos() ) );
  }
}
