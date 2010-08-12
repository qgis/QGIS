/***************************************************************************
qgsmaptoolselectradius.h  -  map tool for selecting features by radius
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/* $Id$ */

#ifndef QGSMAPTOOLSELECTRADIUS_H
#define QGSMAPTOOLSELECTRADIUS_H


#include "qgsmaptool.h"
#include "qgspoint.h"

class QgsMapCanvas;
class QgsRubberBand;


class QgsMapToolSelectRadius : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectRadius( QgsMapCanvas* canvas );

    virtual ~QgsMapToolSelectRadius();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

  private:

    //! sets the rubber band to a circle approximated using 40 segments.
    // The radius center point is defined in the canvasPressEvent event.
    void setRadiusRubberBand( QgsPoint & radiusEdge );

    //! used for storing all of the maps point for the polygon
    QgsRubberBand* mRubberBand;

    //! Center point for the radius
    QgsPoint mRadiusCenter;

    bool mDragging;
};

#endif
