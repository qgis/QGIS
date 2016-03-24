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

QgsGeorefToolDeletePoint::QgsGeorefToolDeletePoint( QgsMapCanvas* canvas )
    : QgsMapToolEmitPoint( canvas )
{
}

// Mouse press event for overriding
void QgsGeorefToolDeletePoint::canvasPressEvent( QgsMapMouseEvent* e )
{
  // Only add point on Qt:LeftButton
  if ( Qt::LeftButton == e->button() )
  {
    emit deleteDataPoint( e->pos() );
  }
}
