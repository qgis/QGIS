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
#include "qgsgeometryrubberband.h"

#include <QPoint>
#include <QList>
#include "qgis_gui.h"

class QgsRubberBand;
class QgsSnapIndicator;
class QgsVertexMarker;
class QgsMapLayer;
class QgsGeometryValidator;
class QgsMapToolCaptureRubberBand;


#ifndef SIP_RUN

///@cond PRIVATE

/**
 * Class that reprensents a rubber band that can be linear or circular.
 *
 * \since QGIS 3.16
 */
class QgsMapToolCaptureRubberBand: public QgsGeometryRubberBand
{
  public:
    //! Constructor
    QgsMapToolCaptureRubberBand( QgsMapCanvas *mapCanvas, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::LineGeometry );

    //! Returns the curve defined by the rubber band, the caller has to take the ownership, nullptr if no curve is defined.
    QgsCurve *curve();

    /**
     * Returns if the curve defined by the rubber band is complete :
     * has more than 2 points for circular string and more than 1 point for linear string
     */
    bool curveIsComplete() const;

    /**
     * Resets the rubber band with the specified geometry type
     * that must be line geometry or polygon geometry.
     * \a firstPolygonPoint is the first point that will be used to render the polygon rubber band (if \a geomType is PolygonGeometry)
     */
    void reset( QgsWkbTypes::GeometryType geomType = QgsWkbTypes::LineGeometry, QgsWkbTypes::Type stringType = QgsWkbTypes::LineString, const QgsPoint &firstPolygonPoint = QgsPoint() );

    //! Sets the geometry type of the rubberband without removing already existing points
    void setRubberBandGeometryType( QgsWkbTypes::GeometryType geomType );

    //! Adds point to the rubber band
    void addPoint( const QgsPoint &point, bool doUpdate = true );

    //! Moves the last point to the \a point position
    void movePoint( const QgsPoint &point );

    //! Moves the point with \a index to the \a point position
    void movePoint( int index, const QgsPoint &point );

    //! Returns the points count in the rubber band (except the first point if polygon)
    int pointsCount();

    //! Returns the type of the curve (linear string or circular string)
    QgsWkbTypes::Type stringType() const;

    //! Sets the type of the curve (linear string or circular string)
    void setStringType( const QgsWkbTypes::Type &type );

    //! Returns the last point of the rubber band
    QgsPoint lastPoint() const;

    //! Returns the point of the rubber band at position from end
    QgsPoint pointFromEnd( int posFromEnd ) const;

    //! Removes the last point of the rrubber band
    void removeLastPoint();

  private:
    QgsWkbTypes::Type mStringType = QgsWkbTypes::LineString;

    void setGeometry( QgsAbstractGeometry *geom ) override;
    void updateCurve();

    QgsCurve *createLinearString();
    QgsCurve *createCircularString();

    QgsPointSequence mPoints;
    QgsPoint mFirstPolygonPoint;
};

/// @endcond

#endif //SIP_RUN

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

    //! Specific capabilities of the tool
    enum Capability
    {
      NoCapabilities = 0,       //!< No specific capabilities
      SupportsCurves = 1,       //!< Supports curved geometries input
    };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    //! constructor
    QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );

    ~QgsMapToolCapture() override;

    /**
     * Returns flags containing the supported capabilities
     */
    virtual QgsMapToolCapture::Capabilities capabilities() const;

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

    /**
     * Returns the rubberBand currently owned by this map tool and
     * transfers ownership to the caller.
     *
     * \since QGIS 3.8
     */
    QgsRubberBand *takeRubberBand() SIP_FACTORY;

  public slots:
    //! Enable the digitizing with curve
    void setCircularDigitizingEnabled( bool enable );

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
     *   1 if the current layer is NULLPTR or not a vector layer
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

    // TODO QGIS 4.0 returns an enum instead of a magic constant

    /**
     * Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)
     * \returns 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed
     */
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

    //! Returns extemity point of the captured curve in map coordinates
    QgsPoint firstCapturedMapPoint();
    QgsPoint lastCapturedMapPoint();

    //! Reset the
    void resetRubberBand();

  private:
    //! The capture mode in which this tool operates
    CaptureMode mCaptureMode;

    //! Flag to indicate a map canvas capture operation is taking place
    bool mCapturing = false;

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
    QgsWkbTypes::Type mDigitizingType = QgsWkbTypes::LineString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapToolCapture::Capabilities )

#endif
