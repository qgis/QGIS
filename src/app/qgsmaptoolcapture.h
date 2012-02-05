/***************************************************************************
    qgsmaptoolcapture.h  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCAPTURE_H
#define QGSMAPTOOLCAPTURE_H

#include "qgsmapcanvassnapper.h"
#include "qgsmaptooledit.h"
#include "qgspoint.h"
#include "qgsgeometry.h"
#include "qgslegend.h"

#include <QPoint>
#include <QList>

class QgsRubberBand;
class QgsVertexMarker;
class QgsMapLayer;
class QgsGeometryValidator;

class QgsMapToolCapture : public QgsMapToolEdit
{
    Q_OBJECT

  public:

    enum CaptureMode
    {
      CaptureNone,
      CapturePoint,
      CaptureLine,
      CapturePolygon
    };

    //! constructor
    QgsMapToolCapture( QgsMapCanvas* canvas, CaptureMode mode = CaptureNone );

    //! destructor
    virtual ~QgsMapToolCapture();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e ) = 0;

    virtual void keyPressEvent( QKeyEvent* e );

    //! Resize rubber band
    virtual void renderComplete();

    //! deactive the tool
    virtual void deactivate();

  public slots:
    void currentLayerChanged( QgsMapLayer *layer );
    void addError( QgsGeometry::Error );
    void validationFinished();

  protected:
    int nextPoint( const QPoint &p, QgsPoint &layerPoint, QgsPoint &mapPoint );

    /**Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     @return 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed*/
    int addVertex( const QPoint& p );

    /**Removes the last vertex from mRubberBand and mCaptureList*/
    void undo();

    void startCapturing();
    void stopCapturing();

    CaptureMode mode() { return mCaptureMode; }

    int size() { return mCaptureList.size(); }
    QList<QgsPoint>::iterator begin() { return mCaptureList.begin(); }
    QList<QgsPoint>::iterator end() { return mCaptureList.end(); }
    const QList<QgsPoint> &points() { return mCaptureList; }
    void closePolygon();

  private:
    /** which capturing tool is being used */
    enum CaptureMode mCaptureMode;

    /** Flag to indicate a map canvas capture operation is taking place */
    bool mCapturing;

    /** rubber band for polylines and polygons */
    QgsRubberBand* mRubberBand;

    /** List to store the points of digitised lines and polygons (in layer coordinates)*/
    QList<QgsPoint> mCaptureList;

    void validateGeometry();
    QString mTip;
    QgsGeometryValidator *mValidator;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;

    bool mCaptureModeFromLayer;

    QList<QgsVertexMarker *> mSnappingMarkers;
};

#endif
