/***************************************************************************
    qgsmaptoolrectangleextent.h  -  map tool for adding rectangle
    from extent
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

#ifndef QGSMAPTOOLRECTANGLEEXTENT_H
#define QGSMAPTOOLRECTANGLEEXTENT_H

#include "qgsmaptooladdrectangle.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolRectangleExtent: public QgsMapToolAddRectangle
{
    Q_OBJECT

  public:
    QgsMapToolRectangleExtent( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
};

#endif // QGSMAPTOOLRECTANGLEEXTENT_H
