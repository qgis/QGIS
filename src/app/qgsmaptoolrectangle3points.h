/***************************************************************************
   qgsmaptoolrectangle3points.h  -  map tool for adding rectangle
   from 3 points
   ---------------------
   begin                : September 2017
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

#ifndef QGSMAPTOOLRECTANGLE3POINTS_H
#define QGSMAPTOOLRECTANGLE3POINTS_H

#include "qgsmaptooladdrectangle.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolRectangle3Points: public QgsMapToolAddRectangle
{
    Q_OBJECT

  public:
    enum CreateMode
    {
      DistanceMode,
      ProjectedMode,
    };
    QgsMapToolRectangle3Points( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CreateMode createMode, CaptureMode mode = CaptureLine );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

  private:
    CreateMode mCreateMode;

};

#endif // QGSMAPTOOLRECTANGLE3POINTS_H
