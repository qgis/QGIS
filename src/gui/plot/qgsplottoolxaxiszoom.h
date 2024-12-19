/***************************************************************************
                          qgsplottoolxaxiszoom.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLOTTOOLXAXISZOOM_H
#define QGSPLOTTOOLXAXISZOOM_H

#include "qgsplottoolzoom.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

// we probably want to generalize this in future to allow for y/other axis constrained
// zooms, so let's not get locked to stable api...
#define SIP_NO_FILE

class QgsPlotRectangularRubberBand;
class QgsElevationProfileCanvas;

/**
 * \ingroup gui
 * \brief Plot tool for zooming into and out of the plot's x-axis only.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotToolXAxisZoom : public QgsPlotToolZoom
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotToolXAxisZoom, with the associated \a canvas.
     */
    QgsPlotToolXAxisZoom( QgsElevationProfileCanvas *canvas SIP_TRANSFERTHIS );
    ~QgsPlotToolXAxisZoom() override;

  protected:
    QPointF constrainStartPoint( QPointF scenePoint ) const override;
    QPointF constrainMovePoint( QPointF scenePoint ) const override;
    QRectF constrainBounds( const QRectF &sceneBounds ) const override;
    void zoomOutClickOn( QPointF scenePoint ) override;
    void zoomInClickOn( QPointF scenePoint ) override;
  private:
    QgsElevationProfileCanvas *mElevationCanvas = nullptr;
};

#endif // QGSPLOTTOOLXAXISZOOM_H
