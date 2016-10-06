/***************************************************************************
    qgsmaptoolcircularstringcurvepoint.h  -  map tool for adding circular
    strings by start / curve / endpoint
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCIRCULARSTRINGCURVEPOINT_H
#define QGSMAPTOOLCIRCULARSTRINGCURVEPOINT_H

#include "qgsmaptooladdcircularstring.h"

class QgsMapToolCircularStringCurvePoint: public QgsMapToolAddCircularString
{
    Q_OBJECT

  public:
    QgsMapToolCircularStringCurvePoint( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolCircularStringCurvePoint();

    void cadCanvasReleaseEvent( QgsMapMouseEvent* e ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent* e ) override;
};

#endif // QGSMAPTOOLCIRCULARSTRINGCURVEPOINT_H
