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


#include "qgsmaptooladvanceddigitizing.h"
#include "qgscompoundcurvev2.h"
#include "qgspoint.h"
#include "qgsgeometry.h"
#include "qgslayertreeview.h"
#include "qgspointlocator.h"

#include <QPoint>
#include <QList>

class QgsRubberBand;
class QgsVertexMarker;
class QgsMapLayer;
class QgsGeometryValidator;

/** \ingroup gui
 * \class QgsMapToolCapture
 */
class GUI_EXPORT QgsMapToolCapture : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolCapture( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget, CaptureMode mode = CaptureNone );

    //! destructor
    virtual ~QgsMapToolCapture();

    //! active the tool
    virtual void activate() override;

    //! deactive the tool
    virtual void deactivate() override;

    /** Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS*/
    int addCurve( QgsCurveV2* c );

    /**
     * Get the capture curve
     *
     * @return Capture curve
     */
    const QgsCompoundCurveV2* captureCurve() const { return &mCaptureCurve; }


    /**
     * Update the rubberband according to mouse position
     *
     * @param e The mouse event
     */
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent * e ) override;

    /**
     * Intercept key events like Esc or Del to delete the last point
     * @param e key event
     */
    virtual void keyPressEvent( QKeyEvent* e ) override;

#ifdef Q_OS_WIN
    virtual bool eventFilter( QObject *obj, QEvent *e ) override;
#endif

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
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    Q_DECL_DEPRECATED int nextPoint( const QgsPoint& mapPoint, QgsPoint& layerPoint );

    /** Converts a map point to layer coordinates
     *  @param mapPoint the point in map coordinates
     *  @param[in,out] layerPoint the point in layer coordinates
     *  @return
     *   0 in case of success
     *   1 if the current layer is null or not a vector layer
     *   2 if the transformation failed
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
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
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    Q_DECL_DEPRECATED int nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint );

    /** Converts a point to map coordinates and layer coordinates
     * @param p the input point
     * @param[in,out] layerPoint the point in layer coordinates
     * @param[in,out] mapPoint the point in map coordinates
     * @return
     *  0 in case of success
     *  1 if the current layer is null or not a vector layer
     *  2 if the transformation failed
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    int nextPoint( QPoint p, QgsPointV2 &layerPoint, QgsPointV2 &mapPoint );

    /** Fetches the original point from the source layer if it has the same
     * CRS as the current layer.
     * @return 0 in case of success, 1 if not applicable (CRS mismatch), 2 in case of failure
     * @note added in 2.14
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    int fetchLayerPoint( QgsPointLocator::Match match, QgsPointV2& layerPoint );

    /** Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     * @return 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
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

  private:
    //! whether tracing has been requested by the user
    bool tracingEnabled();
    //! first point that will be used as a start of the trace
    QgsPoint tracingStartPoint();
    //! handle of mouse movement when tracing enabled and capturing has started
    bool tracingMouseMove( QgsMapMouseEvent* e );
    //! handle of addition of clicked point (with the rest of the trace) when tracing enabled
    bool tracingAddVertex( const QgsPoint& point );

  private:
    /** Flag to indicate a map canvas capture operation is taking place */
    bool mCapturing;

    /** Rubber band for polylines and polygons */
    QgsRubberBand* mRubberBand;

    /** Temporary rubber band for polylines and polygons. this connects the last added point to the mouse cursor position */
    QgsRubberBand* mTempRubberBand;

    /** List to store the points of digitised lines and polygons (in layer coordinates)*/
    QgsCompoundCurveV2 mCaptureCurve;

    void validateGeometry();
    QStringList mValidationWarnings;
    QgsGeometryValidator *mValidator;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;

    bool mCaptureModeFromLayer;

    QgsVertexMarker* mSnappingMarker;

#ifdef Q_OS_WIN
    int mSkipNextContextMenuEvent;
#endif
};

#endif
