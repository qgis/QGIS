/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_pleasing.h"
#include "qwt_spline_parametrization.h"

#include <qpainterpath.h>

static inline double qwtChordalLength( const QPointF& point1, const QPointF& point2 )
{
    const double dx = point2.x() - point1.x();
    const double dy = point2.y() - point1.y();

    return std::sqrt( dx * dx + dy * dy );
}

template< class Param >
static QPointF qwtVector( Param param,
    const QPointF& p1, const QPointF& p2 )
{
    return ( p2 - p1 ) / param( p1, p2 );
}

template< class Param >
static QPointF qwtVectorCardinal( Param param,
    const QPointF& p1, const QPointF& p2, const QPointF& p3 )
{
    const double t1 = param( p1, p2 );
    const double t2 = param( p2, p3 );

    return t2 * ( p3 - p1 ) / ( t1 + t2 );
}

namespace QwtSplinePleasingP
{
    struct Tension
    {
        double t1;
        double t2;
    };

    struct param
    {
        param( const QwtSplineParametrization* p ):
            parameter( p )
        {
        }

        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return parameter->valueIncrement( p1, p2 );
        }

        const QwtSplineParametrization* parameter;
    };

    struct paramUniform
    {
        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return QwtSplineParametrization::valueIncrementUniform( p1, p2 );
        }
    };

    class PathStore
    {
      public:
        inline void init( int )
        {
        }

        inline void start( const QPointF& p0 )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF& cp1,
            const QPointF& cp2, const QPointF& p2 )
        {
            path.cubicTo( cp1, cp2, p2 );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
      public:
        inline ControlPointsStore():
            m_cp( NULL )
        {
        }

        inline void init( int size )
        {
            controlPoints.resize( size );
            m_cp = controlPoints.data();
        }

        inline void start( const QPointF& )
        {
        }

        inline void addCubic( const QPointF& cp1,
            const QPointF& cp2, const QPointF& )
        {
            QLineF& l = *m_cp++;
            l.setPoints( cp1, cp2 );
        }

        QVector< QLineF > controlPoints;

      private:
        QLineF* m_cp;
    };
}

static inline QwtSplinePleasingP::Tension qwtTensionPleasing(
    double d13, double d23, double d24,
    const QPointF& p1, const QPointF& p2,
    const QPointF& p3, const QPointF& p4 )
{
    QwtSplinePleasingP::Tension tension;

    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

    if ( b1 )
    {
        if ( b2 )
        {
            tension.t1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
            tension.t2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        }
        else
        {
            tension.t1 = tension.t2 = d23 / d24;
        }
    }
    else
    {
        if ( b2 )
        {
            tension.t1 = tension.t2 = d23 / d13;
        }
        else
        {
            tension.t1 = d23 / d13;
            tension.t2 = d23 / d24;
        }
    }

    return tension;
}

template< class SplineStore, class Param >
static SplineStore qwtSplinePathPleasing( const QPolygonF& points,
    bool isClosed, Param param )
{
    using namespace QwtSplinePleasingP;

    const int size = points.size();

    const QPointF* p = points.constData();

    SplineStore store;
    store.init( isClosed ? size : size - 1 );
    store.start( p[0] );

    double d13;
    QPointF vec1;

    if ( isClosed )
    {
        d13 = qwtChordalLength(p[0], p[2]);

        const Tension t0 = qwtTensionPleasing(
            qwtChordalLength( p[size - 1], p[1]), qwtChordalLength(p[0], p[1]),
            d13, p[size - 1], p[0], p[1], p[2] );

        const QPointF vec0 = qwtVectorCardinal< Param >( param, p[size - 1], p[0], p[1] );
        vec1 = qwtVectorCardinal< Param >( param, p[0], p[1], p[2] );

        store.addCubic( p[0] + vec0 * t0.t1, p[1] - vec1 * t0.t2, p[1] );
    }
    else
    {
        d13 = qwtChordalLength(p[0], p[2]);

        const Tension t0 = qwtTensionPleasing(
            qwtChordalLength( p[0], p[1]), qwtChordalLength(p[0], p[1]),
            d13,  p[0], p[0], p[1], p[2] );

        const QPointF vec0 = 0.5 * qwtVector< Param >( param, p[0], p[1] );
        vec1 = qwtVectorCardinal< Param >( param, p[0], p[1], p[2] );

        store.addCubic( p[0] + vec0 * t0.t1, p[1] - vec1 * t0.t2, p[1] );
    }

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = qwtChordalLength( p[i], p[i + 1] );
        const double d24 = qwtChordalLength( p[i], p[i + 2] );

        const QPointF vec2 = qwtVectorCardinal< Param >( param, p[i], p[i + 1], p[i + 2] );

        const Tension t = qwtTensionPleasing(
            d13, d23, d24, p[i - 1], p[i], p[i + 1], p[i + 2] );

        store.addCubic( p[i] + vec1 * t.t1, p[i + 1] - vec2 * t.t2, p[i + 1] );

        d13 = d24;
        vec1 = vec2;
    }

    if ( isClosed )
    {
        const double d24 = qwtChordalLength( p[size - 2], p[0] );

        const Tension tn = qwtTensionPleasing(
            d13, qwtChordalLength( p[size - 2], p[size - 1] ), d24,
            p[size - 3], p[size - 2], p[size - 1], p[0] );

        const QPointF vec2 = qwtVectorCardinal< Param >( param, p[size - 2], p[size - 1], p[0] );
        store.addCubic( p[size - 2] + vec1 * tn.t1, p[size - 1] - vec2 * tn.t2, p[size - 1] );

        const double d34 = qwtChordalLength( p[size - 1], p[0] );
        const double d35 = qwtChordalLength( p[size - 1], p[1] );

        const Tension tc = qwtTensionPleasing( d24, d34, d35, p[size - 2], p[size - 1], p[0], p[1] );

        const QPointF vec3 = qwtVectorCardinal< Param >( param, p[size - 1], p[0], p[1] );

        store.addCubic( p[size - 1] + vec2 * tc.t1, p[0] - vec3 * tc.t2, p[0] );
    }
    else
    {
        const double d24 = qwtChordalLength( p[size - 2], p[size - 1] );

        const Tension tn = qwtTensionPleasing(
            d13, qwtChordalLength( p[size - 2], p[size - 1] ), d24,
            p[size - 3], p[size - 2], p[size - 1], p[size - 1] );

        const QPointF vec2 = 0.5 * qwtVector< Param >( param, p[size - 2], p[size - 1] );
        store.addCubic( p[size - 2] + vec1 * tn.t1, p[size - 1] - vec2 * tn.t2, p[size - 1] );
    }

    return store;
}

/*!
   \brief Constructor

   The default setting is a non closing spline with uniform parametrization.
   ( QwtSplineParametrization::ParameterUniform ).

   \sa QwtSpline::setParametrization(), QwtSpline::setBoundaryType()
 */
QwtSplinePleasing::QwtSplinePleasing()
{
    setParametrization( QwtSplineParametrization::ParameterUniform );
}

//! Destructor
QwtSplinePleasing::~QwtSplinePleasing()
{
}

//! \return 2
uint QwtSplinePleasing::locality() const
{
    return 2;
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   \param points Control points
   \return QPainterPath Painter path, that can be rendered by QPainter
 */
QPainterPath QwtSplinePleasing::painterPath( const QPolygonF& points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return QwtSplineG1::painterPath( points );

    const bool isClosing = ( boundaryType() == QwtSpline::ClosedPolygon );

    using namespace QwtSplinePleasingP;

    PathStore store;
    if ( parametrization()->type() == QwtSplineParametrization::ParameterUniform )
    {
        store = qwtSplinePathPleasing< PathStore >( points,
            isClosing, paramUniform() );
    }
    else
    {
        store = qwtSplinePathPleasing< PathStore >( points,
            isClosing, param( parametrization() ) );
    }

    if ( isClosing )
        store.path.closeSubpath();

    return store.path;
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns the 2 control points of each curve as QLineF.

   \param points Control points
   \return Control points of the interpolating Bezier curves
 */
QVector< QLineF > QwtSplinePleasing::bezierControlLines(
    const QPolygonF& points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return QVector< QLineF >();

    const bool isClosing = ( boundaryType() == QwtSpline::ClosedPolygon );

    using namespace QwtSplinePleasingP;

    ControlPointsStore store;
    if ( parametrization()->type() == QwtSplineParametrization::ParameterUniform )
    {
        store = qwtSplinePathPleasing< ControlPointsStore >( points,
            isClosing, paramUniform() );
    }
    else
    {
        store = qwtSplinePathPleasing< ControlPointsStore >( points,
            isClosing, param( parametrization() ) );
    }

    return store.controlPoints;
}
