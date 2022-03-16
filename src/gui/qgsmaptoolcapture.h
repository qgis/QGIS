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
#include "qobjectuniqueptr.h"
#include "qgssnappingutils.h"

#include <QPoint>
#include <QList>
#include "qgis_gui.h"

class QgsRubberBand;
class QgsSnapIndicator;
class QgsVertexMarker;
class QgsMapLayer;
class QgsGeometryValidator;
class QgsMapToolCaptureRubberBand;
class QgsCurvePolygon;
class QgsMapToolShapeAbstract;
class QgsMapToolShapeMetadata;


/**
 * \ingroup gui
 * QgsMapToolCapture is a base class capable of capturing point, lines and polygons.
 * The tool supports different techniques: straight segments, curves, streaming and shapes
 * Once the the geometry is captured the virtual private handler geometryCaptured is called
 * as well as a more specific handler (pointCaptured, lineCaptured or polygonCaptured)
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

    //! Specific capabilities of the tool
    enum Capability
    {
      NoCapabilities = 1 << 0, //!< No specific capabilities
      SupportsCurves = 1 << 1, //!< Supports curved geometries input
      ValidateGeometries = 1 << 2, //!< Tool supports geometry validation (since QGIS 3.22)
    };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    //! constructor
    QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );

    ~QgsMapToolCapture() override;

    /**
     * Returns flags containing the supported capabilities
     */
    virtual QgsMapToolCapture::Capabilities capabilities() const;

    /**
     * Returns TRUE if the tool supports the specified capture \a technique.
     *
     * \since QGIS 3.20
     */
    virtual bool supportsTechnique( Qgis::CaptureTechnique technique ) const;

    /**
     * Sets the current capture if it is supported by the map tool
     * \since QGIS 3.26
     */
    void setCurrentCaptureTechnique( Qgis::CaptureTechnique technique );

    /**
     * Sets the current shape tool
     * \see QgsMapToolShapeRegistry
     * \since QGIS 3.26
     */
    void setCurrentShapeMapTool( const QgsMapToolShapeMetadata *shapeMapToolMetadata ) SIP_SKIP;

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
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

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

    /**
     * Returns the rubberBand currently owned by this map tool and
     * transfers ownership to the caller.
     *
     * \since QGIS 3.8
     */
    QgsRubberBand *takeRubberBand() SIP_FACTORY;

    /**
     * Creates a QgsPoint with ZM support if necessary (according to the
     * WkbType of the current layer). If the point is snapped, then the Z
     * value is derived from the snapped point.
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

    // TODO QGIS 4.0 returns an enum instead of a magic constant

  public slots:

    /**
     * Enable the digitizing with curve
     * \deprecated since QGIS 3.26 use setCurrentCaptureTechnique() instead
     */
    Q_DECL_DEPRECATED void setCircularDigitizingEnabled( bool enable ) SIP_DEPRECATED;

    /**
     * Toggles the stream digitizing mode.
     * \since QGIS 3.20
    * \deprecated since QGIS 3.26 use setCurrentCaptureTechnique() instead
     */
    Q_DECL_DEPRECATED void setStreamDigitizingEnabled( bool enable ) SIP_DEPRECATED;

  private slots:
    void addError( const QgsGeometry::Error &error );
    void currentLayerChanged( QgsMapLayer *layer );
    //! Update the extra snap layer, this should be called whenever the capturecurve changes
    void updateExtraSnapLayer();

  protected:

    // TODO QGIS 4.0 returns an enum instead of a magic constant

    /**
     * Converts a map point to layer coordinates
     *  \param mapPoint the point in map coordinates
     *  \param[in,out] layerPoint the point in layer coordinates
     *  \returns
     *   0 in case of success
     *   1 if the current layer is NULLPTR
     *   2 if the transformation failed
     */
    int nextPoint( const QgsPoint &mapPoint, QgsPoint &layerPoint );

    // TODO QGIS 4.0 returns an enum instead of a magic constant

    /**
     * Converts a point to map coordinates and layer coordinates
     * \param p the input point
     * \param[in,out] layerPoint the point in layer coordinates
     * \param[in,out] mapPoint the point in map coordinates
     * \returns
     *  0 in case of success
     *  1 if the current layer is NULLPTR or not a vector layer
     *  2 if the transformation failed
     */
    int nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint );

    // TODO QGIS 4.0 returns an enum instead of a magic constant

    /**
     * Fetches the original point from the source layer if it has the same
     * CRS as the current layer.
     * \returns 0 in case of success, 1 if not applicable (CRS mismatch), 2 in case of failure
     * \since QGIS 2.14
     */
    int fetchLayerPoint( const QgsPointLocator::Match &match, QgsPoint &layerPoint );

    /**
     * Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     * \returns 0 in case of success, 2 if coordinate transformation failed
     */
    int addVertex( const QgsPointXY &point );

    /**
     * Variant to supply more information in the case of snapping
     * \param mapPoint The vertex to add in map coordinates
     * \param match Data about the snapping match. Can be an invalid match, if point not snapped.
     * \since QGIS 2.14
     */
    int addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &match );

    /**
     * Removes the last vertex from mRubberBand and mCaptureList.
     *
     * Since QGIS 3.20, if \a isAutoRepeat is set to TRUE then the undo operation will be treated
     * as a auto repeated undo as if the user has held down the undo key for an extended period of time.
     */
    void undo( bool isAutoRepeat = false );

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
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant returns QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED QVector<QgsPointXY> points() const SIP_DEPRECATED;

    // TODO QGIS 4.0 rename it to points()

    /**
     * List of digitized points
     * \returns List of points
     * \since QGIS 3.12
     */
    QgsPointSequence pointsZM() const;

    /**
     * Set the points on which to work
     *
     * \param pointList A list of points
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED void setPoints( const QVector<QgsPointXY> &pointList ) SIP_DEPRECATED;

    /**
     * Set the points on which to work
     *
     * \param pointList A list of points
     * \since QGIS 3.12
     */
    void setPoints( const QgsPointSequence &pointList );

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

    /**
     * Called when the geometry is captured
     * A more specific handler is also called afterwards (pointCaptured, lineCaptured or polygonCaptured)
     * \since QGIS 3.26
     */
    virtual void geometryCaptured( const QgsGeometry &geometry ) {Q_UNUSED( geometry )} SIP_FORCE

    /**
     * Called when a point is captured
     * geometryCaptured is called just before
     * \since QGIS 3.26
     */
    virtual void pointCaptured( const QgsPoint &point ) {Q_UNUSED( point )} SIP_FORCE

    /**
     * Called when a line is captured
     * geometryCaptured is called just before
     * \since QGIS 3.26
     */
    virtual void lineCaptured( const QgsCurve *line ) {Q_UNUSED( line )} SIP_FORCE

    /**
     * Called when a polygon is captured
     * geometryCaptured is called just before
     * \since QGIS 3.26
     */
    virtual void polygonCaptured( const QgsCurvePolygon *polygon ) {Q_UNUSED( polygon )} SIP_FORCE

    //! whether tracing has been requested by the user
    bool tracingEnabled();
    //! first point that will be used as a start of the trace
    QgsPointXY tracingStartPoint();
    //! handle of mouse movement when tracing enabled and capturing has started
    bool tracingMouseMove( QgsMapMouseEvent *e );
    //! handle of addition of clicked point (with the rest of the trace) when tracing enabled
    bool tracingAddVertex( const QgsPointXY &point );

    //! create a curve rubber band
    QgsMapToolCaptureRubberBand *createCurveRubberBand() const;

    //! Reset the
    void resetRubberBand();

    //! The capture mode in which this tool operates
    CaptureMode mCaptureMode;

    //! Flag to indicate a map canvas capture operation is taking place
    bool mCapturing = false;

    //! extremity point of the captured curve in map coordinates
    QgsPoint mCaptureFirstPoint;
    QgsPoint mCaptureLastPoint;

    //! Rubber band for polylines and polygons
    QObjectUniquePtr<QgsRubberBand> mRubberBand;

    //! Temporary rubber band for polylines and polygons. this connects the last added point to the mouse cursor position
    std::unique_ptr<QgsMapToolCaptureRubberBand> mTempRubberBand;

    //! List to store the points of digitized lines and polygons (in layer coordinates)
    QgsCompoundCurve mCaptureCurve;

    QList<QgsPointLocator::Match> mSnappingMatches;
    QgsPointLocator::Match mCircularIntermediateMatch;
    QgsPoint mCircularItermediatePoint;

    void validateGeometry();
    QgsGeometryValidator *mValidator = nullptr;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;

    //! A layer containing the current capture curve to provide additional snapping
    QgsVectorLayer *mExtraSnapLayer = nullptr;
    //! The feature in that layer (for updating)
    QgsFeatureId mExtraSnapFeatureId;

    bool mCaptureModeFromLayer = false;

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    /**
     * Keeps point (in map units) snapped to a segment where we most recently finished tracing,
     * so that we can use as the starting point for further tracing. This is useful mainly when
     * tracing with offset: without knowledge of this point user would need to click a segment
     * again after every time a new trace with offset is created (to get new "anchor" point)
     */
    QgsPointXY mTracingStartPoint;

    //! Used to store the state of digitizing type (linear or circular)
    QgsWkbTypes::Type mLineDigitizingType = QgsWkbTypes::LineString;

    Qgis::CaptureTechnique mCurrentCaptureTechnique = Qgis::CaptureTechnique::StraightSegments;

    QgsMapToolShapeAbstract *mCurrentShapeMapTool = nullptr;

    bool mAllowAddingStreamingPoints = false;
    int mStreamingToleranceInPixels = 1;

    bool mStartNewCurve = false;

    bool mIgnoreSubsequentAutoRepeatUndo = false;

    friend class TestQgsMapToolCapture;


};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapToolCapture::Capabilities )

#endif
