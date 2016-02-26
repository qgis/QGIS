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

    //! active the tool
    virtual void activate();

    //! deactive the tool
    virtual void deactivate();

    /** Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS*/
    int addCurve( QgsCurveV2* c );

    /**
     * Get the capture curve
     *
     * @return Capture curve
     */
    const QgsCompoundCurveV2* captureCurve() const;


    /**
     * Update the rubberband according to mouse position
     *
     * @param e The mouse event
     */
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent * e );

    /**
     * Intercept key events like Esc or Del to delete the last point
     * @param e key event
     */
    virtual void keyPressEvent( QKeyEvent* e );

    /**
     * Clean a temporary rubberband
     */
    void deleteTempRubberBand();

  private slots:
    void validationFinished();
    void currentLayerChanged( QgsMapLayer *layer );
    void addError( QgsGeometry::Error );


  protected:
    /** Converts a map point to layer coordinates
     * @param mapPoint the point in map coordinates
     * @param[in,out] layerPoint the point in layer coordinates
     * @return
     *  0 in case of success
     *  1 if the current layer is null or not a vector layer
     *  2 if the transformation failed
     * @deprecated use nextPoint(const QgsPointV2&, QgsPointV2&)
     */
    int nextPoint( const QgsPoint& mapPoint, QgsPoint& layerPoint ) /Deprecated/;

    /** Converts a map point to layer coordinates
     *  @param mapPoint the point in map coordinates
     *  @param[in,out] layerPoint the point in layer coordinates
     *  @return
     *   0 in case of success
     *   1 if the current layer is null or not a vector layer
     *   2 if the transformation failed
     */
    int nextPoint( const QgsPointV2& mapPoint, QgsPointV2& layerPoint );

    /** Converts a point to map coordinates and layer coordinates
     * @param p the input point
     * @param[in,out] layerPoint the point in layer coordinates
     * @param[in,out] mapPoint the point in map coordinates
     * @return
     *  0 in case of success
     *  1 if the current layer is null or not a vector layer
     *  2 if the transformation failed
     * @deprecated use nextPoint( const QPoint&, QgsPointV2&, QgsPointV2& )
     */
    int nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint ) /Deprecated/;

    /** Converts a point to map coordinates and layer coordinates
     * @param p the input point
     * @param[in,out] layerPoint the point in layer coordinates
     * @param[in,out] mapPoint the point in map coordinates
     * @return
     *  0 in case of success
     *  1 if the current layer is null or not a vector layer
     *  2 if the transformation failed
     */
    int nextPoint( QPoint p, QgsPointV2 &layerPoint, QgsPointV2 &mapPoint );

    /** Fetches the original point from the source layer if it has the same
     * CRS as the current layer.
     * @return 0 in case of success, 1 if not applicable (CRS mismatch), 2 in case of failure
     * @note added in 2.14
     */
    int fetchLayerPoint( QgsPointLocator::Match match, QgsPointV2& layerPoint );

    /** Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     * @return 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed
     */
    int addVertex( const QgsPoint& point );

    /** Variant to supply more information in the case of snapping
     * @param mapPoint The vertex to add in map coordinates
     * @param match Data about the snapping match. Can be an invalid match, if point not snapped.
     * @note added in 2.14
     */
    int addVertex( const QgsPoint& mapPoint, QgsPointLocator::Match match );

    /** Removes the last vertex from mRubberBand and mCaptureList*/
    void undo();

    /**
     * Start capturing
     */
    void startCapturing();

    /**
     * Are we currently capturing?
     *
     * @return Is the tool in capture mode?
     */
    bool isCapturing() const;

    /**
     * Stop capturing
     */
    void stopCapturing();

    /**
     * Number of points digitized
     *
     * @return Number of points
     */
    int size();

    /**
     * List of digitized points
     * @return List of points
     */
    QList<QgsPoint> points();

    /**
     * Set the points on which to work
     *
     * @param pointList A list of points
     */
    void setPoints( const QList<QgsPoint>& pointList );

    /**
     * Close an open polygon
     */
    void closePolygon();
};
