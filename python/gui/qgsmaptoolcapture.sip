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

class QgsMapToolCapture : public QgsMapToolAdvancedDigitizing
{
%TypeHeaderCode
#include "qgsmaptoolcapture.h"
%End
  public:
    //! constructor
    QgsMapToolCapture( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget, CaptureMode mode = CaptureNone );

    //! destructor
    virtual ~QgsMapToolCapture();

    //! deactive the tool
    virtual void deactivate();

    /** Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS*/
    int addCurve( QgsCurveV2* c );

    const QgsCompoundCurveV2* captureCurve() const;

    void deleteTempRubberBand();
    void cadCanvasMoveEvent( QgsMapMouseEvent * e );
    void keyPressEvent( QKeyEvent* e );

  private slots:
    void validationFinished();

    void currentLayerChanged( QgsMapLayer *layer );
    void addError( QgsGeometry::Error );


  protected:
    int nextPoint( const QgsPoint& mapPoint, QgsPoint& layerPoint );
    int nextPoint( const QPoint &p, QgsPoint &layerPoint, QgsPoint &mapPoint );

    /** Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     @return 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed*/
    int addVertex( const QgsPoint& point );

    /** Removes the last vertex from mRubberBand and mCaptureList*/
    void undo();

    void startCapturing();
    bool isCapturing() const;
    void stopCapturing();

    int size();
    QList<QgsPoint> points();
    void setPoints( const QList<QgsPoint>& pointList );
    void closePolygon();
};
