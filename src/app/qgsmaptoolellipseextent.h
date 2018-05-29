/***************************************************************************
    qgmaptoolellipseextent.h  -  map tool for adding ellipse
    from extent
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

#ifndef QGSMAPTOOLELLIPSEEXTENT_H
#define QGSMAPTOOLELLIPSEEXTENT_H

#include "qgsmaptooladdellipse.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolEllipseExtent: public QgsMapToolAddEllipse
{
    Q_OBJECT

  public:
    QgsMapToolEllipseExtent( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLELLIPSEEXTENT_H
