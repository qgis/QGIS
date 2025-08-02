/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_basis.h"
#include "qwt_spline_parametrization.h"
#include <qpainterpath.h>

#if 0
static QPolygonF qwtBasisUniformKnots( const QPolygonF& points )
{
    const int n = points.size();

    if ( n < 3 )
        return points;

    QVector< double > u( n - 2 );
    QVector< double > kx( n - 2 );
    QVector< double > ky( n - 2 );

    u[n - 3] = 4.0;
    kx[n - 3] = 6.0 * points[n - 2].x() - points[n - 1].x();
    ky[n - 3] = 6.0 * points[n - 2].y() - points[n - 1].y();

    for ( int i = n - 4; i >= 0; i-- )
    {
        u[i] = 4.0 - 1.0 / u[i + 1];
        kx[i] = 6.0 * points[i + 1].x() - kx[i + 1] / u[i + 1];
        ky[i] = 6.0 * points[i + 1].y() - ky[i + 1] / u[i + 1];
    }

    QVector< QPointF > knots( n );

    knots[0] = points[0];

    for ( int i = 1; i < n - 1; i++ )
    {
        knots[i].rx() = ( kx[i - 1] - knots[i - 1].x() ) / u[i - 1];
        knots[i].ry() = ( ky[i - 1] - knots[i - 1].y() ) / u[i - 1];
    }

    knots[n - 1] = points[n - 1];

    return knots;
}
#endif

#if 0
static inline void qwtSplineBezierControlPoints( const QwtSplineParametrization* param,
    const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4,
    QPointF& cp1, QPointF& cp2 )
{
    const double t1 = param->valueIncrement( p1, p2 );
    const double t2 = param->valueIncrement( p2, p3 );
    const double t3 = param->valueIncrement( p3, p4 );

    const double t123 = t1 + t2 + t3;

    cp1 = ( t2 + t3 ) / t123 * p2 + t1 / t123 * p3;
    cp2 = ( t3 * p2 + ( t1 + t2 ) * p3 ) / t123;
}
#endif

static QPainterPath qwtSplineBasisPathUniform( const QPolygonF& points,
    QwtSpline::BoundaryType boundaryType )
{
    const int n = points.size();
    const QPointF* pd = points.constData();

    QPainterPath path;

    QPointF cp1 = ( 2.0 * pd[0] + pd[1] ) / 3.0;;

    if ( boundaryType == QwtSpline::ConditionalBoundaries )
    {
        path.moveTo( pd[0] );
    }
    else
    {
        const QPointF cpN = ( pd[n - 1] + 2.0 * pd[0] ) / 3.0;
        path.moveTo( 0.5 * ( cpN + cp1 ) );
    }

    for ( int i = 1; i < n - 1; i++ )
    {
        const QPointF cp2 = ( pd[i - 1] + 2.0 * pd[i] ) / 3.0;
        const QPointF cp3 = ( 2.0 * pd[i] + pd[i + 1] ) / 3.0;

        path.cubicTo( cp1, cp2, 0.5 * ( cp2 + cp3 ) );

        cp1 = cp3;
    }

    if ( boundaryType == QwtSpline::ConditionalBoundaries )
    {
        const QPointF cp2 = ( pd[n - 2] + 2.0 * pd[n - 1] ) / 3.0;
        path.cubicTo( cp1, cp2, pd[n - 1] );
    }
    else
    {
        const QPointF cp2 = ( pd[n - 2] + 2.0 * pd[n - 1] ) / 3.0;
        const QPointF cp3 = ( 2.0 * pd[n - 1] + pd[0] ) / 3.0;

        path.cubicTo( cp1, cp2, 0.5 * ( cp2 + cp3 ) );

        if ( boundaryType == QwtSpline::ClosedPolygon )
        {
            const QPointF cp4 = ( pd[n - 1] + 2.0 * pd[0] ) / 3.0;
            const QPointF cp5 = ( 2.0 * pd[0] + pd[1] ) / 3.0;

            path.cubicTo( cp3, cp4, 0.5 * ( cp4 + cp5 ) );
        }
    }

    return path;
}

static QPainterPath qwtSplineBasisPath( const QPolygonF& points,
    const QwtSplineParametrization* param,
    QwtSpline::BoundaryType boundaryType )
{
    const int n = points.size();
    const QPointF* pd = points.constData();

    QPointF p0;

    double t1 = param->valueIncrement( pd[0], pd[1] );
    double t2 = param->valueIncrement( pd[1], pd[2] );

    double t0;
    if ( boundaryType == QwtSpline::ConditionalBoundaries )
        t0 = t1;
    else
        t0 = param->valueIncrement( pd[n - 1], pd[0] );

    double t012 = t0 + t1 + t2;
    QPointF cp1 = ( ( t1 + t2 ) * pd[0] + t0 * pd[1] ) / t012;

    if ( boundaryType == QwtSpline::ConditionalBoundaries )
    {
        p0 = pd[0];
    }
    else
    {
        const double tN = param->valueIncrement( pd[n - 2], pd[n - 1] );
        const QPointF cpN = ( t1 * pd[n - 1] + ( tN + t0 ) * pd[0] ) / ( tN + t0 + t1 );

        p0 = ( t1 * cpN + t0 * cp1 ) / ( t0 + t1 );
    }

    QPainterPath path;
    path.moveTo( p0 );

    for ( int i = 1; i < n - 2; i++ )
    {
        const double t3 = param->valueIncrement( pd[i + 1], pd[i + 2] );
        const double t123 = t1 + t2 + t3;

        const QPointF cp2 = ( t2 * pd[i - 1] + ( t0 + t1 ) * pd[i] ) / t012;
        const QPointF cp3 = ( ( t2 + t3 ) * pd[i] + t1 * pd[i + 1] ) / t123;

        const QPointF p2 = ( t2 * cp2 + t1 * cp3 ) / ( t1 + t2 );

        path.cubicTo( cp1, cp2, p2 );

        cp1 = cp3;

        t0 = t1;
        t1 = t2;
        t2 = t3;
        t012 = t123;
    }

    {
        double t3;
        if ( boundaryType == QwtSpline::ConditionalBoundaries )
            t3 = t2;
        else
            t3 = param->valueIncrement( pd[n - 1], pd[0] );

        const double t123 = t1 + t2 + t3;

        const QPointF cp2 = ( t2 * pd[n - 3] + ( t0 + t1 ) * pd[n - 2] ) / t012;
        const QPointF cp3 = ( ( t2 + t3 ) * pd[n - 2] + t1 * pd[n - 1] ) / t123;

        const QPointF p2 = ( t2 * cp2 + t1 * cp3 ) / ( t1 + t2 );

        path.cubicTo( cp1, cp2, p2 );

        cp1 = cp3;

        t0 = t1;
        t1 = t2;
        t2 = t3;
        t012 = t123;
    }

    const QPointF cp2 = ( t2 * pd[n - 2] + ( t0 + t1 ) * pd[n - 1] ) / t012;

    if ( boundaryType == QwtSpline::ConditionalBoundaries )
    {
        path.cubicTo( cp1, cp2, pd[n - 1] );
    }
    else
    {
        const double t3 = param->valueIncrement( pd[0], pd[1] );
        const double t123 = t1 + t2 + t3;

        const QPointF cp3 = ( t2 + t3 ) / t123 * pd[n - 1] + t1 / t123 * pd[0];
        const QPointF cp4 = ( t3 * pd[n - 1] + ( t1 + t2 ) * pd[0] ) / t123;

        const QPointF pN = ( t2 * cp2 + t1 * cp3 ) / ( t1 + t2 );

        path.cubicTo( cp1, cp2, pN );
        path.cubicTo( cp3, cp4, p0 );
    }

    return path;
}

//! Constructor
QwtSplineBasis::QwtSplineBasis()
{
}

//! Destructor
QwtSplineBasis::~QwtSplineBasis()
{
}

//! The locality is always 2
uint QwtSplineBasis::locality() const
{
    return 2;
}

/*!
   Approximates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   \param points Control points
   \return Painter path, that can be rendered by QPainter
 */
QPainterPath QwtSplineBasis::painterPath( const QPolygonF& points ) const
{
    if ( points.size() < 4 )
        return QPainterPath();

    QPainterPath path;

    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterUniform:
        {
            path = qwtSplineBasisPathUniform( points, boundaryType() );
            break;
        }
        default:
        {
            path = qwtSplineBasisPath( points, parametrization(), boundaryType() );
        }
    }

    return path;
}
