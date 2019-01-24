/***************************************************************************
                         qgstriangle.h
                         -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTRIANGLE_H
#define QGSTRIANGLE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspolygon.h"
#include "qgscircle.h"
#include "qgslinestring.h"

/**
 * \ingroup core
 * \class QgsTriangle
 * \brief Triangle geometry type.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsTriangle : public QgsPolygon
{
  public:
    QgsTriangle();

    /**
     * Construct a QgsTriangle from three QgsPointV2.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     */
    QgsTriangle( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3 );

    /**
     * Construct a QgsTriangle from three QgsPoint.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     */
    explicit QgsTriangle( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 );

    /**
     * Construct a QgsTriangle from three QPointF.
     * \param p1 first point
     * \param p2 second point
     * \param p3 third point
     */
    explicit QgsTriangle( QPointF p1, QPointF p2, QPointF p3 );

    bool operator==( const QgsTriangle &other ) const;
    bool operator!=( const QgsTriangle &other ) const;

    QString geometryType() const override;
    QgsTriangle *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkbPtr ) override;

    bool fromWkt( const QString &wkt ) override;

    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited (as a polygon): QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;

    QgsPolygon *surfaceToPolygon() const override SIP_FACTORY;

    QgsCurvePolygon *toCurveType() const override SIP_FACTORY;

    //! Inherited method not used. You cannot add an interior ring into a triangle.
    void addInteriorRing( QgsCurve *ring SIP_TRANSFER ) override;

    /**
     * Inherited method not used. You cannot add an interior ring into a triangle.
     * \note not available in Python bindings
     */
    void setInteriorRings( const QVector< QgsCurve *> &rings ) = delete;
    //! Inherited method not used. You cannot delete or insert a vertex directly. Returns always false.
    bool deleteVertex( QgsVertexId position ) override;
    //! Inherited method not used. You cannot delete or insert a vertex directly. Returns always false.
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId vId, const QgsPoint &newPos ) override;

    void setExteriorRing( QgsCurve *ring SIP_TRANSFER ) override;

    QgsCurve *boundary() const override SIP_FACTORY;

    // inherited: double pointDistanceToBoundary( double x, double y ) const;

    /**
     *  Returns coordinates of a vertex.
     *  \param atVertex index of the vertex
     *  \returns Coordinates of the vertex or QgsPoint(0,0) on error (\a atVertex < 0 or > 3).
     */
    QgsPoint vertexAt( int atVertex ) const;

    /**
     * Returns the three lengths of the triangle.
     * \returns Lengths of triangle ABC where [AB] is at 0, [BC] is at 1, [CA] is at 2.
     * An empty list is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.lengths()
     *   # [5.0, 5.0, 7.0710678118654755]
     *   QgsTriangle().lengths()
     *   # []
     * \endcode
     */
    QVector<double> lengths() const;

    /**
     * Returns the three angles of the triangle.
     * \returns Angles in radians of triangle ABC where angle BAC is at 0, angle ABC is at 1, angle BCA is at 2.
     * An empty list is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   [math.degrees(i) for i in tri.angles()]
     *   # [45.0, 90.0, 45.0]
     *   QgsTriangle().angles()
     *   # []
     * \endcode
     */
    QVector<double> angles() const;

    /**
     * Convenient method checking if the geometry is degenerate (have duplicate or colinear point(s)).
     * \returns True if the triangle is degenerate or empty, otherwise false.
     * Example:
     * \code{.py}
     *   tri = QgsTriangle()
     *   tri.isDegenerate()
     *   # True
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.isDegenerate()
     *   # False
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 10, 10 ) )
     *   tri.isDegenerate()
     *   # True
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) )
     *   tri.isDegenerate()
     *   # True
     *  \endcode
     */
    bool isDegenerate();

    /**
     * Is the triangle isocele (two sides with the same length)?
     * \param lengthTolerance The tolerance to use
     * \returns True or False. Always false for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.lengths()
     *   # [5.0, 5.0, 7.0710678118654755]
     *   tri.isIsocele()
     *   # True
     *   # length of [AB] == length of [BC]
     *   QgsTriangle().isIsocele()
     *   # False
     * \endcode
     */
    bool isIsocele( double lengthTolerance = 0.0001 ) const;

    /**
     * Is the triangle equilateral (three sides with the same length)?
     * \param lengthTolerance The tolerance to use
     * \returns True or False. Always false for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) )
     *   tri.lengths()
     *   # [6.0, 6.0000412031918575, 6.0000412031918575]
     *   tri.isEquilateral()
     *   # True
     *   # All lengths are close to 6.0
     *   QgsTriangle().isEquilateral()
     *   # False
     * \endcode
     */
    bool isEquilateral( double lengthTolerance = 0.0001 ) const;

    /**
     * Is the triangle right-angled?
     * \param angleTolerance The tolerance to use
     * \returns True or False. Always false for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   [math.degrees(i) for i in tri.angles()]
     *   # [45.0, 90.0, 45.0]
     *   tri.isRight()
     *   # True
     *   # angle of ABC == 90
     *   QgsTriangle().isRight()
     *   # False
     * \endcode
     */
    bool isRight( double angleTolerance = 0.0001 ) const;

    /**
     * Is the triangle scalene (all sides have different lengths)?
     * \param lengthTolerance The tolerance to use
     * \returns True or False. Always false for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 7.2825, 4.2368 ), QgsPoint( 13.0058, 3.3218 ), QgsPoint( 9.2145, 6.5242 ) )
     *   tri.lengths()
     *   # [5.795980321740233, 4.962793714229921, 2.994131386562721]
     *   tri.isScalene()
     *   # True
     *   # All lengths are different
     *   QgsTriangle().isScalene()
     *   # False
     * \endcode
     */
    bool isScalene( double lengthTolerance = 0.0001 ) const;

    /**
     * An altitude is a segment (defined by a QgsLineString) from a vertex to the opposite side (or, if necessary, to the extension of the opposite side).
     * \returns Three altitudes from this triangle.
     * An empty list is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   [alt.asWkt() for alt in tri.altitudes()]
     *   # ['LineString (0 0, 0 5)', 'LineString (0 5, 2.5 2.5)', 'LineString (5 5, 0 5)']
     *   QgsTriangle().altitudes()
     *   # []
     * \endcode
     */
    QVector<QgsLineString> altitudes() const;

    /**
     * A median is a segment (defined by a QgsLineString) from a vertex to the midpoint of the opposite side.
     * \returns Three medians from this triangle.
     * An empty list is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   [med.asWkt() for med in tri.medians()]
     *   # ['LineString (0 0, 2.5 5)', 'LineString (0 5, 2.5 2.5)', 'LineString (5 5, 0 2.5)']
     *   QgsTriangle().medians()
     *   # []
     * \endcode
     */
    QVector<QgsLineString> medians() const;

    /**
     * The segment (defined by a QgsLineString) returned bisect the angle of a vertex to the opposite side.
     * \param lengthTolerance The tolerance to use.
     * \returns Three angle bisector from this triangle.
     * An empty list is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   [bis.asWkt() for bis in tri.bisectors()]
     *   # ['LineString (0 0, 2.07106781186547462 5)', 'LineString (0 5, 2.5 2.5)', 'LineString (5 5, 0 2.92893218813452538)']
     *   QgsTriangle().bisectors()
     *   # []
     * \endcode
     */
    QVector<QgsLineString> bisectors( double lengthTolerance = 0.0001 ) const;

    /**
     * Medial (or midpoint) triangle of a triangle ABC is the triangle with vertices at the midpoints of the triangle's sides.
     * \returns The medial from this triangle.
     * An empty triangle is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.medial().asWkt()
     *   # 'Triangle ((0 2.5, 2.5 5, 2.5 2.5, 0 2.5))'
     *   QgsTriangle().medial().asWkt()
     *   # 'Triangle ( )'
     * \endcode
     */
    QgsTriangle medial() const;

    /**
     * An orthocenter is the point of intersection of the altitudes of a triangle.
     * \param lengthTolerance The tolerance to use
     * \returns The orthocenter of the triangle.
     * An empty point is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.orthocenter().asWkt()
     *   # 'Point (0 5)'
     *   QgsTriangle().orthocenter().asWkt()
     *   # 'Point (0 0)'
     * \endcode
     */
    QgsPoint orthocenter( double lengthTolerance = 0.0001 ) const;

    /**
     * Center of the circumscribed circle of the triangle.
     * \returns The center of the circumscribed circle of the triangle.
     * An empty point is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.circumscribedCenter().asWkt()
     *   # 'Point (2.5 2.5)'
     *   QgsTriangle().circumscribedCenter().asWkt()
     *   # 'Point (0 0)'
     * \endcode
     */
    QgsPoint circumscribedCenter() const;

    /**
     * Radius of the circumscribed circle of the triangle.
     * \returns The radius of the circumscribed circle of the triangle.
     * 0.0 is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.circumscribedRadius()
     *   # 3.5355339059327378
     *   QgsTriangle().circumscribedRadius()
     *   # 0.0
     * \endcode
     */
    double circumscribedRadius() const;

    /**
     * Circumscribed circle of the triangle.
     * \returns The circumbscribed of the triangle with a QgsCircle.
     * An empty circle is returned for empty triangle.
     * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.circumscribedCircle()
     *   # QgsCircle(Point (2.5 2.5), 3.5355339059327378, 0)
     *   QgsTriangle().circumscribedCircle()
     *   # QgsCircle()
     * \endcode
     */
    QgsCircle circumscribedCircle() const;

    /**
     * Center of the inscribed circle of the triangle. Z dimension is
     * supported and is retrieved from the first 3D point amongst vertices.
     * \returns The center of the inscribed circle of the triangle.
     * An empty point is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.inscribedCenter().asWkt()
     *   # 'Point (1.46446609406726225 3.53553390593273775)'
     *   QgsTriangle().inscribedCenter().asWkt()
     *   # 'Point (0 0)'
     * \endcode
     */
    QgsPoint inscribedCenter() const;

    /**
     * Radius of the inscribed circle of the triangle.
     * \returns The radius of the inscribed circle of the triangle.
     * 0.0 is returned for empty triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.inscribedRadius()
     *   # 1.4644660940672622
     *   QgsTriangle().inscribedRadius()
     *   # 0.0
     * \endcode
     */
    double inscribedRadius() const;

    /**
     * Inscribed circle of the triangle.
     * \returns The inscribed of the triangle with a QgsCircle.
     * An empty circle is returned for empty triangle.
     * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) )
     *   tri.inscribedCircle()
     *   # QgsCircle(Point (1.46446609406726225 3.53553390593273775), 1.4644660940672622, 0)
     *   QgsTriangle().inscribedCircle()
     *   # QgsCircle()
     * \endcode
     */
    QgsCircle inscribedCircle() const;

#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsTriangle.
     * Should be used by qgsgeometry_cast<QgsTriangle *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsTriangle *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::Triangle )
        return static_cast<const QgsTriangle *>( geom );
      return nullptr;
    }
#endif

    QgsTriangle *createEmptyWithSameType() const override SIP_FACTORY;


};
#endif // QGSTRIANGLE_H
