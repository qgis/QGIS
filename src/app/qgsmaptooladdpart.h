/***************************************************************************
    qgsmaptooladdpart.h  - map tool to add new polygons to multipolygon features
    ---------------------
    begin                : May 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolcapturelayergeometry.h"
#include "qgis_app.h"

class QgsCurvePolygon;

//! A map tool that adds new parts to multipart features
class APP_EXPORT QgsMapToolAddPart : public QgsMapToolCaptureLayerGeometry
{
    Q_OBJECT
  public:
    QgsMapToolAddPart( QgsMapCanvas *canvas );

    QgsMapToolCapture::Capabilities capabilities() const override;
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void activate() override;

  private:

    /**
     * Check if there is any feature selected and the layer supports adding the part
     * Returns a nullptr otherwise
     */
    QgsVectorLayer *getLayerAndCheckSelection();

    void layerPointCaptured( const QgsPoint &point ) override;
    void layerLineCaptured( const QgsCurve *line ) override;
    void layerPolygonCaptured( const QgsCurvePolygon *polygon ) override;

    void finalizeEditCommand( QgsVectorLayer *layer, Qgis::GeometryOperationResult errorCode );
};
