/***************************************************************************
                         qgscircularstring.h
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscurve.h"
#include "qgssimplecurve.h"

#include <QString>
#include <QVector>

using namespace Qt::StringLiterals;

/**
 * \ingroup core
 * \class QgsCircularString
 * \brief Circular string geometry type.
 */
class CORE_EXPORT QgsCircularString : public QgsSimpleCurve
{
  public:
    // clang-format off
    /**
     * Constructs an empty circular string.
     */
    QgsCircularString() SIP_HOLDGIL;
    // clang-format on

    /**
     * Constructs a circular string with a single
     * arc passing through \a p1, \a p2 and \a p3.
     *
     * \since QGIS 3.2
     */
    QgsCircularString( const QgsPoint &p1,
                       const QgsPoint &p2,
                       const QgsPoint &p3 ) SIP_HOLDGIL;

    /**
     * Construct a circular string from arrays of coordinates. If the z or m
     * arrays are non-empty then the resultant circular string will have
     * z and m types accordingly.
     *
     * This constructor is more efficient then calling setPoints().
     *
     * If the sizes of \a x and \a y are non-equal then the resultant circular string
     * will be created using the minimum size of these arrays.
     *
     * \warning It is the caller's responsibility to ensure that the supplied arrays
     * are of odd sizes.
     *
     * \since QGIS 3.20
     */
    QgsCircularString( const QVector<double> &x, const QVector<double> &y,
                       const QVector<double> &z = QVector<double>(),
                       const QVector<double> &m = QVector<double>() ) SIP_HOLDGIL;


    /**
     * Creates a circular string with a single arc representing
     * the curve from \a p1 to \a p2 with the specified \a center.
     *
     * If \a useShortestArc is TRUE, then the arc returned will be that corresponding
     * to the shorter arc from \a p1 to \a p2. If it is FALSE, the longer arc from \a p1
     * to \a p2 will be used (i.e. winding the other way around the circle).
     *
     * \since QGIS 3.2
     */
    static QgsCircularString fromTwoPointsAndCenter( const QgsPoint &p1,
        const QgsPoint &p2,
        const QgsPoint &center,
        bool useShortestArc = true );

    QString geometryType() const override SIP_HOLDGIL;
    QgsCircularString *clone() const override SIP_FACTORY;
    void clear() override;

    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17, Qgis::GeoJsonProfile profile = Qgis::GeoJsonProfile::Legacy ) const override SIP_SKIP;
    bool isEmpty() const override SIP_HOLDGIL;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;
    int indexOf( const QgsPoint &point ) const final;

    double length() const override;
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;
    QgsCircularString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0, bool removeRedundantPoints = false ) const override SIP_FACTORY;
    QgsAbstractGeometry *simplifyByDistance( double tolerance ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;

    void draw( QPainter &p ) const override;
    void addToPainterPath( QPainterPath &path ) const override;
    void drawAsPolygon( QPainter &p ) const override;
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool deleteVertex( QgsVertexId position ) override;
    bool deleteVertices( const QSet<QgsVertexId> &positions ) override;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    bool pointAt( int node, QgsPoint &point, Qgis::VertexType &type ) const override;
    void sumUpArea( double &sum SIP_OUT ) const override;
    void sumUpArea3D( double &sum SIP_OUT ) const override;
    bool hasCurvedSegments() const override;
    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    double distanceBetweenVertices( QgsVertexId fromVertex, QgsVertexId toVertex ) const override;
    QgsCircularString *reversed() const override SIP_FACTORY;
    QgsPoint *interpolatePoint( double distance ) const override SIP_FACTORY;
    QgsCircularString *curveSubstring( double startDistance, double endDistance ) const override SIP_FACTORY;

#ifndef SIP_RUN
    std::tuple< std::unique_ptr< QgsCurve >, std::unique_ptr< QgsCurve > > splitCurveAtVertex( int index ) const final;

    /**
     * Cast the \a geom to a QgsCircularString.
     * Should be used by qgsgeometry_cast<QgsCircularString *>( geometry ).
     *
     * Objects will be automatically converted to the appropriate target type.
     *
     * \note Not available in Python.
     */
    inline static const QgsCircularString *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::CircularString )
        return static_cast<const QgsCircularString *>( geom );
      return nullptr;
    }

    /**
     * Cast the \a geom to a QgsCircularString.
     * Should be used by qgsgeometry_cast<QgsCircularString *>( geometry ).
     *
     * Objects will be automatically converted to the appropriate target type.
     *
     * \note Not available in Python.
     */
    inline static QgsCircularString *cast( QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::CircularString )
        return static_cast<QgsCircularString *>( geom );
      return nullptr;
    }
#endif

    QgsCircularString *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
// clang-format off
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + u"..."_s;
    QString str = u"<QgsCircularString: %1>"_s.arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
// clang-format on
#endif

  protected:
    QgsBox3D calculateBoundingBox3D() const override;

  private:
#if 0
    static void arcTo( QPainterPath &path, QPointF pt1, QPointF pt2, QPointF pt3 );
#endif
    //bounding box of a single segment
    static QgsRectangle segmentBoundingBox( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 );
    static QgsPointSequence compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius );
    static double closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
                                     const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon );
    void insertVertexBetween( int after, int before, int pointOnCircle );
    void deleteVertex( int i );

};

// clazy:excludeall=qstring-allocations

#endif // QGSCIRCULARSTRING_H
