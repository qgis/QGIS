/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_PARAMETRIZATION_H
#define QWT_SPLINE_PARAMETRIZATION_H

#include "qwt_global.h"
#include "qwt_math.h"

#include <qpoint.h>

/*!
   \brief Curve parametrization used for a spline interpolation

   Parametrization is the process of finding a parameter value for
   each curve point - usually related to some physical quantity
   ( distance, time ... ).

   Often accumulating the curve length is the intended way of parametrization,
   but as the interpolated curve is not known in advance an approximation
   needs to be used.

   The values are calculated by cumulating increments, that are provided
   by QwtSplineParametrization. As the curve parameters need to be
   montonically increasing, each increment need to be positive.

   - t[0] = 0;
   - t[i] = t[i-1] + valueIncrement( point[i-1], p[i] );

   QwtSplineParametrization provides the most common used type of
   parametrizations and offers an interface to inject custom implementations.

   \note The most relevant types of parametrization are trying to provide an
        approximation of the curve length.

   \sa QwtSpline::setParametrization()
 */
class QWT_EXPORT QwtSplineParametrization
{
  public:
    //! Parametrization type
    enum Type
    {
        /*!
           No parametrization: t[i] = x[i]
           \sa valueIncrementX()
         */
        ParameterX,

        /*!
           No parametrization: t[i] = y[i]
           \sa valueIncrementY()
         */
        ParameterY,

        /*!
           Uniform parametrization: t[i] = i;

           A very fast parametrization, with good results, when the geometry
           of the control points is somehow "equidistant". F.e. when
           recording the position of a body, that is moving with constant
           speed every n seconds.

           \sa valueIncrementUniform()
         */
        ParameterUniform,

        /*!
           Parametrization using the chordal length between two control points

           The chordal length is the most commonly used approximation for
           the curve length.

           \sa valueIncrementChordal()
         */
        ParameterChordal,

        /*!
           Centripetal parametrization

           Based on the square root of the chordal length.

           Its name stems from the physical observations regarding
           the centripetal force, of a body moving along the curve.

           \sa valueIncrementCentripetal()
         */
        ParameterCentripetal,


        /*!
           Parametrization using the manhattan length between two control points

           Approximating the curve length by the manhattan length is faster
           than the chordal length, but usually gives worse results.

           \sa valueIncrementManhattan()
         */
        ParameterManhattan
    };

    explicit QwtSplineParametrization( int type );
    virtual ~QwtSplineParametrization();

    int type() const;

    virtual double valueIncrement( const QPointF&, const QPointF& ) const;

    static double valueIncrementX( const QPointF&, const QPointF& );
    static double valueIncrementY( const QPointF&, const QPointF& );
    static double valueIncrementUniform( const QPointF&, const QPointF& );
    static double valueIncrementChordal( const QPointF&, const QPointF& );
    static double valueIncrementCentripetal( const QPointF&, const QPointF& );
    static double valueIncrementManhattan( const QPointF&, const QPointF& );

  private:
    const int m_type;
};

/*!
   \brief Calculate the ParameterX value increment for 2 points

   \param point1 First point
   \param point2 Second point

   \return point2.x() - point1.x();
 */
inline double QwtSplineParametrization::valueIncrementX(
    const QPointF& point1, const QPointF& point2 )
{
    return point2.x() - point1.x();
}

/*!
   \brief Calculate the ParameterY value increment for 2 points

   \param point1 First point
   \param point2 Second point

   \return point2.y() - point1.y();
 */
inline double QwtSplineParametrization::valueIncrementY(
    const QPointF& point1, const QPointF& point2 )
{
    return point2.y() - point1.y();
}

/*!
   \brief Calculate the ParameterUniform value increment

   \param point1 First point
   \param point2 Second point

   \return 1.0
 */
inline double QwtSplineParametrization::valueIncrementUniform(
    const QPointF& point1, const QPointF& point2 )
{
    Q_UNUSED( point1 )
    Q_UNUSED( point2 )

    return 1.0;
}

/*!
   \brief Calculate the ParameterChordal value increment for 2 points

   \param point1 First point
   \param point2 Second point

   \return qSqrt( dx * dx + dy * dy );
 */
inline double QwtSplineParametrization::valueIncrementChordal(
    const QPointF& point1, const QPointF& point2 )
{
    const double dx = point2.x() - point1.x();
    const double dy = point2.y() - point1.y();

    return std::sqrt( dx * dx + dy * dy );
}

/*!
   \brief Calculate the ParameterCentripetal value increment for 2 points

   \param point1 First point
   \param point2 Second point

   \return The square root of a chordal increment
 */
inline double QwtSplineParametrization::valueIncrementCentripetal(
    const QPointF& point1, const QPointF& point2 )
{
    return std::sqrt( valueIncrementChordal( point1, point2 ) );
}

/*!
   \brief Calculate the ParameterManhattan value increment for 2 points

   \param point1 First point
   \param point2 Second point

   \return | point2.x() - point1.x() | + | point2.y() - point1.y() |
 */
inline double QwtSplineParametrization::valueIncrementManhattan(
    const QPointF& point1, const QPointF& point2 )
{
    return qAbs( point2.x() - point1.x() ) + qAbs( point2.y() - point1.y() );
}

#endif
