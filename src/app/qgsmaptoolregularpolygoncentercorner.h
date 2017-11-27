/***************************************************************************
    qgsmaptoolregularpolygoncentercorner.h  -  map tool for adding regular
    polygon from center and a corner
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

#ifndef QGSMAPTOOLREGULARPOLYGONCENTERCORNER_H
#define QGSMAPTOOLREGULARPOLYGONCENTERCORNER_H

#include "qgsmaptooladdregularpolygon.h"

class QgsMapToolRegularPolygonCenterCorner: public QgsMapToolAddRegularPolygon
{
    Q_OBJECT

  public:
    QgsMapToolRegularPolygonCenterCorner( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolRegularPolygonCenterCorner();

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLREGULARPOLYGONCENTERCORNER_H
