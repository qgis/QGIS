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
 * \brief QgsMapToolCaptureLayerGeometry is a base class for map tools digitizing layer geometries
 * This map tool subclass automatically handles intersection avoidance with other layers in the active project whenever a geometry is digitized by the user.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsMapToolCaptureLayerGeometry : public QgsMapToolCapture
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMapToolCaptureLayerGeometry( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
      : QgsMapToolCapture( canvas, cadDockWidget, mode )
    {}

  private:
    void geometryCaptured( const QgsGeometry &geometry ) override;

    /**
     * Called when the geometry is captured
     * A more specific handler is also called afterwards (layerPointCaptured, layerLineCaptured or layerPolygonCaptured)
     */
    virtual void layerGeometryCaptured( const QgsGeometry &geometry ) SIP_FORCE { Q_UNUSED( geometry ) }

    /**
     * Called when a point is captured
     * The generic geometryCaptured() signal will be emitted immediately before this point-specific signal.
     */
    virtual void layerPointCaptured( const QgsPoint &point ) SIP_FORCE { Q_UNUSED( point ) }

    /**
     * Called when a line is captured
     * The generic geometryCaptured() signal will be emitted immediately before this line-specific signal.
     */
    virtual void layerLineCaptured( const QgsCurve *line ) SIP_FORCE { Q_UNUSED( line ) }

    /**
     * Called when a polygon is captured
     * The generic geometryCaptured() signal will be emitted immediately before this polygon-specific signal.
     */
    virtual void layerPolygonCaptured( const QgsCurvePolygon *polygon ) SIP_FORCE { Q_UNUSED( polygon ) }
};

#endif // QGSMAPTOOLCAPTURELAYERGEOMETRY_H
