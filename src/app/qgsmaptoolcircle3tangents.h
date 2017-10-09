/***************************************************************************
    qgsmaptoolcircle3tangents.h  -  map tool for adding circle
    from 3 tangents
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

#ifndef QGSMAPTOOLCIRCLE3TANGENTS_H
#define QGSMAPTOOLCIRCLE3TANGENTS_H

#include "qgspointlocator.h"
#include "qgsmaptooladdcircle.h"

class QgsMapToolCircle3Tangents: public QgsMapToolAddCircle
{
    Q_OBJECT

  public:
    QgsMapToolCircle3Tangents( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLCIRCLE3TANGENTS_H
