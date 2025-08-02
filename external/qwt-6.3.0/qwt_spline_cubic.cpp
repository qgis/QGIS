/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_cubic.h"
#include "qwt_spline_polynomial.h"

#include <qpolygon.h>
#include <qpainterpath.h>

#define SLOPES_INCREMENTAL 0
#define KAHAN 0

namespace QwtSplineCubicP
{
    class KahanSum
    {
      public:
        inline KahanSum( double value = 0.0 ):
            m_sum( value ),
            m_carry( 0.0 )
        {
        }

        inline void reset()
        {
            m_sum = m_carry = 0.0;
        }

        inline double value() const
        {
            return m_sum;
        }

        inline void add( double value )
        {
            const double y = value - m_carry;
            const double t = m_sum + y;

            m_carry = ( t - m_sum ) - y;
            m_sum = t;
        }

        static inline double sum3( double d1, double d2, double d3 )
        {
            KahanSum sum( d1 );
            sum.add( d2 );
            sum.add( d3 );

            return sum.value();
        }

        static inline double sum4( double d1, double d2, double d3, double d4 )
        {
            KahanSum sum( d1 );
            sum.add( d2 );
            sum.add( d3 );
            sum.add( d4 );

            return sum.value();
        }


      private:
        double m_sum;
        double m_carry; // The carry from the previous operation
    };

    class CurvatureStore
    {
      public:
        inline void setup( int size )
        {
            m_curvatures.resize( size );
            m_cv = m_curvatures.data();
        }

        inline void storeFirst( double,
            const QPointF&, const QPointF&, double b1, double )
        {
            m_cv[0] = 2.0 * b1;
        }

        inline void storeNext( int index, double,
            const QPointF&, const QPointF&, double, double b2 )
        {
            m_cv[index] = 2.0 * b2;
        }

        inline void storeLast( double,
            const QPointF&, const QPointF&, double, double b2 )
        {
            m_cv[m_curvatures.size() - 1] = 2.0 * b2;
        }

        inline void storePrevious( int index, double,
            const QPointF&, const QPointF&, double b1, double )
        {
            m_cv[index] = 2.0 * b1;
        }

        inline void closeR()
        {
            m_cv[0] = m_cv[m_curvatures.size() - 1];
        }

        QVector< double > curvatures() const { return m_curvatures; }

      private:
        QVector< double > m_curvatures;
        double* m_cv;
    };

    class SlopeStore
    {
      public:
        inline void setup( int size )
        {
            m_slopes.resize( size );
            m_m = m_slopes.data();
        }

        inline void storeFirst( double h,
            const QPointF& p1, const QPointF& p2, double b1, double b2 )
        {
            const double s = ( p2.y() - p1.y() ) / h;
            m_m[0] = s - h * ( 2.0 * b1 + b2 ) / 3.0;
#if KAHAN
            m_sum.add( m_m[0] );
#endif
        }

        inline void storeNext( int index, double h,
            const QPointF& p1, const QPointF& p2, double b1, double b2 )
        {
#if SLOPES_INCREMENTAL
            Q_UNUSED( p1 )
            Q_UNUSED( p2 )
#if KAHAN
            m_sum.add( ( b1 + b2 ) * h );
            m_m[index] = m_sum.value();
#else
            m_m[index] = m_m[index - 1] + ( b1 + b2 ) * h;
#endif
#else
            const double s = ( p2.y() - p1.y() ) / h;
            m_m[index] = s + h * ( b1 + 2.0 * b2 ) / 3.0;
#endif
        }

        inline void storeLast( double h,
            const QPointF& p1, const QPointF& p2, double b1, double b2 )
        {
            const double s = ( p2.y() - p1.y() ) / h;
            m_m[m_slopes.size() - 1] = s + h * ( b1 + 2.0 * b2 ) / 3.0;
#if KAHAN
            m_sum.add( m_m[m_slopes.size() - 1] );
#endif
        }

        inline void storePrevious( int index, double h,
            const QPointF& p1, const QPointF& p2, double b1, double b2 )
        {
#if SLOPES_INCREMENTAL
            Q_UNUSED( p1 )
            Q_UNUSED( p2 )
#if KAHAN
            m_sum.add( -( b1 + b2 ) * h );
            m_m[index] = m_sum.value();
#else
            m_m[index] = m_m[index + 1] - ( b1 + b2 ) * h;
#endif

#else
            const double s = ( p2.y() - p1.y() ) / h;
            m_m[index] = s - h * ( 2.0 * b1 + b2 ) / 3.0;
#endif
        }

        inline void closeR()
        {
            m_m[0] = m_m[m_slopes.size() - 1];
        }

        QVector< double > slopes() const { return m_slopes; }

      private:
        QVector< double > m_slopes;
        double* m_m;
#if SLOPES_INCREMENTAL
        KahanSum m_sum;
#endif
    };
}

namespace QwtSplineCubicP
{
    class Equation2
    {
      public:
        inline Equation2()
        {
        }

        inline Equation2( double p0, double q0, double r0 ):
            p( p0 ),
            q( q0 ),
            r( r0 )
        {
        }

        inline void setup( double p0, double q0, double r0 )
        {
            p = p0;
            q = q0;
            r = r0;
        }

        inline Equation2 normalized() const
        {
            Equation2 c;
            c.p = 1.0;
            c.q = q / p;
            c.r = r / p;

            return c;
        }

        inline double resolved1( double x2 ) const
        {
            return ( r - q * x2 ) / p;
        }

        inline double resolved2( double x1 ) const
        {
            return ( r - p * x1 ) / q;
        }

        inline double resolved1( const Equation2& eq ) const
        {
            // find x1
            double k = q / eq.q;
            return ( r - k * eq.r ) / ( p - k * eq.p );
        }

        inline double resolved2( const Equation2& eq ) const
        {
            // find x2
            const double k = p / eq.p;
            return ( r - k * eq.r ) / ( q - k * eq.q );
        }

        // p * x1 + q * x2 = r
        double p, q, r;
    };

    class Equation3
    {
      public:
        inline Equation3()
        {
        }

        inline Equation3( const QPointF& p1, const QPointF& p2, const QPointF& p3 )
        {
            const double h1 = p2.x() - p1.x();
            const double s1 = ( p2.y() - p1.y() ) / h1;

            const double h2 = p3.x() - p2.x();
            const double s2 = ( p3.y() - p2.y() ) / h2;

            p = h1;
            q = 2 * ( h1 + h2 );
            u = h2;
            r = 3 * ( s2 - s1 );
        }

        inline Equation3( double cp, double cq, double du, double dr ):
            p( cp ),
            q( cq ),
            u( du ),
            r( dr )
        {
        }

        inline bool operator==( const Equation3& c ) const
        {
            return ( p == c.p ) && ( q == c.q ) &&
                   ( u == c.u ) && ( r == c.r );
        }

        inline void setup( double cp, double cq, double du, double dr )
        {
            p = cp;
            q = cq;
            u = du;
            r = dr;
        }

        inline Equation3 normalized() const
        {
            Equation3 c;
            c.p = 1.0;
            c.q = q / p;
            c.u = u / p;
            c.r = r / p;

            return c;
        }

        inline Equation2 substituted1( const Equation3& eq ) const
        {
            // eliminate x1
            const double k = p / eq.p;
            return Equation2( q - k * eq.q, u - k * eq.u, r - k * eq.r );
        }

        inline Equation2 substituted2( const Equation3& eq ) const
        {
            // eliminate x2

            const double k = q / eq.q;
            return Equation2( p - k * eq.p, u - k * eq.u, r - k * eq.r );
        }

        inline Equation2 substituted3( const Equation3& eq ) const
        {
            // eliminate x3

            const double k = u / eq.u;
            return Equation2( p - k * eq.p, q - k * eq.q, r - k * eq.r );
        }

        inline Equation2 substituted1( const Equation2& eq ) const
        {
            // eliminate x1
            const double k = p / eq.p;
            return Equation2( q - k * eq.q, u, r - k * eq.r );
        }

        inline Equation2 substituted3( const Equation2& eq ) const
        {
            // eliminate x3

            const double k = u / eq.q;
            return Equation2( p, q - k * eq.p, r - k * eq.r );
        }


        inline double resolved1( double x2, double x3 ) const
        {
            return ( r - q * x2 - u * x3 ) / p;
        }

        inline double resolved2( double x1, double x3 ) const
        {
            return ( r - u * x3 - p * x1 ) / q;
        }

        inline double resolved3( double x1, double x2 ) const
        {
            return ( r - p * x1 - q * x2 ) / u;
        }

        // p * x1 + q * x2 + u * x3 = r
        double p, q, u, r;
    };
}

#if 0
static QDebug operator<<( QDebug debug, const QwtSplineCubicP::Equation2& eq )
{
    debug.nospace() << "EQ2(" << eq.p << ", " << eq.q << ", " << eq.r << ")";
    return debug.space();
}

static QDebug operator<<( QDebug debug, const QwtSplineCubicP::Equation3& eq )
{
    debug.nospace() << "EQ3(" << eq.p << ", "
                    << eq.q << ", " << eq.u << ", " << eq.r << ")";
    return debug.space();
}
#endif

namespace QwtSplineCubicP
{
    template< class T >
    class EquationSystem
    {
      public:
        void setStartCondition( double p, double q, double u, double r )
        {
            m_conditionsEQ[0].setup( p, q, u, r );
        }

        void setEndCondition( double p, double q, double u, double r )
        {
            m_conditionsEQ[1].setup( p, q, u, r );
        }

        const T& store() const
        {
            return m_store;
        }

        void resolve( const QPolygonF& p )
        {
            const int n = p.size();
            if ( n < 3 )
                return;

            if ( m_conditionsEQ[0].p == 0.0 ||
                ( m_conditionsEQ[0].q == 0.0 && m_conditionsEQ[0].u != 0.0 ) )
            {
                return;
            }

            if ( m_conditionsEQ[1].u == 0.0 ||
                ( m_conditionsEQ[1].q == 0.0 && m_conditionsEQ[1].p != 0.0 ) )
            {
                return;
            }

            const double h0 = p[1].x() - p[0].x();
            const double h1 = p[2].x() - p[1].x();
            const double hn = p[n - 1].x() - p[n - 2].x();

            m_store.setup( n );


            if ( n == 3 )
            {
                // For certain conditions the first/last point does not
                // necessarily meet the spline equation and we would
                // have many solutions. In this case we resolve using
                // the spline equation - as for all other conditions.

                const Equation3 eqSpline0( p[0], p[1], p[2] ); // ???
                const Equation2 eq0 = m_conditionsEQ[0].substituted1( eqSpline0 );

                // The equation system can be solved without substitution
                // from the start/end conditions and eqSpline0 ( = eqSplineN ).

                double b1;

                if ( m_conditionsEQ[0].normalized() == m_conditionsEQ[1].normalized() )
                {
                    // When we have 3 points only and start/end conditions
                    // for 3 points mean the same condition the system
                    // is under-determined and has many solutions.
                    // We chose b1 = 0.0

                    b1 = 0.0;
                }
                else
                {
                    const Equation2 eq = m_conditionsEQ[1].substituted1( eqSpline0 );
                    b1 = eq0.resolved1( eq );
                }

                const double b2 = eq0.resolved2( b1 );
                const double b0 = eqSpline0.resolved1( b1, b2 );

                m_store.storeFirst( h0, p[0], p[1], b0, b1 );
                m_store.storeNext( 1, h0, p[0], p[1], b0, b1 );
                m_store.storeNext( 2, h1, p[1], p[2], b1, b2 );

                return;
            }

            const Equation3 eqSplineN( p[n - 3], p[n - 2], p[n - 1] );
            const Equation2 eqN = m_conditionsEQ[1].substituted3( eqSplineN );

            Equation2 eq = eqN;
            if ( n > 4 )
            {
                const Equation3 eqSplineR( p[n - 4], p[n - 3], p[n - 2] );
                eq = eqSplineR.substituted3( eq );
                eq = substituteSpline( p, eq );
            }

            const Equation3 eqSpline0( p[0], p[1], p[2] );

            double b0, b1;
            if ( m_conditionsEQ[0].u == 0.0 )
            {
                eq = eqSpline0.substituted3( eq );

                const Equation3& eq0 = m_conditionsEQ[0];
                b0 = Equation2( eq0.p, eq0.q, eq0.r ).resolved1( eq );
                b1 = eq.resolved2( b0 );
            }
            else
            {
                const Equation2 eqX = m_conditionsEQ[0].substituted3( eq );
                const Equation2 eqY = eqSpline0.substituted3( eq );

                b0 = eqY.resolved1( eqX );
                b1 = eqY.resolved2( b0 );
            }

            m_store.storeFirst( h0, p[0], p[1], b0, b1 );
            m_store.storeNext( 1, h0, p[0], p[1], b0, b1 );

            const double bn2 = resolveSpline( p, b1 );

            const double bn1 = eqN.resolved2( bn2 );
            const double bn0 = m_conditionsEQ[1].resolved3( bn2, bn1 );

            const double hx = p[n - 2].x() - p[n - 3].x();
            m_store.storeNext( n - 2, hx, p[n - 3], p[n - 2], bn2, bn1 );
            m_store.storeNext( n - 1, hn, p[n - 2], p[n - 1], bn1, bn0 );
        }

      private:
        Equation2 substituteSpline( const QPolygonF& points, const Equation2& eq )
        {
            const int n = points.size();

            m_eq.resize( n - 2 );
            m_eq[n - 3] = eq;

            // eq[i].resolved2( b[i-1] ) => b[i]

            double slope2 = ( points[n - 3].y() - points[n - 4].y() ) / eq.p;

            for ( int i = n - 4; i > 1; i-- )
            {
                const Equation2& eq2 = m_eq[i + 1];
                Equation2& eq1 = m_eq[i];

                eq1.p = points[i].x() - points[i - 1].x();
                const double slope1 = ( points[i].y() - points[i - 1].y() ) / eq1.p;

                const double v = eq2.p / eq2.q;

                eq1.q = 2.0 * ( eq1.p + eq2.p ) - v * eq2.p;
                eq1.r = 3.0 * ( slope2 - slope1 ) - v * eq2.r;

                slope2 = slope1;
            }

            return m_eq[2];
        }

        double resolveSpline( const QPolygonF& points, double b1 )
        {
            const int n = points.size();
            const QPointF* p = points.constData();

            for ( int i = 2; i < n - 2; i++ )
            {
                // eq[i].resolved2( b[i-1] ) => b[i]
                const double b2 = m_eq[i].resolved2( b1 );
                m_store.storeNext( i, m_eq[i].p, p[i - 1], p[i], b1, b2 );

                b1 = b2;
            }

            return b1;
        }

      private:
        Equation3 m_conditionsEQ[2];
        QVector< Equation2 > m_eq;
        T m_store;
    };

    template< class T >
    class EquationSystem2
    {
      public:
        const T& store() const
        {
            return m_store;
        }

        void resolve( const QPolygonF& p )
        {
            const int n = p.size();

            if ( p[n - 1].y() != p[0].y() )
            {
                // TODO ???
            }

            const double h0 = p[1].x() - p[0].x();
            const double s0 = ( p[1].y() - p[0].y() ) / h0;

            if ( n == 3 )
            {
                const double h1 = p[2].x() - p[1].x();
                const double s1 = ( p[2].y() - p[1].y() ) / h1;

                const double b = 3.0 * ( s0 - s1 ) / ( h0 + h1 );

                m_store.setup( 3 );
                m_store.storeLast( h1, p[1], p[2], -b, b );
                m_store.storePrevious( 1, h1, p[1], p[2], -b, b );
                m_store.closeR();

                return;
            }

            const double hn = p[n - 1].x() - p[n - 2].x();

            Equation2 eqn, eqX;
            substitute( p, eqn, eqX );

            const double b0 = eqn.resolved2( eqX );
            const double bn = eqn.resolved1( b0 );

            m_store.setup( n );
            m_store.storeLast( hn, p[n - 2], p[n - 1], bn, b0 );
            m_store.storePrevious( n - 2, hn, p[n - 2], p[n - 1], bn, b0 );

            resolveSpline( p, b0, bn );

            m_store.closeR();
        }

      private:

        void substitute( const QPolygonF& points, Equation2& eqn, Equation2& eqX )
        {
            const int n = points.size();

            const double hn = points[n - 1].x() - points[n - 2].x();

            const Equation3 eqSpline0( points[0], points[1], points[2] );
            const Equation3 eqSplineN(
                QPointF( points[0].x() - hn, points[n - 2].y() ), points[0], points[1] );

            m_eq.resize( n - 1 );

            double dq = 0;
            double dr = 0;

            m_eq[1] = eqSpline0;

            double slope1 = ( points[2].y() - points[1].y() ) / m_eq[1].u;

            // a) p1 * b[0] + q1 * b[1] + u1 * b[2] = r1
            // b) p2 * b[n-2] + q2 * b[0] + u2 * b[1] = r2
            // c) pi * b[i-1] + qi * b[i] + ui * b[i+1] = ri
            //
            // Using c) we can substitute b[i] ( starting from 2 ) by b[i+1]
            // until we reach n-1. As we know, that b[0] == b[n-1] we found
            // an equation where only 2 coefficients ( for b[n-2], b[0] ) are left unknown.
            // Each step we have an equation that depends on b[0], b[i] and b[i+1]
            // that can also be used to substitute b[i] in b). Ding so we end up with another
            // equation depending on b[n-2], b[0] only.
            // Finally 2 equations with 2 coefficients can be solved.

            for ( int i = 2; i < n - 1; i++ )
            {
                const Equation3& eq1 = m_eq[i - 1];
                Equation3& eq2 = m_eq[i];

                dq += eq1.p * eq1.p / eq1.q;
                dr += eq1.p * eq1.r / eq1.q;

                eq2.u = points[i + 1].x() - points[i].x();
                const double slope2 = ( points[i + 1].y() - points[i].y() ) / eq2.u;

                const double k = eq1.u / eq1.q;

                eq2.p = -eq1.p * k;
                eq2.q = 2.0 * ( eq1.u + eq2.u ) - eq1.u * k;
                eq2.r = 3.0 * ( slope2 - slope1 ) - eq1.r * k;

                slope1 = slope2;
            }


            // b[0] * m_p[n-2] + b[n-2] * m_q[n-2] + b[n-1] * pN = m_r[n-2]
            eqn.setup( m_eq[n - 2].q, m_eq[n - 2].p + eqSplineN.p, m_eq[n - 2].r );

            // b[n-2] * pN + b[0] * ( qN - dq ) + b[n-2] * m_p[n-2] = rN - dr
            eqX.setup( m_eq[n - 2].p + eqSplineN.p, eqSplineN.q - dq, eqSplineN.r - dr );
        }

        void resolveSpline( const QPolygonF& points, double b0, double bi )
        {
            const int n = points.size();

            for ( int i = n - 3; i >= 1; i-- )
            {
                const Equation3& eq = m_eq[i];

                const double b = eq.resolved2( b0, bi );
                m_store.storePrevious( i, eq.u, points[i], points[i + 1], b, bi );

                bi = b;
            }
        }

        void resolveSpline2( const QPolygonF& points,
            double b0, double bi, QVector< double >& m )
        {
            const int n = points.size();

            bi = m_eq[0].resolved3( b0, bi );

            for ( int i = 1; i < n - 2; i++ )
            {
                const Equation3& eq = m_eq[i];

                const double b = eq.resolved3( b0, bi );
                m[i + 1] = m[i] + ( b + bi ) * m_eq[i].u;

                bi = b;
            }
        }

        void resolveSpline3( const QPolygonF& points,
            double b0, double b1, QVector< double >& m )
        {
            const int n = points.size();

            double h0 = ( points[1].x() - points[0].x() );
            double s0 = ( points[1].y() - points[0].y() ) / h0;

            m[1] = m[0] + ( b0 + b1 ) * h0;

            for ( int i = 1; i < n - 1; i++ )
            {
                const double h1 = ( points[i + 1].x() - points[i].x() );
                const double s1 = ( points[i + 1].y() - points[i].y() ) / h1;

                const double r = 3.0 * ( s1 - s0 );

                const double b2 = ( r - h0 * b0 - 2.0 * ( h0 + h1 ) * b1 ) / h1;
                m[i + 1] = m[i] + ( b1 + b2 ) * h1;

                h0 = h1;
                s0 = s1;
                b0 = b1;
                b1 = b2;
            }
        }

        void resolveSpline4( const QPolygonF& points,
            double b2, double b1, QVector< double >& m )
        {
            const int n = points.size();

            double h2 = ( points[n - 1].x() - points[n - 2].x() );
            double s2 = ( points[n - 1].y() - points[n - 2].y() ) / h2;

            for ( int i = n - 2; i > 1; i-- )
            {
                const double h1 = ( points[i].x() - points[i - 1].x() );
                const double s1 = ( points[i].y() - points[i - 1].y() ) / h1;

                const double r = 3.0 * ( s2 - s1 );
                const double k = 2.0 * ( h1 + h2 );

                const double b0 = ( r - h2 * b2 - k * b1 ) / h1;

                m[i - 1] = m[i] - ( b0 + b1 ) * h1;

                h2 = h1;
                s2 = s1;
                b2 = b1;
                b1 = b0;
            }
        }

      public:
        QVector< Equation3 > m_eq;
        T m_store;
    };
}

static void qwtSetupEndEquations(
    int conditionBegin, double valueBegin, int conditionEnd, double valueEnd,
    const QPolygonF& points, QwtSplineCubicP::Equation3 eq[2] )
{
    const int n = points.size();

    const double h0 = points[1].x() - points[0].x();
    const double s0 = ( points[1].y() - points[0].y() ) / h0;

    const double hn = ( points[n - 1].x() - points[n - 2].x() );
    const double sn = ( points[n - 1].y() - points[n - 2].y() ) / hn;

    switch( conditionBegin )
    {
        case QwtSpline::Clamped1:
        {
            // first derivative at end points given

            // 3 * a1 * h + b1 = b2
            // a1 * h * h + b1 * h + c1 = s

            // c1 = slopeBegin
            // => b1 * ( 2 * h / 3.0 ) + b2 * ( h / 3.0 ) = s - slopeBegin

            // c2 = slopeEnd
            // => b1 * ( 1.0 / 3.0 ) + b2 * ( 2.0 / 3.0 ) = ( slopeEnd - s ) / h;

            eq[0].setup( 2 * h0 / 3.0, h0 / 3.0, 0.0, s0 - valueBegin );
            break;
        }
        case QwtSpline::Clamped2:
        {
            // second derivative at end points given

            // b0 = 0.5 * cvStart
            // => b0 * 1.0 + b1 * 0.0 = 0.5 * cvStart

            // b1 = 0.5 * cvEnd
            // => b0 * 0.0 + b1 * 1.0 = 0.5 * cvEnd

            eq[0].setup( 1.0, 0.0, 0.0, 0.5 * valueBegin );
            break;
        }
        case QwtSpline::Clamped3:
        {
            // third derivative at end point given

            // 3 * a * h0 + b[0] = b[1]

            // a = marg_0 / 6.0
            // => b[0] * 1.0 + b[1] * ( -1.0 ) = -0.5 * v0 * h0

            // a = marg_n / 6.0
            // => b[n-2] * 1.0 + b[n-1] * ( -1.0 ) = -0.5 * v1 * h5

            eq[0].setup( 1.0, -1.0, 0.0, -0.5 * valueBegin * h0 );

            break;
        }
        case QwtSpline::LinearRunout:
        {
            const double r0 = qBound( 0.0, valueBegin, 1.0 );
            if ( r0 == 0.0 )
            {
                // clamping s0
                eq[0].setup( 2 * h0 / 3.0, h0 / 3.0, 0.0, 0.0 );
            }
            else
            {
                eq[0].setup( 1.0 + 2.0 / r0, 2.0 + 1.0 / r0, 0.0, 0.0 );
            }
            break;
        }
        case QwtSplineC2::NotAKnot:
        case QwtSplineC2::CubicRunout:
        {
            // building one cubic curve from 3 points

            double v0;

            if ( conditionBegin == QwtSplineC2::CubicRunout )
            {
                // first/last point are the endpoints of the curve

                // b0 = 2 * b1 - b2
                // => 1.0 * b0 - 2 * b1 + 1.0 * b2 = 0.0

                v0 = 1.0;
            }
            else
            {
                // first/last points are on the curve,
                // the imaginary endpoints have the same distance as h0/hn

                v0 = h0 / ( points[2].x() - points[1].x() );
            }

            eq[0].setup( 1.0, -( 1.0 + v0 ), v0, 0.0 );
            break;
        }
        default:
        {
            // a natural spline, where the
            // second derivative at end points set to 0.0
            eq[0].setup( 1.0, 0.0, 0.0, 0.0 );
            break;
        }
    }

    switch( conditionEnd )
    {
        case QwtSpline::Clamped1:
        {
            // first derivative at end points given
            eq[1].setup( 0.0, 1.0 / 3.0 * hn, 2.0 / 3.0 * hn, valueEnd - sn );
            break;
        }
        case QwtSpline::Clamped2:
        {
            // second derivative at end points given
            eq[1].setup( 0.0, 0.0, 1.0, 0.5 * valueEnd );
            break;
        }
        case QwtSpline::Clamped3:
        {
            // third derivative at end point given
            eq[1].setup( 0.0, 1.0, -1.0, -0.5 * valueEnd * hn );
            break;
        }
        case QwtSpline::LinearRunout:
        {
            const double rn = qBound( 0.0, valueEnd, 1.0 );
            if ( rn == 0.0 )
            {
                // clamping sn
                eq[1].setup( 0.0, 1.0 / 3.0 * hn, 2.0 / 3.0 * hn, 0.0 );
            }
            else
            {
                eq[1].setup( 0.0, 2.0 + 1.0 / rn, 1.0 + 2.0 / rn, 0.0 );
            }

            break;
        }
        case QwtSplineC2::NotAKnot:
        case QwtSplineC2::CubicRunout:
        {
            // building one cubic curve from 3 points

            double vn;

            if ( conditionEnd == QwtSplineC2::CubicRunout )
            {
                // last point is the endpoints of the curve
                vn = 1.0;
            }
            else
            {
                // last points on the curve,
                // the imaginary endpoints have the same distance as hn

                vn = hn / ( points[n - 2].x() - points[n - 3].x() );
            }

            eq[1].setup( vn, -( 1.0 + vn ), 1.0, 0.0 );
            break;
        }
        default:
        {
            // a natural spline, where the
            // second derivative at end points set to 0.0
            eq[1].setup( 0.0, 0.0, 1.0, 0.0 );
            break;
        }
    }
}

class QwtSplineCubic::PrivateData
{
};

/*!
   \brief Constructor
   The default setting is a non closing natural spline with no parametrization.
 */
QwtSplineCubic::QwtSplineCubic()
    : m_data( NULL )
{
    // a natural spline

    setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped2 );
    setBoundaryValue( QwtSpline::AtBeginning, 0.0 );

    setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped2 );
    setBoundaryValue( QwtSpline::AtEnd, 0.0 );
}

//! Destructor
QwtSplineCubic::~QwtSplineCubic()
{
}

/*!
   A cubic spline is non local, where changing one point has em effect on all
   polynomials.

   \return 0
 */
uint QwtSplineCubic::locality() const
{
    return 0;
}

/*!
   \brief Find the first derivative at the control points

   In opposite to the implementation QwtSplineC2::slopes the first derivates
   are calculated directly, without calculating the second derivates first.

   \param points Control nodes of the spline
   \return Vector with the values of the 2nd derivate at the control points

   \sa curvatures(), QwtSplinePolynomial::fromCurvatures()
   \note The x coordinates need to be increasing or decreasing
 */
QVector< double > QwtSplineCubic::slopes( const QPolygonF& points ) const
{
    using namespace QwtSplineCubicP;

    if ( points.size() <= 2 )
        return QVector< double >();

    if ( ( boundaryType() == QwtSpline::PeriodicPolygon )
        || ( boundaryType() == QwtSpline::ClosedPolygon ) )
    {
        EquationSystem2< SlopeStore > eqs;
        eqs.resolve( points );

        return eqs.store().slopes();
    }

    if ( points.size() == 3 )
    {
        if ( boundaryCondition( QwtSpline::AtBeginning ) == QwtSplineCubic::NotAKnot
            || boundaryCondition( QwtSpline::AtEnd ) == QwtSplineCubic::NotAKnot )
        {
#if 0
            const double h0 = points[1].x() - points[0].x();
            const double h1 = points[2].x() - points[1].x();

            const double s0 = ( points[1].y() - points[0].y() ) / h0;
            const double s1 = ( points[2].y() - points[1].y() ) / h1;

            /*
               the system is under-determined and we only
               compute a quadratic spline.
             */

            const double b = ( s1 - s0 ) / ( h0 + h1 );

            QVector< double > m( 3 );
            m[0] = s0 - h0 * b;
            m[1] = s1 - h1 * b;
            m[2] = s1 + h1 * b;

            return m;
#else
            return QVector< double >();
#endif
        }
    }

    Equation3 eq[2];
    qwtSetupEndEquations(
        boundaryCondition( QwtSpline::AtBeginning ),
        boundaryValue( QwtSpline::AtBeginning ),
        boundaryCondition( QwtSpline::AtEnd ),
        boundaryValue( QwtSpline::AtEnd ),
        points, eq );

    EquationSystem< SlopeStore > eqs;
    eqs.setStartCondition( eq[0].p, eq[0].q, eq[0].u, eq[0].r );
    eqs.setEndCondition( eq[1].p, eq[1].q, eq[1].u, eq[1].r );
    eqs.resolve( points );

    return eqs.store().slopes();
}

/*!
   \brief Find the second derivative at the control points

   \param points Control nodes of the spline
   \return Vector with the values of the 2nd derivate at the control points

   \sa slopes()
   \note The x coordinates need to be increasing or decreasing
 */
QVector< double > QwtSplineCubic::curvatures( const QPolygonF& points ) const
{
    using namespace QwtSplineCubicP;

    if ( points.size() <= 2 )
        return QVector< double >();

    if ( ( boundaryType() == QwtSpline::PeriodicPolygon )
        || ( boundaryType() == QwtSpline::ClosedPolygon ) )
    {
        EquationSystem2< CurvatureStore > eqs;
        eqs.resolve( points );

        return eqs.store().curvatures();
    }

    if ( points.size() == 3 )
    {
        if ( boundaryCondition( QwtSpline::AtBeginning ) == QwtSplineC2::NotAKnot
            || boundaryCondition( QwtSpline::AtEnd ) == QwtSplineC2::NotAKnot )
        {
            return QVector< double >();
        }
    }

    Equation3 eq[2];
    qwtSetupEndEquations(
        boundaryCondition( QwtSpline::AtBeginning ),
        boundaryValue( QwtSpline::AtBeginning ),
        boundaryCondition( QwtSpline::AtEnd ),
        boundaryValue( QwtSpline::AtEnd ),
        points, eq );

    EquationSystem< CurvatureStore > eqs;
    eqs.setStartCondition( eq[0].p, eq[0].q, eq[0].u, eq[0].r );
    eqs.setEndCondition( eq[1].p, eq[1].q, eq[1].u, eq[1].r );
    eqs.resolve( points );

    return eqs.store().curvatures();
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   \param points Control points
   \return Painter path, that can be rendered by QPainter

   \note The implementation simply calls QwtSplineC1::painterPath()
 */
QPainterPath QwtSplineCubic::painterPath( const QPolygonF& points ) const
{
    // as QwtSplineCubic can calculate slopes directly we can
    // use the implementation of QwtSplineC1 without any performance loss.

    return QwtSplineC1::painterPath( points );
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns the 2 control points of each curve as QLineF.

   \param points Control points
   \return Control points of the interpolating Bezier curves

   \note The implementation simply calls QwtSplineC1::bezierControlLines()
 */
QVector< QLineF > QwtSplineCubic::bezierControlLines( const QPolygonF& points ) const
{
    // as QwtSplineCubic can calculate slopes directly we can
    // use the implementation of QwtSplineC1 without any performance loss.

    return QwtSplineC1::bezierControlLines( points );
}

/*!
   \brief Calculate the interpolating polynomials for a non parametric spline

   \param points Control points
   \return Interpolating polynomials

   \note The x coordinates need to be increasing or decreasing
   \note The implementation simply calls QwtSplineC2::polynomials(), but is
        intended to be replaced by a one pass calculation some day.
 */
QVector< QwtSplinePolynomial > QwtSplineCubic::polynomials( const QPolygonF& points ) const
{
    return QwtSplineC2::polynomials( points );
}

