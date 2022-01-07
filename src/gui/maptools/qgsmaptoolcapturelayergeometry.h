/***************************************************************************
    qgsmaptoolcapturelayergeometry.h  -  base class for map tools digitizing layer geometries
    ---------------------
    begin                : January 2022
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCAPTURELAYERGEOMETRY_H
#define QGSMAPTOOLCAPTURELAYERGEOMETRY_H

#include "qgsmaptoolcapture.h"

class QgsAdvancedDigitizingDockWidget;
class QgsMapCanvas;

/**
 * \ingroup gui
 * QgsMapToolCaptureLayerGeometry is a base class for map tools digitizing layer geometries
 * It will implement avoid intersections
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsMapToolCaptureLayerGeometry : public QgsMapToolCapture
{
  public:
    //! Constructor
    QgsMapToolCaptureLayerGeometry( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );

  private:
    void geometryCaptured( const QgsGeometry &geometry ) override;

    /**
     * Called when the geometry is captured
     * A more specific handler is also called afterwards (pointCaptured, lineCaptured or polygonCaptured)
     * \since QGIS 3.24
     */
    virtual void layerGeometryCaptured( const QgsGeometry &geometry ) {Q_UNUSED( geometry )} SIP_FORCE

    /**
     * Called when a point is captured
     * geometryCaptured is called just before
     * \since QGIS 3.24
     */
    virtual void layerPointCaptured( const QgsPoint &point ) {Q_UNUSED( point )} SIP_FORCE

    /**
     * Called when a line is captured
     * geometryCaptured is called just before
     * \since QGIS 3.24
     */
    virtual void layerLineCaptured( const QgsCurve *line ) {Q_UNUSED( line )} SIP_FORCE

    /**
     * Called when a polygon is captured
     * geometryCaptured is called just before
     * \since QGIS 3.24
     */
    virtual void layerPolygonCaptured( const QgsCurvePolygon *polygon ) {Q_UNUSED( polygon )} SIP_FORCE
};

#endif // QGSMAPTOOLCAPTURELAYERGEOMETRY_H
