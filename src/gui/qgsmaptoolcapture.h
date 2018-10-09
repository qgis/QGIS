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
#include "qgspointlocator.h"
#include "qgscompoundcurve.h"
#include "qgsgeometry.h"

#include <QPoint>
#include <QList>
#include "qgis_gui.h"

class QgsRubberBand;
class QgsSnapIndicator;
class QgsVertexMarker;
class QgsMapLayer;
class QgsGeometryValidator;

/**
 * \ingroup gui
 * \class QgsMapToolCapture
 */
class GUI_EXPORT QgsMapToolCapture : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT

  public:

    //! Different capture modes
    enum CaptureMode
    {
      CaptureNone,    //!< Do not capture / determine mode from layer geometry type
      CapturePoint,   //!< Capture points
      CaptureLine,    //!< Capture lines
      CapturePolygon  //!< Capture polygons
    };

    //! constructor
    QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );

    ~QgsMapToolCapture() override;

    void activate() override;
    void deactivate() override;

    /**
     * The capture mode
     *
     * \returns Capture mode
     */
    CaptureMode mode() const { return mCaptureMode; }

    //! Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS
    int addCurve( QgsCurve *c );

    /**
     * Clear capture curve.
     *
     * \since QGIS 3.0
     */
    void clearCurve( );

    /**
     * Gets the capture curve
     *
     * \returns Capture curve
     */
    const QgsCompoundCurve *captureCurve() const { return &mCaptureCurve; }

    /**
     * Returns a list of matches for each point on the captureCurve.
     *
     * \since QGIS 3.0
     */
    QList<QgsPointLocator::Match> snappingMatches() const;

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    /**
     * Intercept key events like Esc or Del to delete the last point
     * \param e key event
     */
    void keyPressEvent( QKeyEvent *e ) override;

    /**
     * Clean a temporary rubberband
     */
    void deleteTempRubberBand();

    //! convenient method to clean members
    void clean() override;

  private slots:
    void validationFinished();
    void addError( QgsGeometry::Error );
    void currentLayerChanged( QgsMapLayer *layer );


  protected:

    /**
     * Converts a map point to layer coordinates
     *  \param mapPoint the point in map coordinates
     *  \param[in,out] layerPoint the point in layer coordinates
     *  \returns
     *   0 in case of success
     *   1 if the current layer is null or not a vector layer
     *   2 if the transformation failed
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    int nextPoint( const QgsPoint &mapPoint, QgsPoint &layerPoint );

    /**
     * Converts a point to map coordinates and layer coordinates
     * \param p the input point
     * \param[in,out] layerPoint the point in layer coordinates
     * \param[in,out] mapPoint the point in map coordinates
     * \returns
     *  0 in case of success
     *  1 if the current layer is null or not a vector layer
     *  2 if the transformation failed
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    int nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint );

    /**
     * Fetches the original point from the source layer if it has the same
     * CRS as the current layer.
     * \returns 0 in case of success, 1 if not applicable (CRS mismatch), 2 in case of failure
     * \since QGIS 2.14
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    int fetchLayerPoint( const QgsPointLocator::Match &match, QgsPoint &layerPoint );

    /**
     * Creates a QgsPoint with ZM support if necessary (according to the
     * WkbType of the current layer). If the point is snapped, then the Z
     * value is took from the snapped point.
     *
     * \param e A mouse event
     *
     * \returns a point with ZM support if necessary
     *
     * \since QGIS 3.0
     */
    QgsPoint mapPoint( const QgsMapMouseEvent &e ) const;

    /**
     * Creates a QgsPoint with ZM support if necessary (according to the
     * WkbType of the current layer).
     *
     * \param point A point in 2D
     *
     * \returns a point with ZM support if necessary
     *
     * \since QGIS 3.0
     */
    QgsPoint mapPoint( const QgsPointXY &point ) const;

    /**
     * Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     * \returns 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed
     */
    // TODO QGIS 3.0 returns an enum instead of a magic constant
    int addVertex( const QgsPointXY &point );

    /**
     * Variant to supply more information in the case of snapping
     * \param mapPoint The vertex to add in map coordinates
     * \param match Data about the snapping match. Can be an invalid match, if point not snapped.
     * \since QGIS 2.14
     */
    int addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &match );

    //! Removes the last vertex from mRubberBand and mCaptureList
    void undo();

    /**
     * Start capturing
     */
    void startCapturing();

    /**
     * Are we currently capturing?
     *
     * \returns Is the tool in capture mode?
     */
    bool isCapturing() const;

    /**
     * Number of points digitized
     *
     * \returns Number of points
     */
    int size();

    /**
     * List of digitized points
     * \returns List of points
     */
    QVector<QgsPointXY> points() const;

    /**
     * Set the points on which to work
     *
     * \param pointList A list of points
     */
    void setPoints( const QVector<QgsPointXY> &pointList );

    /**
     * Close an open polygon
     */
    void closePolygon();

  protected slots:

    /**
     * Stop capturing
     */
    void stopCapturing();

  private:
    //! whether tracing has been requested by the user
    bool tracingEnabled();
    //! first point that will be used as a start of the trace
    QgsPointXY tracingStartPoint();
    //! handle of mouse movement when tracing enabled and capturing has started
    bool tracingMouseMove( QgsMapMouseEvent *e );
    //! handle of addition of clicked point (with the rest of the trace) when tracing enabled
    bool tracingAddVertex( const QgsPointXY &point );

  private:
    //! The capture mode in which this tool operates
    CaptureMode mCaptureMode;

    //! Flag to indicate a map canvas capture operation is taking place
    bool mCapturing = false;

    //! Rubber band for polylines and polygons
    QgsRubberBand *mRubberBand = nullptr;

    //! Temporary rubber band for polylines and polygons. this connects the last added point to the mouse cursor position
    QgsRubberBand *mTempRubberBand = nullptr;

    //! List to store the points of digitized lines and polygons (in layer coordinates)
    QgsCompoundCurve mCaptureCurve;

    QList<QgsPointLocator::Match> mSnappingMatches;

    void validateGeometry();
    QStringList mValidationWarnings;
    QgsGeometryValidator *mValidator = nullptr;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;

    bool mCaptureModeFromLayer = false;

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    /**
     * Keeps point (in map units) snapped to a segment where we most recently finished tracing,
     * so that we can use as the starting point for further tracing. This is useful mainly when
     * tracing with offset: without knowledge of this point user would need to click a segment
     * again after every time a new trace with offset is created (to get new "anchor" point)
     */
    QgsPointXY mTracingStartPoint;

    friend class TestQgsMapToolReshape;

};

#endif
