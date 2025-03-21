/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_POLYNOMIAL_H
#define QWT_SPLINE_POLYNOMIAL_H

#include "qwt_global.h"

#include <qpoint.h>
#include <qmetatype.h>

/*!
   \brief A cubic polynomial without constant term

   QwtSplinePolynomial is a 3rd degree polynomial
   of the form: y = c3 * x³ + c2 * x² + c1 * x;

   QwtSplinePolynomial is usually used in combination with polygon
   interpolation, where it is not necessary to store a constant term ( c0 ),
   as the translation is known from the corresponding polygon points.

   \sa QwtSplineC1
 */
class QWT_EXPORT QwtSplinePolynomial
{
  public:
    QwtSplinePolynomial( double c3 = 0.0, double c2 = 0.0, double c1 = 0.0 );

    bool operator==( const QwtSplinePolynomial& ) const;
    bool operator!=( const QwtSplinePolynomial& ) const;

    double valueAt( double x ) const;
    double slopeAt( double x ) const;
    double curvatureAt( double x ) const;

    static QwtSplinePolynomial fromSlopes(
        const QPointF& p1, double m1,
        const QPointF& p2, double m2 );

    static QwtSplinePolynomial fromSlopes(
        double x, double y, double m1, double m2 );

    static QwtSplinePolynomial fromCurvatures(
        const QPointF& p1, double cv1,
        const QPointF& p2, double cv2 );

    static QwtSplinePolynomial fromCurvatures(
        double dx, double dy, double cv1, double cv2 );

  public:
    //! coefficient of the cubic summand
    double c3;

    //! coefficient of the quadratic summand
    double c2;

    //! coefficient of the linear summand
    double c1;
};

Q_DECLARE_TYPEINFO( QwtSplinePolynomial, Q_MOVABLE_TYPE );
Q_DECLARE_METATYPE( QwtSplinePolynomial )

/*!
   \brief Constructor

   \param a3 Coefficient of the cubic summand
   \param a2 Coefficient of the quadratic summand
   \param a1 Coefficient of the linear summand
 */
inline QwtSplinePolynomial::QwtSplinePolynomial( double a3, double a2, double a1 )
    : c3( a3 )
    , c2( a2 )
    , c1( a1 )
{
}

/*!
   \param other Other polynomial
   \return true, when both polynomials have the same coefficients
 */
inline bool QwtSplinePolynomial::operator==( const QwtSplinePolynomial& other ) const
{
    return ( c3 == other.c3 ) && ( c2 == other.c2 ) && ( c1 == other.c1 );
}

/*!
   \param other Other polynomial
   \return true, when the polynomials have different coefficients
 */
inline bool QwtSplinePolynomial::operator!=( const QwtSplinePolynomial& other ) const
{
    return ( !( *this == other ) );
}

/*!
   Calculate the value of a polynomial for a given x

   \param x Parameter
   \return Value at x
 */
inline double QwtSplinePolynomial::valueAt( double x ) const
{
    return ( ( ( c3 * x ) + c2 ) * x + c1 ) * x;
}

/*!
   Calculate the value of the first derivate of a polynomial for a given x

   \param x Parameter
   \return Slope at x
 */
inline double QwtSplinePolynomial::slopeAt( double x ) const
{
    return ( 3.0 * c3 * x + 2.0 * c2 ) * x + c1;
}

/*!
   Calculate the value of the second derivate of a polynomial for a given x

   \param x Parameter
   \return Curvature at x
 */
inline double QwtSplinePolynomial::curvatureAt( double x ) const
{
    return 6.0 * c3 * x + 2.0 * c2;
}

/*!
   Find the coefficients for the polynomial including 2 points with
   specific values for the 1st derivates at these points.

   \param p1 First point
   \param m1 Value of the first derivate at p1
   \param p2 Second point
   \param m2 Value of the first derivate at p2

   \return Coefficients of the polynomials
   \note The missing constant term of the polynomial is p1.y()
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromSlopes(
    const QPointF& p1, double m1, const QPointF& p2, double m2 )
{
    return fromSlopes( p2.x() - p1.x(), p2.y() - p1.y(), m1, m2 );
}

/*!
   Find the coefficients for the polynomial from the offset between 2 points
   and specific values for the 1st derivates at these points.

   \param dx X-offset
   \param dy Y-offset
   \param m1 Value of the first derivate at p1
   \param m2 Value of the first derivate at p2

   \return Coefficients of the polynomials
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromSlopes(
    double dx, double dy, double m1, double m2 )
{
    const double c2 = ( 3.0 * dy / dx - 2 * m1 - m2 ) / dx;
    const double c3 = ( ( m2 - m1 ) / dx - 2.0 * c2 ) / ( 3.0 * dx );

    return QwtSplinePolynomial( c3, c2, m1 );
}

/*!
   Find the coefficients for the polynomial including 2 points with
   specific values for the 2nd derivates at these points.

   \param p1 First point
   \param cv1 Value of the second derivate at p1
   \param p2 Second point
   \param cv2 Value of the second derivate at p2

   \return Coefficients of the polynomials
   \note The missing constant term of the polynomial is p1.y()
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromCurvatures(
    const QPointF& p1, double cv1, const QPointF& p2, double cv2 )
{
    return fromCurvatures( p2.x() - p1.x(), p2.y() - p1.y(), cv1, cv2 );
}

/*!
   Find the coefficients for the polynomial from the offset between 2 points
   and specific values for the 2nd derivates at these points.

   \param dx X-offset
   \param dy Y-offset
   \param cv1 Value of the second derivate at p1
   \param cv2 Value of the second derivate at p2

   \return Coefficients of the polynomials
 */
inline QwtSplinePolynomial QwtSplinePolynomial::fromCurvatures(
    double dx, double dy, double cv1, double cv2 )
{
    const double c3 = ( cv2 - cv1 ) / ( 6.0 * dx );
    const double c2 = 0.5 * cv1;
    const double c1 = dy / dx - ( c3 * dx + c2 ) * dx;

    return QwtSplinePolynomial( c3, c2, c1 );
}

#ifndef QT_NO_DEBUG_STREAM

class QDebug;
QWT_EXPORT QDebug operator<<( QDebug, const QwtSplinePolynomial& );

#endif

#endif
