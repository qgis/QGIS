/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_MATH_H
#define QWT_MATH_H

#include "qwt_global.h"

/*
   Microsoft says:

   Define _USE_MATH_DEFINES before including math.h to expose these macro
   definitions for common math constants.  These are placed under an #ifdef
   since these commonly-defined names are not part of the C/C++ standards.
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#define undef_USE_MATH_DEFINES
#endif

#include <cmath>

#ifdef undef_USE_MATH_DEFINES
#undef _USE_MATH_DEFINES
#undef undef_USE_MATH_DEFINES
#endif

#ifndef M_E
#define M_E ( 2.7182818284590452354 )
#endif

#ifndef M_LOG2E
#define M_LOG2E ( 1.4426950408889634074 )
#endif

#ifndef M_LOG10E
#define M_LOG10E ( 0.43429448190325182765 )
#endif

#ifndef M_LN2
#define M_LN2 ( 0.69314718055994530942 )
#endif

#ifndef M_LN10
#define M_LN10 ( 2.30258509299404568402 )
#endif

#ifndef M_PI
#define M_PI ( 3.14159265358979323846 )
#endif

#ifndef M_PI_2
#define M_PI_2 ( 1.57079632679489661923 )
#endif

#ifndef M_PI_4
#define M_PI_4 ( 0.78539816339744830962 )
#endif

#ifndef M_1_PI
#define M_1_PI ( 0.31830988618379067154 )
#endif

#ifndef M_2_PI
#define M_2_PI ( 0.63661977236758134308 )
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI ( 1.12837916709551257390 )
#endif

#ifndef M_SQRT2
#define M_SQRT2 ( 1.41421356237309504880 )
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 ( 0.70710678118654752440 )
#endif

#if defined( QT_WARNING_PUSH )
    /*
        early Qt versions not having QT_WARNING_PUSH is full of warnings
        so that we do not care of suppressing those from below
     */
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdouble-promotion")
    QT_WARNING_DISABLE_GCC("-Wdouble-promotion")
#endif

/*
    On systems, where qreal is a float you often run into
    compiler issues with qMin/qMax.
 */

//! \return Minimum of a and b.
QWT_CONSTEXPR inline float qwtMinF( float a, float b )
{
    return ( a < b ) ? a : b;
}

//! \return Minimum of a and b.
QWT_CONSTEXPR inline double qwtMinF( double a, double b )
{
    return ( a < b ) ? a : b;
}

//! \return Minimum of a and b.
QWT_CONSTEXPR inline qreal qwtMinF( float a, double b )
{
    return ( a < b ) ? a : b;
}

//! \return Minimum of a and b.
QWT_CONSTEXPR inline qreal qwtMinF( double a, float b )
{
    return ( a < b ) ? a : b;
}

//! \return Maximum of a and b.
QWT_CONSTEXPR inline float qwtMaxF( float a, float b )
{
    return ( a < b ) ? b : a;
}

//! \return Maximum of a and b.
QWT_CONSTEXPR inline double qwtMaxF( double a, double b )
{
    return ( a < b ) ? b : a;
}

//! \return Maximum of a and b.
QWT_CONSTEXPR inline qreal qwtMaxF( float a, double b )
{
    return ( a < b ) ? b : a;
}

//! \return Maximum of a and b.
QWT_CONSTEXPR inline qreal qwtMaxF( double a, float b )
{
    return ( a < b ) ? b : a;
}

#if defined( QT_WARNING_POP )
    QT_WARNING_POP
#endif

QWT_EXPORT double qwtNormalizeRadians( double radians );
QWT_EXPORT double qwtNormalizeDegrees( double degrees );
QWT_EXPORT quint32 qwtRand();

/*!
   \brief Compare 2 values, relative to an interval

   Values are "equal", when :
   \f$\cdot value2 - value1 <= abs(intervalSize * 10e^{-6})\f$

   \param value1 First value to compare
   \param value2 Second value to compare
   \param intervalSize interval size

   \return 0: if equal, -1: if value2 > value1, 1: if value1 > value2
 */
inline int qwtFuzzyCompare( double value1, double value2, double intervalSize )
{
    const double eps = qAbs( 1.0e-6 * intervalSize );

    if ( value2 - value1 > eps )
        return -1;

    if ( value1 - value2 > eps )
        return 1;

    return 0;
}

//! Return the sign
inline int qwtSign( double x )
{
    if ( x > 0.0 )
        return 1;
    else if ( x < 0.0 )
        return ( -1 );
    else
        return 0;
}

//! Return the square of a number
inline double qwtSqr( double x )
{
    return x * x;
}

//! Approximation of arc tangent ( error below 0,005 radians )
inline double qwtFastAtan( double x )
{
    if ( x < -1.0 )
        return -M_PI_2 - x / ( x * x + 0.28 );

    if ( x > 1.0 )
        return M_PI_2 - x / ( x * x + 0.28 );

    return x / ( 1.0 + x * x * 0.28 );
}

//! Approximation of arc tangent ( error below 0,005 radians )
inline double qwtFastAtan2( double y, double x )
{
    if ( x > 0 )
        return qwtFastAtan( y / x );

    if ( x < 0 )
    {
        const double d = qwtFastAtan( y / x );
        return ( y >= 0 ) ? d + M_PI : d - M_PI;
    }

    if ( y < 0.0 )
        return -M_PI_2;

    if ( y > 0.0 )
        return M_PI_2;

    return 0.0;
}

/* !
   \brief Calculate a value of a cubic polynomial

   \param x Value
   \param a Cubic coefficient
   \param b Quadratic coefficient
   \param c Linear coefficient
   \param d Constant offset

   \return Value of the polyonom for x
 */
inline double qwtCubicPolynomial( double x,
    double a, double b, double c, double d )
{
    return ( ( ( a * x ) + b ) * x + c ) * x + d;
}

//! Translate degrees into radians
inline double qwtRadians( double degrees )
{
    return degrees * M_PI / 180.0;
}

//! Translate radians into degrees
inline double qwtDegrees( double degrees )
{
    return degrees * 180.0 / M_PI;
}

/*!
    The same as qCeil, but avoids including qmath.h
    \return Ceiling of value.
 */
inline int qwtCeil( qreal value )
{
    using std::ceil;
    return int( ceil( value ) );
}
/*!
    The same as qFloor, but avoids including qmath.h
    \return Floor of value.
 */
inline int qwtFloor( qreal value )
{
    using std::floor;
    return int( floor( value ) );
}

#endif
