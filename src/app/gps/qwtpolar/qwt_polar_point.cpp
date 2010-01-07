/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <cmath>
#include <qglobal.h>
#include "qwt_math.h"
#include "qwt_polar_point.h"

/*! 
   Convert and assign values from a point in Cartesian coordinates

   \param p Point in Cartesian coordinates
   \sa setPoint(), toPoint()
*/
QwtPolarPoint::QwtPolarPoint(const QwtDoublePoint &p)
{
    d_radius = ::sqrt(qwtSqr(p.x()) + qwtSqr(p.y()) );
    d_azimuth = ::atan2(p.y(), p.x());
}

/*! 
   Convert and assign values from a point in Cartesian coordinates
   \param p Point in Cartesian coordinates
*/
void QwtPolarPoint::setPoint(const QwtDoublePoint &p)
{
    d_radius = ::sqrt(qwtSqr(p.x()) + qwtSqr(p.y()) );
    d_azimuth = ::atan2(p.y(), p.x());
}

/*! 
   Convert and return values in Cartesian coordinates

   \note Invalid or null points will be returned as QwtDoublePoint(0.0, 0.0)
   \sa isValid(), isNull()
*/
QwtDoublePoint QwtPolarPoint::toPoint() const
{
    if ( d_radius <= 0.0 )
        return QwtDoublePoint(0.0, 0.0);

    const double x = d_radius * ::cos(d_azimuth);
    const double y = d_radius * ::sin(d_azimuth);

    return QwtDoublePoint(x, y);
}

/*!
    Returns true if point1 is equal to point2; otherwise returns false.

    Two points are equal to each other if radius and
    azimuth-coordinates are the same. Points are not equal, when 
    the azimuth differs, but other.azimuth() == azimuth() % (2 * PI).

    \sa normalized()
*/
bool QwtPolarPoint::operator==(const QwtPolarPoint &other) const
{
    return d_radius == other.d_radius && d_azimuth == other.d_azimuth;
}

/*!
    Returns true if point1 is not equal to point2; otherwise returns false.

    Two points are equal to each other if radius and
    azimuth-coordinates are the same. Points are not equal, when 
    the azimuth differs, but other.azimuth() == azimuth() % (2 * PI).

    \sa normalized()
*/
bool QwtPolarPoint::operator!=(const QwtPolarPoint &other) const
{
    return d_radius != other.d_radius || d_azimuth != other.d_azimuth;
}

/*!
   Normalize radius and azimuth

   When the radius is < 0.0 it is set to 0.0. The azimuth is
   a value >= 0.0 and < 2 * M_PI.
*/
QwtPolarPoint QwtPolarPoint::normalized() const
{
    const double radius = qwtMax(d_radius, 0.0);

    double azimuth = d_azimuth;
    if ( azimuth < -2.0 * M_PI || azimuth >= 2 * M_PI )
        azimuth = ::fmod(d_azimuth, 2 * M_PI);

    if ( azimuth < 0.0 )
        azimuth += 2 * M_PI;

    return QwtPolarPoint(azimuth, radius);
}
