/***************************************************************************
    qgmaptoolellipsecenterpoint.h  -  map tool for adding ellipse
    from center and a point
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLELLIPSECENTERPOINT_H
#define QGSMAPTOOLELLIPSECENTERPOINT_H

#include "qgsmaptooladdellipse.h"

class QgsMapToolEllipseCenterPoint: public QgsMapToolAddEllipse
{
    Q_OBJECT

  public:
    QgsMapToolEllipseCenterPoint( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLELLIPSECENTERPOINT_H
