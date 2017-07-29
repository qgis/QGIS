#ifndef QGSMAPTOOLSQUARECENTER_H
#define QGSMAPTOOLSQUARECENTER_H

/***************************************************************************
    qgsmaptoolsquarecenter.h  -  map tool for adding square
    from center and a point
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

#include "qgsmaptooladdregularpolygon.h"

class QgsMapToolSquareCenter: public QgsMapToolAddRegularPolygon
{
    Q_OBJECT

  public:
    QgsMapToolSquareCenter( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolSquareCenter();

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLSQUARECENTER_H
