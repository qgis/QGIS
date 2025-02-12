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
  emit pointMoving( match.isValid() ? match.point() : toMapCoordinates( e->pos() ) );
}

void QgsGeorefToolMovePoint::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() & Qt::LeftButton )
  {
    if ( mStartPointMapCoords.isEmpty() )
    {
      emit pointBeginMove( toMapCoordinates( e->pos() ) );
    }
    else
    {
      mSnapIndicator->setMatch( QgsPointLocator::Match() );
      emit pointEndMove( toMapCoordinates( e->pos() ) );
    }
  }
  else if ( e->button() & Qt::RightButton )
  {
    if ( !mStartPointMapCoords.isEmpty() )
    {
      mSnapIndicator->setMatch( QgsPointLocator::Match() );
      emit pointCancelMove( toMapCoordinates( e->pos() ) );
    }
  }
}
