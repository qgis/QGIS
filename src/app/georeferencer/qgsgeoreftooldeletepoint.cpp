/***************************************************************************
     qgsgeoreftooldeletepoint.cpp
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
#include "qgsgeoreftooldeletepoint.h"
#include "moc_qgsgeoreftooldeletepoint.cpp"
#include "qgsmapmouseevent.h"

QgsGeorefToolDeletePoint::QgsGeorefToolDeletePoint( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
}

void QgsGeorefToolDeletePoint::canvasMoveEvent( QgsMapMouseEvent *e )
{
  emit hoverPoint( toMapCoordinates( e->pos() ) );
}

void QgsGeorefToolDeletePoint::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( Qt::LeftButton == e->button() )
  {
    emit deletePoint( toMapCoordinates( e->pos() ) );
  }
}
