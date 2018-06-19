/***************************************************************************
    qgsmaptoolregularpolygoncenterpoint.h  -  map tool for adding regular
    polygon from center and a point
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

#ifndef QGSMAPTOOLREGULARPOLYGONCENTERPOINT_H
#define QGSMAPTOOLREGULARPOLYGONCENTERPOINT_H

#include "qgsmaptooladdregularpolygon.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolRegularPolygonCenterPoint: public QgsMapToolAddRegularPolygon
{
    Q_OBJECT

  public:
    QgsMapToolRegularPolygonCenterPoint( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolRegularPolygonCenterPoint() override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLREGULARPOLYGONCENTERPOINT_H
