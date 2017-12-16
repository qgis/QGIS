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

#ifndef QGSMAPTOOLSELECTRADIUS_H
#define QGSMAPTOOLSELECTRADIUS_H


#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgis_app.h"

class QgsMapCanvas;
class QgsRubberBand;


class APP_EXPORT QgsMapToolSelectRadius : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectRadius( QgsMapCanvas *canvas );

    ~QgsMapToolSelectRadius() override;

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse press event
    void canvasPressEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse release event
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

  private:

    //! sets the rubber band to a circle approximated using 40 segments.
    // The radius center point is defined in the canvasPressEvent event.
    void setRadiusRubberBand( QgsPointXY &radiusEdge );

    //! used for storing all of the maps point for the polygon
    QgsRubberBand *mRubberBand = nullptr;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    bool mDragging;

    QColor mFillColor;

    QColor mStrokeColor;
};

#endif
