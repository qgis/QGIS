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
#include "qgis_app.h"

class QgsVertexMarker;
class QgsDoubleSpinBox;
class QGraphicsProxyWidget;

class APP_EXPORT QgsMapToolOffsetCurve: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolOffsetCurve( QgsMapCanvas *canvas );
    ~QgsMapToolOffsetCurve() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

  private slots:
    //! Places curve offset to value entered in the spin box
    void placeOffsetCurveToValue();

    //! Apply the offset either from the spin box or from the mouse event
    void applyOffset( bool forceCopy = false );

  private:
    //! Rubberband that shows the position of the offset curve
    QgsRubberBand *mRubberBand = nullptr;
    //! Geometry to manipulate
    QgsGeometry mOriginalGeometry;
    //! Geometry being manipulated
    QgsGeometry mManipulatedGeometry;
    //! Geometry after manipulation
    QgsGeometry mModifiedGeometry;
    //! ID of manipulated feature
    QgsFeatureId mModifiedFeature = -1;
    //! Layer ID of source layer
    QString mSourceLayerId;
    //! Internal flag to distinguish move from click
    bool mGeometryModified = false;
    //! Shows current distance value and allows numerical editing
    QgsDoubleSpinBox *mDistanceWidget = nullptr;
    //! Marker to show the cursor was snapped to another location
    QgsVertexMarker *mSnapVertexMarker = nullptr;
    //! Forces geometry copy (no modification of geometry in current layer)
    bool mCtrlWasHeldOnFeatureSelection = false;
    bool mMultiPartGeometry = false;
    int mModifiedPart = 0;

    void prepareGeometry( QgsVectorLayer *vl, const QgsPointLocator::Match &match, QgsFeature &snappedFeature );

    void deleteRubberBandAndGeometry();
    void createDistanceWidget();
    void deleteDistanceWidget();
    void setOffsetForRubberBand( double offset );
    //! Creates a linestring from the polygon ring containing the snapped vertex. Caller takes ownership of the created object
    QgsGeometry linestringFromPolygon( const QgsGeometry &featureGeom, int vertex );
    //! Returns a single line from a multiline (or does nothing if geometry is already a single line). Deletes the input geometry
    QgsGeometry convertToSingleLine( const QgsGeometry &geom, int vertex );
};

#endif // QGSMAPTOOLOFFSETCURVE_H
