/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_local.h"
#include "qwt_spline_parametrization.h"
#include "qwt_spline_polynomial.h"

#include <qpainterpath.h>

static inline bool qwtIsStrictlyMonotonic( double dy1, double dy2 )
{
    if ( dy1 == 0.0 || dy2 == 0.0 )
        return false;

    return ( dy1 > 0.0 ) == ( dy2 > 0.0 );
}

static inline double qwtSlopeLine( const QPointF& p1, const QPointF& p2 )
{
    // ???
    const double dx = p2.x() - p1.x();
    return dx ? ( p2.y() - p1.y() ) / dx : 0.0;
}

static inline double qwtSlopeCardinal(
    double dx1, double dy1, double s1, double dx2, double dy2, double s2 )
{
    Q_UNUSED(s1)
    Q_UNUSED(s2)

    return ( dy1 + dy2 ) / ( dx1 + dx2 );
}

static inline double qwtSlopeParabolicBlending(
    double dx1, double dy1, double s1, double dx2, double dy2, double s2 )
{
    Q_UNUSED( dy1 )
    Q_UNUSED( dy2 )

    return ( dx2 * s1 + dx1 * s2 ) / ( dx1 + dx2 );
}

static inline double qwtSlopePChip(
    double dx1, double dy1, double s1, double dx2, double dy2, double s2 )
{
    if ( qwtIsStrictlyMonotonic( dy1, dy2 ) )
    {
#if 0
        // weighting the slopes by the dx1/dx2
        const double w1 = ( 3 * dx1 + 3 * dx2 ) / ( 2 * dx1 + 4 * dx2 );
        const double w2 = ( 3 * dx1 + 3 * dx2 ) / ( 4 * dx1 + 2 * dx2 );

        s1 *= w1;
        s2 *= w2;

        // harmonic mean ( see https://en.wikipedia.org/wiki/Pythagorean_means )
        return 2.0 / ( 1.0 / s1 + 1.0 / s2 );
#endif
        // the same as above - but faster

        const double s12 = ( dy1 + dy2 ) / ( dx1 + dx2 );
        return 3.0 * ( s1 * s2 ) / ( s1 + s2 + s12 );
    }

    return 0.0;
}

namespace QwtSplineLocalP
{
    class PathStore
    {
      public:
        inline void init( const QVector< QPointF >& )
        {
        }

        inline void start( const QPointF& p0, double )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF& p1, double m1,
            const QPointF& p2, double m2 )
        {
            const double dx3 = ( p2.x() - p1.x() ) / 3.0;

            path.cubicTo( p1.x() + dx3, p1.y() + m1 * dx3,
                p2.x() - dx3, p2.y() - m2 * dx3,
                p2.x(), p2.y() );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
      public:
        inline void init( const QVector< QPointF >& points )
        {
            if ( points.size() > 0 )
                controlPoints.resize( points.size() - 1 );
            m_cp = controlPoints.data();
        }

        inline void start( const QPointF&, double )
        {
        }

        inline void addCubic( const QPointF& p1, double m1,
            const QPointF& p2, double m2 )
        {
            const double dx3 = ( p2.x() - p1.x() ) / 3.0;

            QLineF& l = *m_cp++;
            l.setLine( p1.x() + dx3, p1.y() + m1 * dx3,
                p2.x() - dx3, p2.y() - m2 * dx3 );
        }

        QVector< QLineF > controlPoints;

      private:
        QLineF* m_cp;
    };

    class SlopeStore
    {
      public:
        void init( const QVector< QPointF >& points )
        {
            slopes.resize( points.size() );
            m_m = slopes.data();
        }

        inline void start( const QPointF&, double m0 )
        {
            *m_m++ = m0;
        }

        inline void addCubic( const QPointF&, double,
            const QPointF&, double m2 )
        {
            *m_m++ = m2;
        }

        QVector< double > slopes;

      private:
        double* m_m;
    };

    struct slopeCardinal
    {
        static inline double value( double dx1, double dy1, double s1,
            double dx2, double dy2, double s2 )
        {
            return qwtSlopeCardinal( dx1, dy1, s1, dx2, dy2, s2 );
        }
    };

    struct slopeParabolicBlending
    {
        static inline double value( double dx1, double dy1, double s1,
            double dx2, double dy2, double s2 )
        {
            return qwtSlopeParabolicBlending( dx1, dy1, s1, dx2, dy2, s2 );
        }
    };

    struct slopePChip
    {
        static inline double value( double dx1, double dy1, double s1,
            double dx2, double dy2, double s2 )
        {
            return qwtSlopePChip( dx1, dy1, s1, dx2, dy2, s2 );
        }
    };
}

template< class Slope >
static inline double qwtSlopeP3(
    const QPointF& p1, const QPointF& p2, const QPointF& p3 )
{
    const double dx1 = p2.x() - p1.x();
    const double dy1 = p2.y() - p1.y();
    const double dx2 = p3.x() - p2.x();
    const double dy2 = p3.y() - p2.y();

    return Slope::value( dx1, dy1, dy1 / dx1, dx2, dy2, dy2 / dx2 );
}

static inline double qwtSlopeAkima( double s1, double s2, double s3, double s4 )
{
    if ( ( s1 == s2 ) && ( s3 == s4 ) )
    {
        return 0.5 * ( s2 + s3 );
    }

    const double ds12 = qAbs( s2 - s1 );
    const double ds34 = qAbs( s4 - s3 );

    return ( s2 * ds34 + s3 * ds12 ) / ( ds12 + ds34 );
}

static inline double qwtSlopeAkima( const QPointF& p1, const QPointF& p2,
    const QPointF& p3, const QPointF& p4, const QPointF& p5 )
{
    const double s1 = qwtSlopeLine( p1, p2 );
    const double s2 = qwtSlopeLine( p2, p3 );
    const double s3 = qwtSlopeLine( p3, p4 );
    const double s4 = qwtSlopeLine( p4, p5 );

    return qwtSlopeAkima( s1, s2, s3, s4 );
}

template< class Slope >
static void qwtSplineBoundariesL1(
    const QwtSplineLocal* spline, const QVector< QPointF >& points,
    double& slopeBegin, double& slopeEnd )
{
    const int n = points.size();
    const QPointF* p = points.constData();

    if ( ( spline->boundaryType() == QwtSpline::PeriodicPolygon )
        || ( spline->boundaryType() == QwtSpline::ClosedPolygon ) )
    {
        const QPointF pn = p[0] - ( p[n - 1] - p[n - 2] );
        slopeBegin = slopeEnd = qwtSlopeP3< Slope >( pn, p[0], p[1] );
    }
    else
    {
        const double m2 = qwtSlopeP3< Slope >( p[0], p[1], p[2] );
        slopeBegin = spline->slopeAtBeginning( points, m2 );

        const double mn2 = qwtSlopeP3< Slope >( p[n - 3], p[n - 2], p[n - 1] );
        slopeEnd = spline->slopeAtEnd( points, mn2 );
    }
}

template< class SplineStore, class Slope >
static inline SplineStore qwtSplineL1(
    const QwtSplineLocal* spline, const QVector< QPointF >& points )
{
    const int size = points.size();
    const QPointF* p = points.constData();

    double slopeBegin, slopeEnd;
    qwtSplineBoundariesL1< Slope >( spline, points, slopeBegin, slopeEnd );

    double m1 = slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], m1 );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();
    double s1 = dy1 / dx1;

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i + 1].x() - p[i].x();
        const double dy2 = p[i + 1].y() - p[i].y();

        // cardinal spline doesn't need the line slopes, but
        // the compiler will eliminate pointless calculations
        const double s2 = dy2 / dx2;

        const double m2 = Slope::value( dx1, dy1, s1, dx2, dy2, s2 );

        store.addCubic( p[i - 1], m1, p[i], m2 );

        dx1 = dx2;
        dy1 = dy2;
        s1 = s2;
        m1 = m2;
    }

    store.addCubic( p[size - 2], m1, p[size - 1], slopeEnd );

    return store;
}

static inline void qwtSplineAkimaBoundaries(
    const QwtSplineLocal* spline, const QVector< QPointF >& points,
    double& slopeBegin, double& slopeEnd )
{
    const int n = points.size();
    const QPointF* p = points.constData();

    if ( ( spline->boundaryType() == QwtSpline::PeriodicPolygon )
        || ( spline->boundaryType() == QwtSpline::ClosedPolygon ) )
    {
        const QPointF p2 = p[0] - ( p[n - 1] - p[n - 2] );
        const QPointF p1 = p2 - ( p[n - 2] - p[n - 3] );

        slopeBegin = slopeEnd = qwtSlopeAkima( p1, p2, p[0], p[1], p[2] );

        return;
    }

    if ( spline->boundaryCondition( QwtSpline::AtBeginning ) == QwtSpline::Clamped1
        && spline->boundaryCondition( QwtSpline::AtEnd ) == QwtSpline::Clamped1 )
    {
        slopeBegin = spline->boundaryValue( QwtSpline::AtBeginning);
        slopeEnd = spline->boundaryValue( QwtSpline::AtEnd );

        return;
    }

    if ( n == 3 )
    {
        const double s1 = qwtSlopeLine( p[0], p[1] );
        const double s2 = qwtSlopeLine( p[1], p[2] );
        const double m = qwtSlopeAkima( 0.5 * s1, s1, s2, 0.5 * s2 );

        slopeBegin = spline->slopeAtBeginning( points, m );
        slopeEnd = spline->slopeAtEnd( points, m );
    }
    else
    {
        double s[3];

        s[0] = qwtSlopeLine( p[0], p[1] );
        s[1] = qwtSlopeLine( p[1], p[2] );
        s[2] = qwtSlopeLine( p[2], p[3] );

        const double m2 = qwtSlopeAkima( 0.5 * s[0], s[0], s[1], s[2] );

        slopeBegin = spline->slopeAtBeginning( points, m2 );

        s[0] = qwtSlopeLine( p[n - 4], p[n - 3] );
        s[1] = qwtSlopeLine( p[n - 3], p[n - 2] );
        s[2] = qwtSlopeLine( p[n - 2], p[n - 1] );

        const double mn2 = qwtSlopeAkima( s[0], s[1], s[2], 0.5 * s[2] );

        slopeEnd = spline->slopeAtEnd( points, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineAkima(
    const QwtSplineLocal* spline, const QVector< QPointF >& points )
{
    const int size = points.size();
    const QPointF* p = points.constData();

    double slopeBegin, slopeEnd;
    qwtSplineAkimaBoundaries( spline, points, slopeBegin, slopeEnd );

    double m1 = slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], m1 );

    double s2 = qwtSlopeLine( p[0], p[1] );
    double s3 = qwtSlopeLine( p[1], p[2] );
    double s1 = 0.5 * s2;

    for ( int i = 0; i < size - 3; i++ )
    {
        const double s4 = qwtSlopeLine( p[i + 2],  p[i + 3] );

        const double m2 = qwtSlopeAkima( s1, s2, s3, s4 );
        store.addCubic( p[i], m1, p[i + 1], m2 );

        s1 = s2;
        s2 = s3;
        s3 = s4;

        m1 = m2;
    }

    const double m2 = qwtSlopeAkima( s1, s2, s3, 0.5 * s3 );

    store.addCubic( p[size - 3], m1, p[size - 2], m2 );
    store.addCubic( p[size - 2], m2, p[size - 1], slopeEnd );

    return store;
}

template< class SplineStore >
static inline SplineStore qwtSplineLocal(
    const QwtSplineLocal* spline, const QVector< QPointF >& points )
{
    SplineStore store;

    const int size = points.size();
    if ( size <= 1 )
        return store;

    if ( size == 2 )
    {
        const double s0 = qwtSlopeLine( points[0], points[1] );
        const double m1 = spline->slopeAtBeginning( points, s0 );
        const double m2 = spline->slopeAtEnd( points, s0 );

        store.init( points );
        store.start( points[0], m1 );
        store.addCubic( points[0], m1, points[1], m2 );

        return store;
    }

    switch( spline->type() )
    {
        case QwtSplineLocal::Cardinal:
        {
            using namespace QwtSplineLocalP;
            store = qwtSplineL1< SplineStore, slopeCardinal >( spline, points );
            break;
        }
        case QwtSplineLocal::ParabolicBlending:
        {
            using namespace QwtSplineLocalP;
            store = qwtSplineL1< SplineStore, slopeParabolicBlending >( spline, points );
            break;
        }
        case QwtSplineLocal::PChip:
        {
            using namespace QwtSplineLocalP;
            store = qwtSplineL1< SplineStore, slopePChip >( spline, points );
            break;
        }
        case QwtSplineLocal::Akima:
        {
            store = qwtSplineAkima< SplineStore >( spline, points );
            break;
        }
        default:
            break;
    }

    return store;
}

/*!
   \brief Constructor

   \param type Spline type, specifying the type of interpolation
   \sa type()
 */
QwtSplineLocal::QwtSplineLocal( Type type )
    : m_type( type )
{
    setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::LinearRunout );
    setBoundaryValue( QwtSpline::AtBeginning, 0.0 );

    setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::LinearRunout );
    setBoundaryValue( QwtSpline::AtEnd, 0.0 );
}

//! Destructor
QwtSplineLocal::~QwtSplineLocal()
{
}

/*!
   \return Spline type, specifying the type of interpolation
 */
QwtSplineLocal::Type QwtSplineLocal::type() const
{
    return m_type;
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   \param points Control points
   \return Painter path, that can be rendered by QPainter
 */
QPainterPath QwtSplineLocal::painterPath( const QPolygonF& points ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        using namespace QwtSplineLocalP;
        return qwtSplineLocal< PathStore >( this, points).path;
    }

    return QwtSplineC1::painterPath( points );
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns the 2 control points of each curve as QLineF.

   \param points Control points
   \return Control points of the interpolating Bezier curves
 */
QVector< QLineF > QwtSplineLocal::bezierControlLines( const QPolygonF& points ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        using namespace QwtSplineLocalP;
        return qwtSplineLocal< ControlPointsStore >( this, points ).controlPoints;
    }

    return QwtSplineC1::bezierControlLines( points );
}

/*!
   \brief Find the first derivative at the control points

   \param points Control nodes of the spline
   \return Vector with the values of the 2nd derivate at the control points

   \note The x coordinates need to be increasing or decreasing
 */
QVector< double > QwtSplineLocal::slopes( const QPolygonF& points ) const
{
    using namespace QwtSplineLocalP;
    return qwtSplineLocal< SlopeStore >( this, points ).slopes;
}

/*!
   \brief Calculate the interpolating polynomials for a non parametric spline

   \param points Control points
   \return Interpolating polynomials

   \note The x coordinates need to be increasing or decreasing
   \note The implementation simply calls QwtSplineC1::polynomials(), but is
        intended to be replaced by a one pass calculation some day.
 */
QVector< QwtSplinePolynomial > QwtSplineLocal::polynomials( const QPolygonF& points ) const
{
    // Polynomial store -> TODO
    return QwtSplineC1::polynomials( points );
}

/*!
   The locality of an spline interpolation identifies how many adjacent
   polynomials are affected, when changing the position of one point.

   The Cardinal, ParabolicBlending and PChip algorithms have a locality of 1,
   while the Akima interpolation has a locality of 2.

   \return 1 or 2.
 */
uint QwtSplineLocal::locality() const
{
    switch ( m_type )
    {
        case Akima:
        {
            // polynomials: 2 left, 2 right
            return 2;
        }
        case Cardinal:
        case ParabolicBlending:
        case PChip:
        {
            // polynomials: 1 left, 1 right
            return 1;
        }
    }

    return QwtSplineC1::locality();
}
