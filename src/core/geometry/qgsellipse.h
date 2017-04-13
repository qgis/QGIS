/***************************************************************************
                         qgsellipse.h
                         --------------
    begin                : March 2017
    copyright            : (C) 2017 by Loîc Bartoletti
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

#ifndef QGSELLIPSE_H
#define QGSELLIPSE_H

#include <QString>

#include "qgis_core.h"
#include "qgspointv2.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsrectangle.h"

/** \ingroup core
 * \class QgsEllipse
 * \brief Ellipse geometry type.
 *
 * An ellipse is defined by a center point (mCenter) with a semi-major axis, a semi-minor axis and an azimuth.
 * The azimuth is the north angle to the first quadrant (always oriented on the semi-major axis), in degrees. By default, the semi-major axis is oriented to the east (90 degrees).
 * The semi-minor axis is always smaller than the semi-major axis. If it is set larger, it will be swapped and the azimuth will increase by 90 degrees.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsEllipse
{

  public:
    QgsEllipse();

    /** Constructs an ellipse by defining all the members.
     * @param center The center of the ellipse.
     * @param semiMajorAxis Semi-major axis of the ellipse.
     * @param semiMinorAxis Semi-minor axis of the ellipse.
     * @param azimuth Angle in degrees started from the North to the first quadrant.
     */
    QgsEllipse( const QgsPointV2 &center, const double semiMajorAxis, const double semiMinorAxis, const double azimuth = 90 );

    /**
     * Constructs an ellipse by foci (\a pt1 and \a pt2) and a point \a pt3.
     * The center point can have z and m values which are the result from the midpoint operation between \a pt1 and \a pt2.
     * Axes are calculated from the 2D distance with the third point \a pt3.
     * The azimuth is the angle between \a pt1 and \a pt2.
     * @param pt1 First focus.
     * @param pt2 Second focus.
     * @param pt3 A point to calculate the axes.
     */
    static QgsEllipse fromFoci( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QgsPointV2 &pt3 );

    /**
     * Constructs an ellipse by an extent (aka bounding box / QgsRectangle).
     * The center point can have z and m values which are the result from the midpoint operation between \a pt1 and \a pt2.
     * Axes are calculated from the 2D distance between \a pt1 and \a pt2.
     * The azimuth always takes the default value.
     * @param pt1 First corner.
     * @param pt2 Second corner.
     */
    static QgsEllipse byExtent( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );

    /**
     * Constructs an ellipse by a center point and a another point.
     * The center point keep z and m values from \a ptc.
     * Axes are calculated from the 2D distance between \a ptc and \a pt1.
     * The azimuth always takes the default value.
     * @param ptc Center point.
     * @param pt1 First point.
     */
    static QgsEllipse byCenterPoint( const QgsPointV2 &ptc, const QgsPointV2 &pt1 );

    /**
     * Constructs an ellipse by a central point and two other points.
     * The center point keep z and m values from \a ptc.
     * Axes are calculated from the 2D distance between \a ptc and \a pt1 and \a pt2.
     * The azimuth is the angle between \a ptc and \a pt1.
     * @param ptc Center point.
     * @param pt1 First point.
     * @param pt2 Second point.
     */
    static QgsEllipse byCenter2Points( const QgsPointV2 &ptc, const QgsPointV2 &pt1, const QgsPointV2 &pt2 );

    virtual bool operator ==( const QgsEllipse &elp ) const;
    virtual bool operator !=( const QgsEllipse &elp ) const;

    //! An ellipse is empty if axes are equal to 0
    virtual bool isEmpty() const;

    /** Returns the center point.
     * @see setCenter()
     * @see rcenter()
     */
    QgsPointV2 center() const {return mCenter; }

    /** Returns the semi-major axis.
     * @see setSemiMajorAxis()
     */
    double semiMajorAxis() const {return mSemiMajorAxis; }

    /** Returns the semi-minor axis.
     * @see setSemiMinorAxis()
     */
    double semiMinorAxis() const {return mSemiMinorAxis; }

    /** Returns the azimuth.
     * @see setAzimuth()
     */
    double azimuth() const {return mAzimuth; }

    /** Returns a reference to the center point of this ellipse.
     * Using a reference makes it possible to directly manipulate center in place.
     * @see center()
     * @see setCenter()
     * @note not available in Python bindings
     */
    QgsPointV2 &rcenter() { return mCenter; }

    /** Sets the center point.
     * @see center()
     * @see rcenter()
     */
    void setCenter( const QgsPointV2 &center ) {mCenter = center; }

    /** Sets the semi-major axis.
     * @see semiMajorAxis()
     */
    virtual void setSemiMajorAxis( const double semiMajorAxis );

    /** Sets the semi-minor axis.
     * @see semiMinorAxis()
     */
    virtual void setSemiMinorAxis( const double semiMinorAxis );

    /** Sets the azimuth (orientation).
     * @see azimuth()
     */
    void setAzimuth( const double azimuth );

    /** The distance between the center and each foci.
     * @see fromFoci()
     * @see foci()
     * @return The distance between the center and each foci.
     */
    virtual double focusDistance() const;

    /** Two foci of the ellipse. The axes are oriented by the azimuth and are on the semi-major axis.
     * @see fromFoci()
     * @see focusDistance()
     * @return the two foci.
     */
    virtual QVector<QgsPointV2> foci() const;

    //! The eccentricity of the ellipse.
    virtual double eccentricity() const;
    //! The area of the ellipse.
    virtual double area() const;
    //! The circumference of the ellipse using first approximation of Ramanujan.
    virtual double perimeter() const;

    /** The four quadrant of the ellipse.
     * The are oriented and started always from semi-major axis.
     * @return quadrant defined by four point.
     */
    virtual QVector<QgsPointV2> quadrant() const;

    /** Returns a list of points into \a pts, with segmentation from \a segments.
     * @param pts List of points returned.
     * @param segments Number of segments used to segment geometry.
     */
    virtual void points( QgsPointSequence &pts, unsigned int segments = 36 ) const;

    /** Returns a segmented polygon.
     * @param segments Number of segments used to segment geometry.
     */
    virtual QgsPolygonV2 toPolygon( unsigned int segments = 36 ) const;

    /** Returns a segmented linestring.
     * @param segments Number of segments used to segment geometry.
     */
    virtual QgsLineString toLineString( unsigned int segments = 36 ) const;
    //virtual QgsCurvePolygon toCurvePolygon() const;

    /** Returns the oriented minimal bounding box for the ellipse.
     */
    virtual QgsPolygonV2 orientedBoundingBox() const;

    /** Returns the minimal bounding box for the ellipse.
     */
    virtual QgsRectangle boundingBox() const;

    /**
     * returns a string representation of the ellipse.
     * Members will be truncated to the specified precision.
     */
    virtual QString toString( int pointPrecision = 17, int axisPrecision = 17, int azimuthPrecision = 2 ) const;

  protected:
    QgsPointV2 mCenter;
    double mSemiMajorAxis;
    double mSemiMinorAxis;
    double mAzimuth;

  private:
    //! The semi-minor axis is always smaller than the semi-major axis. If it is set larger, it will be swapped and the azimuth will increase by 90 degrees.
    void normalizeAxis();
};

#endif // QGSELLIPSE_H
