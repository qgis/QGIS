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
#include "qgspolygon.h"
#include "qgslinestring.h"

/** \ingroup core
 * \class QgsTriangle
 * \brief Triangle geometry type.
 * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsTriangle : public QgsPolygonV2
{
  public:
    QgsTriangle();

    /** Construct a QgsTriangle from three QgsPointV2.
     * An empty triangle is returned if there are identical points or if the points are collinear.
     * @param p1 first point
     * @param p2 second point
     * @param p3 third point
     */
    QgsTriangle( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3 );

    /** Construct a QgsTriangle from three QgsPoint.
     * An empty triangle is returned if there are identical points or if the points are collinear.
     * @param p1 first point
     * @param p2 second point
     * @param p3 third point
     */
    explicit QgsTriangle( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3 );

    /** Construct a QgsTriangle from three QPointF.
     * An empty triangle is returned if there are identical points or if the points are collinear.
     * @param p1 first point
     * @param p2 second point
     * @param p3 third point
     */
    explicit QgsTriangle( const QPointF p1, const QPointF p2, const QPointF p3 );

    // inherited: bool operator==( const QgsTriangle& other ) const;
    // inherited: bool operator!=( const QgsTriangle& other ) const;

    virtual QString geometryType() const override { return QStringLiteral( "Triangle" ); }
    virtual QgsTriangle* clone() const override;
    void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr& wkbPtr ) override;

    bool fromWkt( const QString &wkt ) override;

    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    QgsPolygonV2* surfaceToPolygon() const override;

    QgsAbstractGeometry* toCurveType() const override;

    //! Inherited method not used. You cannot add an interior ring into a triangle.
    void addInteriorRing( QgsCurve* ring ) override;
    //! Inherited method not used. You cannot add an interior ring into a triangle.
    void setInteriorRings( const QList< QgsCurve *> &rings ) = delete;
    //! Inherited method not used. You cannot delete or insert a vertex directly. Returns always false.
    bool deleteVertex( QgsVertexId position ) override;
    //! Inherited method not used. You cannot delete or insert a vertex directly. Returns always false.
    bool insertVertex( QgsVertexId position, const QgsPointV2 &vertex ) override;
    bool moveVertex( QgsVertexId vId, const QgsPointV2& newPos ) override;

    virtual void setExteriorRing( QgsCurve* ring ) override;

    virtual QgsAbstractGeometry* boundary() const override;

    // inherited: double pointDistanceToBoundary( double x, double y ) const;

    /**
     *  Returns coordinates of a vertex.
     *  @param atVertex index of the vertex
     *  @return Coordinates of the vertex or QgsPointV2(0,0) on error (\a atVertex < 0 or > 3).
     */
    QgsPointV2 vertexAt( int atVertex ) const;

    /**
     * Returns the three lengths of the triangle.
     * @return Lengths of triangle ABC where [AB] is at 0, [BC] is at 1, [CA] is at 2
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.lengths()
     *   # [5.0, 5.0, 7.0710678118654755]
     * \endcode
     */
    QVector<double> lengths() const;

    /**
     * Returns the three angles of the triangle.
     * @return Angles in radians of triangle ABC where angle BAC is at 0, angle ABC is at 1, angle BCA is at 2
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   [math.degrees(i) for i in tri.angles()]
     *   # [45.0, 90.0, 45.0]
     * \endcode
     */
    QVector<double> angles() const;

    /**
     * Is the triangle isocele (two sides with the same length)?
     * @param lengthTolerance The tolerance to use
     * @return True or False
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.lengths()
     *   # [5.0, 5.0, 7.0710678118654755]
     *   tri.isIsocele()
     *   # True
     *   # length of [AB] == length of [BC]
     * \endcode
     */
    bool isIsocele( double lengthTolerance = 0.0001 ) const;

    /**
     * Is the triangle equilateral (three sides with the same length)?
     * @param lengthTolerance The tolerance to use
     * @return True or False
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 10, 10 ), QgsPointV2( 16, 10 ), QgsPointV2( 13, 15.1962 ) )
     *   tri.lengths()
     *   # [6.0, 6.0000412031918575, 6.0000412031918575]
     *   tri.isEquilateral()
     *   # True
     *   # All lengths are close to 6.0
     * \endcode
     */
    bool isEquilateral( double lengthTolerance = 0.0001 ) const;

    /**
     * Is the triangle right-angled?
     * @param angleTolerance The tolerance to use
     * @return True or False
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   [math.degrees(i) for i in tri.angles()]
     *   # [45.0, 90.0, 45.0]
     *   tri.isRight()
     *   # True
     *   # angle of ABC == 90
     * \endcode
     */
    bool isRight( double angleTolerance = 0.0001 ) const;

    /**
     * Is the triangle scalene (all sides have differen lengths)?
     * @param lengthTolerance The tolerance to use
     * @return True or False
     * @return True or False
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 7.2825, 4.2368 ), QgsPointV2( 13.0058, 3.3218 ), QgsPointV2( 9.2145, 6.5242 ) )
     *   tri.lengths()
     *   # [5.795980321740233, 4.962793714229921, 2.994131386562721]
     *   tri.isScalene()
     *   # True
     *   # All lengths are different
     * \endcode
     */
    bool isScalene( double lengthTolerance = 0.0001 ) const;

    /**
     * An altitude is a segment (defined by a QgsLineString) from a vertex to the opposite side (or, if necessary, to the extension of the opposite side).
     * @return Three altitudes from this triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   [alt.asWkt() for alt in tri.altitudes()]
     *   # ['LineString (0 0, 0 5)', 'LineString (0 5, 2.5 2.5)', 'LineString (5 5, 0 5)']
     * \endcode
     */
    QVector<QgsLineString> altitudes( ) const;

    /**
     * A median is a segment (defined by a QgsLineString) from a vertex to the midpoint of the opposite side.
     * @return Three medians from this triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   [med.asWkt() for med in tri.medians()]
     *   # ['LineString (0 0, 2.5 5)', 'LineString (0 5, 2.5 2.5)', 'LineString (5 5, 0 2.5)']
     * \endcode
     */
    QVector<QgsLineString> medians( ) const;

    /**
     * The segment (defined by a QgsLineString) returned bisect the angle of a vertex to the opposite side.
     * @param lengthTolerance The tolerance to use
     * @return Three angle bisector from this triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   [bis.asWkt() for bis in tri.bisectors()]
     *   # ['LineString (0 0, 2.07106781186547462 5)', 'LineString (0 5, 2.5 2.5)', 'LineString (5 5, 0 2.92893218813452538)']
     * \endcode
     */
    QVector<QgsLineString> bisectors( double lengthTolerance = 0.0001 ) const;

    /**
     * Medial (or midpoint) triangle of a triangle ABC is the triangle with vertices at the midpoints of the triangle's sides.
     * @return The medial from this triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.medial().asWkt()
     *   # 'Triangle ((0 2.5, 2.5 5, 2.5 2.5, 0 2.5))'
     * \endcode
     */
    QgsTriangle medial( ) const;

    /**
     * An orthocenter is the point of intersection of the altitudes of a triangle.
     * @param lengthTolerance The tolerance to use
     * @return The orthocenter of the triangle.
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.orthocenter().asWkt()
     *   # 'Point (0 5)'
     * \endcode
     */
    QgsPointV2 orthocenter( double lengthTolerance = 0.0001 ) const;

    /**
     * Center of the circumscribed circle of the triangle.
     * @return The center of the circumscribed circle of the triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.circumscribedCenter().asWkt()
     *   # 'Point (2.5 2.5)'
     * \endcode
     */
    QgsPointV2 circumscribedCenter( ) const;

    /**
     * Radius of the circumscribed circle of the triangle.
     * @return The radius of the circumscribed circle of the triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.circumscribedRadius()
     *   # 3.5355339059327378
     * \endcode
     */
    double circumscribedRadius( ) const;

    // TODO:
    // QgsCircle circumscribedCircle ( ) const; // need QgsCircle (from CADDigitize.CADCircle)

    /**
     * Center of the inscribed circle of the triangle.
     * @return The center of the inscribed circle of the triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.inscribedCenter().asWkt()
     *   # 'Point (1.46446609406726225 3.53553390593273775)'
     * \endcode
     */
    QgsPointV2 inscribedCenter( ) const;

    /**
     * Radius of the inscribed circle of the triangle.
     * @return The radius of the inscribed circle of the triangle
     * * Example:
     * \code{.py}
     *   tri = QgsTriangle( QgsPointV2( 0, 0 ), QgsPointV2( 0, 5 ), QgsPointV2( 5, 5 ) )
     *   tri.inscribedRadius()
     *   # 1.4644660940672622
     * \endcode
     */
    double inscribedRadius( ) const;

    // TODO:
    // QgsCircle inscribedCircle ( ) const; // need QgsCircle (from CADDigitize.CADCircle)
  private:

    /**
     * @brief Convenient method checking the validity of geometry (no duplicate point(s), no colinearity).
     * @param p1 first point
     * @param p2 second point
     * @param p3 third point
     * @return True if the points can create a triangle, otherwise false.
     */
    bool validateGeom( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3 );

};
#endif // QGSTRIANGLE_H
