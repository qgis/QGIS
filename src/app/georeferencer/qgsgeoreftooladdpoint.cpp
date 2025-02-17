/***************************************************************************
     qgsgeoreftooladdpoint.cpp
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
#include "qgsgeoreftooladdpoint.h"
#include "moc_qgsgeoreftooladdpoint.cpp"
#include "qgsmapmouseevent.h"

QgsGeorefToolAddPoint::QgsGeorefToolAddPoint( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *advancedDigitizingDockWidget )
  : QgsMapToolCapture( canvas, advancedDigitizingDockWidget, QgsMapToolCapture::CaptureMode::CapturePoint )
{
}

void QgsGeorefToolAddPoint::pointCaptured( const QgsPoint &point )
{
  emit showCoordDialog( point );
}
