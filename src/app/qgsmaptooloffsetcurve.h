/***************************************************************************
                              qgsmaptooloffsetcurve.h
    ------------------------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLOFFSETCURVE_H
#define QGSMAPTOOLOFFSETCURVE_H

#include "qgsmaptooledit.h"
class QgsGeometry;

class QgsMapToolOffsetCurve: public QgsMapToolEdit
{
  public:
    QgsMapToolOffsetCurve( QgsMapCanvas* canvas );
    ~QgsMapToolOffsetCurve();

    void canvasPressEvent( QMouseEvent * e );
    void canvasReleaseEvent( QMouseEvent * e );
    void canvasMoveEvent( QMouseEvent * e );

  private:

    /**Rubberband that shows the position of the offset curve*/
    QgsRubberBand* mRubberBand;
    /**Geometry to manipulate*/
    QgsGeometry* mGeometry;

    void deleteRubberBandAndGeometry();
};

#endif // QGSMAPTOOLOFFSETCURVE_H
