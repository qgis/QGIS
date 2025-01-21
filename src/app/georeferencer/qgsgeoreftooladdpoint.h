/***************************************************************************
     qgsgeoreftooladdpoint.h
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

#ifndef QGSGEOREFTOOLADDPOINT_H
#define QGSGEOREFTOOLADDPOINT_H


#include "qgsmaptoolcapture.h"

class QgsPointXY;
class QgsMapCanvas;
class QgsAdvancedDigitizingDockWidget;

class QgsGeorefToolAddPoint : public QgsMapToolCapture
{
    Q_OBJECT

  public:
    explicit QgsGeorefToolAddPoint( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *advancedDigitizingDockWidget );

    void pointCaptured( const QgsPoint &point );

  signals:
    void showCoordDialog( const QgsPointXY &sourceCoordinates );
};

#endif // QGSGEOREFTOOLADDPOINT_H
