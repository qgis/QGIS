/***************************************************************************
    qgmaptoolcircle3points.h  -  map tool for adding circle
    from 3 points
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

#ifndef QGSMAPTOOLCIRCLE3POINTS_H
#define QGSMAPTOOLCIRCLE3POINTS_H

#include "qgsmaptooladdcircle.h"

class QgsMapToolCircle3Points: public QgsMapToolAddCircle
{
    Q_OBJECT

  public:
    QgsMapToolCircle3Points( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLCIRCLE3POINTS_H
