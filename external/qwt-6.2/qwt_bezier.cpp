/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_bezier.h"
#include "qwt_math.h"

#include <qpolygon.h>
#include <qstack.h>

namespace
{
    class BezierData
    {
      public:
        inline BezierData()
        {
            // default constructor with uninitialized points
        }

        inline BezierData( const QPointF& p1, const QPointF& cp1,
                const QPointF& cp2, const QPointF& p2 ):
            m_x1( p1.x() ),
            m_y1( p1.y() ),
            m_cx1( cp1.x() ),
            m_cy1( cp1.y() ),
            m_cx2( cp2.x() ),
            m_cy2( cp2.y() ),
            m_x2( p2.x() ),
            m_y2( p2.y() )
        {
        }

        static inline double minFlatness( double tolerance )
        {
            // we can simplify the tolerance criterion check in
            // the subdivision loop, by precalculating some
            // flatness value.

            return 16 * ( tolerance * tolerance );
        }

        inline double flatness() const
        {
            // algo by Roger Willcocks ( http://www.rops.org )

            const double ux = 3.0 * m_cx1 - 2.0 * m_x1 - m_x2;
            const double uy = 3.0 * m_cy1 - 2.0 * m_y1 - m_y2;
            const double vx = 3.0 * m_cx2 - 2.0 * m_x2 - m_x1;
            const double vy = 3.0 * m_cy2 - 2.0 * m_y2 - m_y1;

            const double ux2 = ux * ux;
            const double uy2 = uy * uy;

            const double vx2 = vx * vx;
            const double vy2 = vy * vy;

            return qwtMaxF( ux2, vx2 ) + qwtMaxF( uy2, vy2 );
        }

        inline BezierData subdivided()
        {
            BezierData bz;

            const double c1 = midValue( m_cx1, m_cx2 );

            bz.m_cx1 = midValue( m_x1, m_cx1 );
            m_cx2 = midValue( m_cx2, m_x2 );
            bz.m_x1 = m_x1;
            bz.m_cx2 = midValue( bz.m_cx1, c1 );
            m_cx1 = midValue( c1, m_cx2 );
            bz.m_x2 = m_x1 = midValue( bz.m_cx2, m_cx1 );

            const double c2 = midValue( m_cy1, m_cy2 );

            bz.m_cy1 = midValue( m_y1, m_cy1 );
            m_cy2 = midValue( m_cy2, m_y2 );
            bz.m_y1 = m_y1;
            bz.m_cy2 = midValue( bz.m_cy1, c2 );
            m_cy1 = midValue( m_cy2, c2 );
            bz.m_y2 = m_y1 = midValue( bz.m_cy2, m_cy1 );

            return bz;
        }

        inline QPointF p2() const
        {
            return QPointF( m_x2, m_y2 );
        }

      private:
        inline double midValue( double v1, double v2 )
        {
            return 0.5 * ( v1 + v2 );
        }

        double m_x1, m_y1;
        double m_cx1, m_cy1;
        double m_cx2, m_cy2;
        double m_x2, m_y2;
    };
}

/*!
   \brief Constructor

   \param tolerance Termination criterion for the subdivision
   \sa setTolerance()
 */

QwtBezier::QwtBezier( double tolerance )
    : m_tolerance( qwtMaxF( tolerance, 0.0 ) )
    , m_flatness( BezierData::minFlatness( m_tolerance ) )
{
}

//! Destructor
QwtBezier::~QwtBezier()
{
}

/*!
   Set the tolerance

   The tolerance is a measurement for the flatness of a curve.
   A curve with a flatness below the tolerance is considered as being flat
   terminating the subdivision algorithm.

   When interpolating a Bezier curve to render it as a sequence of lines
   to some sort of raster ( f.e to screen ) a value of 0.5 of the pixel size
   is a good value for the tolerance.

   \param tolerance Termination criterion for the subdivision
   \sa tolerance()
 */
void QwtBezier::setTolerance( double tolerance )
{
    m_tolerance = qwtMaxF( tolerance, 0.0 );
    m_flatness = BezierData::minFlatness( m_tolerance );
}

/*!
   \brief Interpolate a Bézier curve by a polygon

   \param p1 Start point
   \param cp1 First control point
   \param cp2 Second control point
   \param p2 End point

   \return Interpolating polygon
 */
QPolygonF QwtBezier::toPolygon( const QPointF& p1,
    const QPointF& cp1, const QPointF& cp2, const QPointF& p2 ) const
{
    QPolygonF polygon;

    if ( m_flatness > 0.0 )
    {
        // a flatness of 0.0 is not achievable
        appendToPolygon( p1, cp1, cp2, p2, polygon );
    }

    return polygon;
}

/*!
   \brief Interpolate a Bézier curve by a polygon

   appendToPolygon() is tailored for cumulating points from a sequence
   of bezier curves like being created by a spline interpolation.

   \param p1 Start point
   \param cp1 First control point
   \param cp2 Second control point
   \param p2 End point
   \param polygon Polygon, where the interpolating points are added

   \note If the last point of the incoming polygon matches p1 it won't be
        inserted a second time.
 */
void QwtBezier::appendToPolygon( const QPointF& p1, const QPointF& cp1,
    const QPointF& cp2, const QPointF& p2, QPolygonF& polygon ) const
{
    if ( m_flatness <= 0.0 )
    {
        // a flatness of 0.0 is not achievable
        return;
    }

    if ( polygon.isEmpty() || polygon.last() != p1 )
        polygon += p1;

    // to avoid deep stacks we convert the recursive algo
    // to something iterative, where the parameters of the
    // recursive class are pushed to a stack instead

    QStack< BezierData > stack;
    stack.push( BezierData( p1, cp1, cp2, p2 ) );

    while( true )
    {
        BezierData& bz = stack.top();

        if ( bz.flatness() < m_flatness )
        {
            if ( stack.size() == 1 )
            {
                polygon += p2;
                return;
            }

            polygon += bz.p2();
            stack.pop();
        }
        else
        {
            stack.push( bz.subdivided() );
        }
    }

}

/*!
   Find a point on a Bézier Curve

   \param p1 Start point
   \param cp1 First control point
   \param cp2 Second control point
   \param p2 End point
   \param t Parameter value, something between [0,1]

   \return Point on the curve
 */
QPointF QwtBezier::pointAt( const QPointF& p1,
    const QPointF& cp1, const QPointF& cp2, const QPointF& p2, double t )
{
    const double d1 = 3.0 * t;
    const double d2 = 3.0 * t * t;
    const double d3 = t * t * t;
    const double s = 1.0 - t;

    const double x = ( ( s * p1.x() + d1 * cp1.x() ) * s + d2 * cp2.x() ) * s + d3 * p2.x();
    const double y = ( ( s * p1.y() + d1 * cp1.y() ) * s + d2 * cp2.y() ) * s + d3 * p2.y();

    return QPointF( x, y );
}

