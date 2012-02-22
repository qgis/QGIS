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
#include "qgsgeometry.h"
#include "qgssnapper.h"

class QgsVertexMarker;
class QDoubleSpinBox;
class QGraphicsProxyWidget;

class QgsMapToolOffsetCurve: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolOffsetCurve( QgsMapCanvas* canvas );
    ~QgsMapToolOffsetCurve();

    void canvasPressEvent( QMouseEvent * e );
    void canvasReleaseEvent( QMouseEvent * e );
    void canvasMoveEvent( QMouseEvent * e );

  private slots:
    /**Places curve offset to value entered in the spin box*/
    void placeOffsetCurveToValue();

  private:

    /**Rubberband that shows the position of the offset curve*/
    QgsRubberBand* mRubberBand;
    /**Geometry to manipulate*/
    QgsGeometry* mOriginalGeometry;
    /**Geometry after manipulation*/
    QgsGeometry mModifiedGeometry;
    /**ID of manipulated feature*/
    QgsFeatureId mModifiedFeature;
    /**Layer ID of source layer*/
    QString mSourceLayerId;
    /**Internal flag to distinguish move from click*/
    bool mGeometryModified;
    /**Embedded item widget for distance spinbox*/
    QGraphicsProxyWidget* mDistanceItem;
    /**Shows current distance value and allows numerical editing*/
    QDoubleSpinBox* mDistanceSpinBox;
    /**Marker to show the cursor was snapped to another location*/
    QgsVertexMarker* mSnapVertexMarker;
    /**Forces geometry copy (no modification of geometry in current layer)*/
    bool mForceCopy;


    void deleteRubberBandAndGeometry();
    QgsGeometry* createOriginGeometry( QgsVectorLayer* vl, const QgsSnappingResult& sr, QgsFeature& snappedFeature );
    void createDistanceItem();
    void deleteDistanceItem();
    void setOffsetForRubberBand( double offset, bool leftSide );
    /**Creates a linestring from the polygon ring containing the snapped vertex. Caller takes ownership of the created object*/
    QgsGeometry* linestringFromPolygon( QgsGeometry* featureGeom, int vertex );
    /**Sets snapping with default vertex search tolerance to all layers (to vertex and segment)*/
    void configureSnapper( QgsSnapper& s );
};

#endif // QGSMAPTOOLOFFSETCURVE_H
