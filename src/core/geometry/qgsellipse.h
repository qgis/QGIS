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
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsrectangle.h"

/**
 * \ingroup core
 * \class QgsEllipse
 * \brief Ellipse geometry type.
 *
 * An ellipse is defined by a center point with a semi-major axis, a semi-minor axis and an azimuth.
 * The azimuth is the north angle to the first quadrant (always oriented on the semi-major axis), in degrees. By default, the semi-major axis is oriented to the east (90 degrees).
 * The semi-minor axis is always smaller than the semi-major axis. If it is set larger, it will be swapped and the azimuth will increase by 90 degrees.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsEllipse
{

  public:

    /**
     * Constructor for QgsEllipse.
     */
    QgsEllipse() = default;

    virtual ~QgsEllipse() = default;

    /**
     * Constructs an ellipse by defining all the members.
     * \param center The center of the ellipse.
     * \param semiMajorAxis Semi-major axis of the ellipse.
     * \param semiMinorAxis Semi-minor axis of the ellipse.
     * \param azimuth Angle in degrees started from the North to the first quadrant.
     */
    QgsEllipse( const QgsPoint &center, double semiMajorAxis, double semiMinorAxis, double azimuth = 90 );

    /**
     * Constructs an ellipse by foci (\a pt1 and \a pt2) and a point \a pt3.
     * The center point can have m value which is the result from the midpoint
     * operation between \a pt1 and \a pt2. Z dimension is also supported and
     * is retrieved from the first 3D point amongst \a pt1 and \a pt2.
     * Axes are calculated from the 2D distance with the third point \a pt3.
     * The azimuth is the angle between \a pt1 and \a pt2.
     * \param pt1 First focus.
     * \param pt2 Second focus.
     * \param pt3 A point to calculate the axes.
     */
    static QgsEllipse fromFoci( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 );

    /**
     * Constructs an ellipse by an extent (aka bounding box / QgsRectangle).
     * The center point can have m value which is the result from the midpoint
     * operation between \a pt1 and \a pt2. Z dimension is also supported and
     * is retrieved from the first 3D point amongst \a pt1 and \a pt2.
     * Axes are calculated from the 2D distance between \a pt1 and \a pt2.
     * The azimuth always takes the default value.
     * \param pt1 First corner.
     * \param pt2 Second corner.
     */
    static QgsEllipse fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 );

    /**
     * Constructs an ellipse by a center point and a another point.
     * The center point keeps m value from \a ptc. Z dimension is also
     * supported and is retrieved from the first 3D point amongst \a ptc and
     * \a pt1.
     * Axes are calculated from the 2D distance between \a ptc and \a pt1.
     * The azimuth always takes the default value.
     * \param ptc Center point.
     * \param pt1 First point.
     */
    static QgsEllipse fromCenterPoint( const QgsPoint &ptc, const QgsPoint &pt1 );

    /**
     * Constructs an ellipse by a central point and two other points.
     * The center point keeps m value from \a ptc. Z dimension is also
     * supported and is retrieved from the first 3D point amongst \a ptc,
     * \a pt1 and \a pt2.
     * Axes are calculated from the 2D distance between \a ptc and \a pt1 and \a pt2.
     * The azimuth is the angle between \a ptc and \a pt1.
     * \param ptc Center point.
     * \param pt1 First point.
     * \param pt2 Second point.
     */
    static QgsEllipse fromCenter2Points( const QgsPoint &ptc, const QgsPoint &pt1, const QgsPoint &pt2 );

    virtual bool operator ==( const QgsEllipse &elp ) const;
    virtual bool operator !=( const QgsEllipse &elp ) const;

    //! An ellipse is empty if axes are equal to 0
    virtual bool isEmpty() const;

    /**
     * Returns the center point.
     * \see setCenter()
     * \see rcenter()
     */
    QgsPoint center() const {return mCenter; }

    /**
     * Returns the semi-major axis.
     * \see setSemiMajorAxis()
     */
    double semiMajorAxis() const {return mSemiMajorAxis; }

    /**
     * Returns the semi-minor axis.
     * \see setSemiMinorAxis()
     */
    double semiMinorAxis() const {return mSemiMinorAxis; }

    /**
     * Returns the azimuth.
     * \see setAzimuth()
     */
    double azimuth() const {return mAzimuth; }

    /**
     * Returns a reference to the center point of this ellipse.
     * Using a reference makes it possible to directly manipulate center in place.
     * \see center()
     * \see setCenter()
     * \note not available in Python bindings
     */
    QgsPoint &rcenter() SIP_SKIP { return mCenter; }

    /**
     * Sets the center point.
     * \see center()
     * \see rcenter()
     */
    void setCenter( const QgsPoint &center ) {mCenter = center; }

    /**
     * Sets the semi-major axis.
     * \see semiMajorAxis()
     */
    virtual void setSemiMajorAxis( double semiMajorAxis );

    /**
     * Sets the semi-minor axis.
     * \see semiMinorAxis()
     */
    virtual void setSemiMinorAxis( double semiMinorAxis );

    /**
     * Sets the azimuth (orientation).
     * \see azimuth()
     */
    void setAzimuth( double azimuth );

    /**
     * The distance between the center and each foci.
     * \see fromFoci()
     * \see foci()
     * \return The distance between the center and each foci.
     */
    virtual double focusDistance() const;

    /**
     * Two foci of the ellipse. The axes are oriented by the azimuth and are on the semi-major axis.
     * \see fromFoci()
     * \see focusDistance()
     * \return the two foci.
     */
    virtual QVector<QgsPoint> foci() const;

    /**
     * The eccentricity of the ellipse.
     * nan is returned if the ellipse is empty.
     */
    virtual double eccentricity() const;
    //! The area of the ellipse.
    virtual double area() const;
    //! The circumference of the ellipse using first approximation of Ramanujan.
    virtual double perimeter() const;

    /**
     * The four quadrants of the ellipse.
     * They are oriented and started always from semi-major axis.
     * \return quadrants defined by four points.
     */
    virtual QVector<QgsPoint> quadrant() const;

    /**
     * Returns a list of points with segmentation from \a segments.
     * \param segments Number of segments used to segment geometry.
     */
    virtual QgsPointSequence points( unsigned int segments = 36 ) const;

    /**
     * Returns a segmented polygon.
     * \param segments Number of segments used to segment geometry.
     */
    virtual QgsPolygon *toPolygon( unsigned int segments = 36 ) const SIP_FACTORY;

    /**
     * Returns a segmented linestring.
     * \param segments Number of segments used to segment geometry.
     */
    virtual QgsLineString *toLineString( unsigned int segments = 36 ) const SIP_FACTORY;
    //virtual QgsCurvePolygon toCurvePolygon() const;

    /**
     * Returns the oriented minimal bounding box for the ellipse.
     */
    virtual QgsPolygon *orientedBoundingBox() const SIP_FACTORY;

    /**
     * Returns the minimal bounding box for the ellipse.
     */
    virtual QgsRectangle boundingBox() const;

    /**
     * returns a string representation of the ellipse.
     * Members will be truncated to the specified precision.
     */
    virtual QString toString( int pointPrecision = 17, int axisPrecision = 17, int azimuthPrecision = 2 ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsEllipse: %1>" ).arg( sipCpp->toString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:
    QgsPoint mCenter;
    double mSemiMajorAxis = 0.0;
    double mSemiMinorAxis = 0.0;
    double mAzimuth = 90.0;

  private:
    //! The semi-minor axis is always smaller than the semi-major axis. If it is set larger, it will be swapped and the azimuth will increase by 90 degrees.
    void normalizeAxis();
};

#endif // QGSELLIPSE_H
