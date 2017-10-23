/***************************************************************************
    qgmaptoolregularpolygon2points.h  -  map tool for adding regular
    polygon from 2 points
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

#ifndef QGSMAPTOOLREGULARPOLYGON2POINTS_H
#define QGSMAPTOOLREGULARPOLYGON2POINTS_H

#include "qgsmaptooladdregularpolygon.h"

class QgsMapToolRegularPolygon2Points: public QgsMapToolAddRegularPolygon
{
    Q_OBJECT

  public:
    QgsMapToolRegularPolygon2Points( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolRegularPolygon2Points();

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLREGULARPOLYGON2POINTS_H
