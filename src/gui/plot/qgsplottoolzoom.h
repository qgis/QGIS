/***************************************************************************
                          qgsplottoolzoom.h
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

#ifndef QGSPLOTTOOLZOOM_H
#define QGSPLOTTOOLZOOM_H

#include "qgsplottool.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsPlotRectangularRubberBand;


/**
 * \ingroup gui
 * \brief Plot tool for zooming into and out of the plot.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotToolZoom : public QgsPlotTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotToolZoom, with the associated \a canvas.
     */
    QgsPlotToolZoom( QgsPlotCanvas *canvas SIP_TRANSFERTHIS );
    ~QgsPlotToolZoom() override;

    void plotPressEvent( QgsPlotMouseEvent *event ) override;
    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void deactivate() override;

  protected:

    /**
     * Applies constraints to the start point of the zoom rubber band.
     *
     * \note Not available in Python bindings
     */
    virtual QPointF constrainStartPoint( QPointF scenePoint ) const SIP_SKIP;

    /**
     * Applies constraints to a move point of the zoom rubber band.
     *
     * \note Not available in Python bindings
     */
    virtual QPointF constrainMovePoint( QPointF scenePoint ) const SIP_SKIP;

    /**
     * Applies constraints to the overall bounds of the rubber band.
     *
     * \note Not available in Python bindings
     */
    virtual QRectF constrainBounds( const QRectF &sceneBounds ) const SIP_SKIP;

    /**
     * Handles a zoom out click on the given point.
     *
     * \note Not available in Python bindings
     */
    virtual void zoomOutClickOn( QPointF scenePoint ) SIP_SKIP;

    /**
     * Handles a zoom out click on the given point.
     *
     * \note Not available in Python bindings
     */
    virtual void zoomInClickOn( QPointF scenePoint ) SIP_SKIP;

    //! Will be TRUE will marquee zoom operation is in progress
    bool mMarqueeZoom = false;

  private:

    //! Start position for mouse press
    QPoint mMousePressStartPos;

    QPointF mRubberBandStartPos;

    //! Rubber band item
    std::unique_ptr< QgsPlotRectangularRubberBand > mRubberBand;

    void startMarqueeZoom( QPointF scenePoint );

};

#endif // QGSPLOTTOOLZOOM_H
