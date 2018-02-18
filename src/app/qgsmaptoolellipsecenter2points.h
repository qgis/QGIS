/***************************************************************************
    qgsmaptoolellipsecenter2points.h  -  map tool for adding ellipse
    from center and 2 points
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLELLIPSECENTER2POINTS_H
#define QGSMAPTOOLELLIPSECENTER2POINTS_H

#include "qgsmaptooladdellipse.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolEllipseCenter2Points: public QgsMapToolAddEllipse
{
    Q_OBJECT

  public:
    QgsMapToolEllipseCenter2Points( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLELLIPSECENTER2POINTS_H
