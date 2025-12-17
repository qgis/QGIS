/***************************************************************************
                         qgsnurbscurve.h
                         -----------------
    begin                : September 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNURBSCURVE_H
#define QGSNURBSCURVE_H

#include <float.h>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscurve.h"

#include <QPolygonF>

/**
 * \ingroup core
 * \class QgsNurbsCurve
 * \brief Represents a NURBS (Non-Uniform Rational B-Spline) curve geometry in 2D/3D.
 *
 * NURBS curves are a mathematical model commonly used in computer graphics
 * for representing curves. They are parametric curves defined by control points,
 * weights, knot vectors, and a degree.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsNurbsCurve : public QgsCurve
{
  public:

    /**
     * Constructor for an empty NURBS curve geometry.
     */
    QgsNurbsCurve();

    /**
     * Constructs a NURBS curve from control points, degree, knot vector and weights.
     * \param ctrlPoints control points defining the curve
     * \param degree degree of the NURBS curve (typically 1-3)
     * \param knots knot vector (must have size = control points count + degree + 1)
     * \param weights weight vector for rational curves (same size as control points)
     * \param closed whether the curve should be treated as closed
     */
    QgsNurbsCurve( const QVector<QgsPoint> &ctrlPoints, int degree,
                   const QVector<double> &knots, const QVector<double> &weights,
                   bool closed = false );

    QgsCurve *clone() const override SIP_FACTORY;

    /**
     * Evaluates the NURBS curve at parameter t ∈ [0,1].
     * Uses the Cox-de Boor algorithm for B-spline basis function evaluation.
     * \param t parameter value between 0 and 1
     * \returns point on the curve at parameter t
     */
    [[nodiscard]] QgsPoint evaluate( double t ) const;

    /**
     * Returns TRUE if this curve represents a Bézier curve.
     * A Bézier curve is a special case of NURBS with uniform weights and specific knot vector.
     */
    bool isBezier() const;

    /**
     * Returns TRUE if this curve represents a B-spline (non-rational NURBS).
     */
    bool isBSpline() const;

    /**
     * Returns TRUE if this curve is rational (has non-uniform weights).
     */
    bool isRational() const;

    bool isClosed() const override SIP_HOLDGIL;
    bool isClosed2D() const override SIP_HOLDGIL;

    // QgsCurve interface
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;
    void draw( QPainter &p ) const override;
    void drawAsPolygon( QPainter &p ) const override;
    QgsPoint endPoint() const override SIP_HOLDGIL;
    bool equals( const QgsCurve &other ) const override;
    int indexOf( const QgsPoint &point ) const override;
    QgsPoint *interpolatePoint( double distance ) const override SIP_FACTORY;
    int numPoints() const override SIP_HOLDGIL;
    bool pointAt( int node, QgsPoint &point SIP_OUT, Qgis::VertexType &type SIP_OUT ) const override;
    void points( QgsPointSequence &pts SIP_OUT ) const override;
    QgsCurve *reversed() const override SIP_FACTORY;
    void scroll( int firstVertexIndex ) override;
    std::tuple<std::unique_ptr<QgsCurve>, std::unique_ptr<QgsCurve>> splitCurveAtVertex( int index ) const override SIP_SKIP;
    QgsPoint startPoint() const override SIP_HOLDGIL;
    void sumUpArea( double &sum SIP_OUT ) const override;
    double xAt( int index ) const override;
    double yAt( int index ) const override;
    double zAt( int index ) const override;
    double mAt( int index ) const override;

    QPolygonF asQPolygonF() const override;

    void addToPainterPath( QPainterPath &path ) const override;
    QgsCurve *curveSubstring( double startDistance, double endDistance ) const override SIP_FACTORY;
    double length() const override SIP_HOLDGIL;
    double segmentLength( QgsVertexId startVertex ) const override;
    double distanceBetweenVertices( QgsVertexId fromVertex, QgsVertexId toVertex ) const override;
    QgsAbstractGeometry *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0, bool removeRedundantPoints = false ) const override SIP_FACTORY;
    QgsAbstractGeometry *simplifyByDistance( double tolerance ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    double vertexAngle( QgsVertexId vertex ) const override;
    void swapXy() override;
    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) override;
    QgsAbstractGeometry *createEmptyWithSameType() const override SIP_FACTORY;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) override SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;
    QgsRectangle boundingBox() const override;
    QgsBox3D boundingBox3D() const override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    QString asKml( int precision = 17 ) const override;
    int dimension() const override SIP_HOLDGIL;
    bool isEmpty() const override SIP_HOLDGIL;
    void clear() override;
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const override SIP_HOLDGIL;
    bool boundingBoxIntersects( const QgsBox3D &box3d ) const override SIP_HOLDGIL;
    QgsPoint centroid() const override;

    // QgsAbstractGeometry interface
    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    bool deleteVertex( QgsVertexId position ) override;
#ifndef SIP_RUN
    void filterVertices( const std::function<bool( const QgsPoint & )> &filter ) override;
#endif
    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;
    bool fuzzyEqual( const QgsAbstractGeometry &other, double epsilon = 1e-8 ) const override SIP_HOLDGIL;
    bool fuzzyDistanceEqual( const QgsAbstractGeometry &other, double epsilon = 1e-8 ) const override SIP_HOLDGIL;
    QString geometryType() const override SIP_HOLDGIL;
    bool hasCurvedSegments() const override SIP_HOLDGIL;
    int partCount() const override SIP_HOLDGIL;
    QgsCurve *toCurveType() const override;
    QgsPoint vertexAt( QgsVertexId id ) const override;
    int vertexCount( int part = 0, int ring = 0 ) const override SIP_HOLDGIL;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    bool isValid( QString &error SIP_OUT,
                  Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;

    /**
     * Returns the degree of the NURBS curve.
     */
    int degree() const SIP_HOLDGIL { return mDegree; }

    /**
     * Sets the degree of the NURBS curve.
     * \param degree curve degree (typically 1-3)
     */
    void setDegree( int degree ) { mDegree = degree; clearCache(); }

    /**
     * Returns the control points of the NURBS curve.
     */
    const QVector<QgsPoint> &controlPoints() const SIP_HOLDGIL { return mControlPoints; }

    /**
     * Sets the control points of the NURBS curve.
     * \param points control points
     */
    void setControlPoints( const QVector<QgsPoint> &points ) { mControlPoints = points; clearCache(); }

    /**
     * Returns the knot vector of the NURBS curve.
     */
    const QVector<double> &knots() const SIP_HOLDGIL { return mKnots; }

    /**
     * Sets the knot vector of the NURBS curve.
     * \param knots knot vector (must have size = control points count + degree + 1)
     */
    void setKnots( const QVector<double> &knots ) { mKnots = knots; clearCache(); }

    /**
     * Returns the weight vector of the NURBS curve.
     */
    const QVector<double> &weights() const SIP_HOLDGIL { return mWeights; }

    /**
     * Sets the weight vector of the NURBS curve.
     * \param weights weight vector (same size as control points)
     */
    void setWeights( const QVector<double> &weights ) { mWeights = weights; clearCache(); }

    /**
     * Returns the weight at the specified control point \a index.
     * Returns 1.0 if index is out of range.
     * \since QGIS 4.0
     */
    double weight( int index ) const SIP_HOLDGIL;

    /**
     * Sets the \a weight at the specified control point \a index.
     * Weight must be positive (> 0).
     * \returns TRUE if successful, FALSE if index is out of range or weight is invalid.
     * \since QGIS 4.0
     */
    bool setWeight( int index, double weight );

    /**
     * Cast the \a geom to a QgsNurbsCurve.
     * Should be used by qgsgeometry_cast<QgsNurbsCurve *>( geometry ).
     * \note Not available in Python.
     */
    inline static const QgsNurbsCurve *cast( const QgsAbstractGeometry *geom ) SIP_SKIP // cppcheck-suppress duplInheritedMember
    {
      if ( geom && geom->geometryType() == QLatin1String( "NurbsCurve" ) )
        return static_cast<const QgsNurbsCurve *>( geom );
      return nullptr;
    }

    /**
     * Cast the \a geom to a QgsNurbsCurve.
     * Should be used by qgsgeometry_cast<QgsNurbsCurve *>( geometry ).
     * \note Not available in Python.
     */
    inline static QgsNurbsCurve *cast( QgsAbstractGeometry *geom ) SIP_SKIP // cppcheck-suppress duplInheritedMember
    {
      if ( geom && geom->geometryType() == QLatin1String( "NurbsCurve" ) )
        return static_cast<QgsNurbsCurve *>( geom );
      return nullptr;
    }

  protected:

    void clearCache() const override;
    int compareToSameClass( const QgsAbstractGeometry *other ) const final;
    QgsBox3D calculateBoundingBox3D() const override;

  private:
    QVector<QgsPoint> mControlPoints;  //! Control points defining the curve shape
    QVector<double> mKnots;           //! Knot vector for B-spline basis functions
    QVector<double> mWeights;         //! Weight vector for rational curves
    int mDegree = 0;                  //! Degree of the NURBS curve
    bool mClosed = false;             //! Whether the curve is closed

};

#endif // QGSNURBSCURVE_H

