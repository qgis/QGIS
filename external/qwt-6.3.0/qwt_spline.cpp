/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline.h"
#include "qwt_spline_parametrization.h"
#include "qwt_spline_polynomial.h"
#include "qwt_bezier.h"

#include <qpainterpath.h>

namespace QwtSplineC1P
{
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

    struct paramY
    {
        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return QwtSplineParametrization::valueIncrementY( p1, p2 );
        }
    };

    struct paramUniform
    {
        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return QwtSplineParametrization::valueIncrementUniform( p1, p2 );
        }
    };

    struct paramCentripetal
    {
        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return QwtSplineParametrization::valueIncrementCentripetal( p1, p2 );
        }
    };

    struct paramChordal
    {
        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return QwtSplineParametrization::valueIncrementChordal( p1, p2 );
        }
    };

    struct paramManhattan
    {
        inline double operator()( const QPointF& p1, const QPointF& p2 ) const
        {
            return QwtSplineParametrization::valueIncrementManhattan( p1, p2 );
        }
    };

    class PathStore
    {
      public:
        inline void init( int size )
        {
            Q_UNUSED(size);
        }

        inline void start( double x1, double y1 )
        {
            path.moveTo( x1, y1 );
        }

        inline void addCubic( double cx1, double cy1,
            double cx2, double cy2, double x2, double y2 )
        {
            path.cubicTo( cx1, cy1, cx2, cy2, x2, y2 );
        }

        inline void end()
        {
            path.closeSubpath();
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

        inline void start( double x1, double y1 )
        {
            Q_UNUSED( x1 );
            Q_UNUSED( y1 );
        }

        inline void addCubic( double cx1, double cy1,
            double cx2, double cy2, double x2, double y2 )
        {
            Q_UNUSED( x2 );
            Q_UNUSED( y2 );

            QLineF& l = *m_cp++;
            l.setLine( cx1, cy1, cx2, cy2 );
        }

        inline void end()
        {
        }

        QVector< QLineF > controlPoints;

      private:
        QLineF* m_cp;
    };

    double slopeBoundary( int boundaryCondition, double boundaryValue,
        const QPointF& p1, const QPointF& p2, double slope1 )
    {
        const double dx = p2.x() - p1.x();
        const double dy = p2.y() - p1.y();

        double m = 0.0;

        switch( boundaryCondition )
        {
            case QwtSpline::Clamped1:
            {
                m = boundaryValue;
                break;
            }
            case QwtSpline::Clamped2:
            {
                const double c2 = 0.5 * boundaryValue;
                const double c1 = slope1;

                m = 0.5 * ( 3.0 * dy / dx - c1 - c2 * dx );
                break;
            }
            case QwtSpline::Clamped3:
            {
                const double c3 = boundaryValue / 6.0;
                m = c3 * dx * dx + 2 * dy / dx - slope1;
                break;
            }
            case QwtSpline::LinearRunout:
            {
                const double s = dy / dx;
                const double r = qBound( 0.0, boundaryValue, 1.0 );

                m = s - r * ( s - slope1 );
                break;
            }
            default:
            {
                m = dy / dx; // something
            }
        }

        return m;
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineC1PathParamX(
    const QwtSplineC1* spline, const QPolygonF& points )
{
    const int n = points.size();

    const QVector< double > m = spline->slopes( points );
    if ( m.size() != n )
        return SplineStore();

    const QPointF* pd = points.constData();
    const double* md = m.constData();

    SplineStore store;
    store.init( m.size() - 1 );
    store.start( pd[0].x(), pd[0].y() );

    for ( int i = 0; i < n - 1; i++ )
    {
        const double dx3 = ( pd[i + 1].x() - pd[i].x() ) / 3.0;

        store.addCubic( pd[i].x() + dx3, pd[i].y() + md[i] * dx3,
            pd[i + 1].x() - dx3, pd[i + 1].y() - md[i + 1] * dx3,
            pd[i + 1].x(), pd[i + 1].y() );
    }

    return store;
}

template< class SplineStore >
static inline SplineStore qwtSplineC1PathParamY(
    const QwtSplineC1* spline, const QPolygonF& points )
{
    const int n = points.size();

    QPolygonF pointsFlipped( n );
    for ( int i = 0; i < n; i++ )
    {
        pointsFlipped[i].setX( points[i].y() );
        pointsFlipped[i].setY( points[i].x() );
    }

    const QVector< double > m = spline->slopes( pointsFlipped );
    if ( m.size() != n )
        return SplineStore();

    const QPointF* pd = pointsFlipped.constData();
    const double* md = m.constData();

    SplineStore store;
    store.init( m.size() - 1 );
    store.start( pd[0].y(), pd[0].x() );

    QVector< QLineF > lines( n );
    for ( int i = 0; i < n - 1; i++ )
    {
        const double dx3 = ( pd[i + 1].x() - pd[i].x() ) / 3.0;

        store.addCubic( pd[i].y() + md[i] * dx3, pd[i].x() + dx3,
            pd[i + 1].y() - md[i + 1] * dx3, pd[i + 1].x() - dx3,
            pd[i + 1].y(), pd[i + 1].x() );
    }

    return store;
}

template< class SplineStore, class Param >
static inline SplineStore qwtSplineC1PathParametric(
    const QwtSplineC1* spline, const QPolygonF& points, Param param )
{
    const bool isClosing = ( spline->boundaryType() == QwtSpline::ClosedPolygon );
    const int n = points.size();

    QPolygonF pointsX, pointsY;
    pointsX.resize( isClosing ? n + 1 : n );
    pointsY.resize( isClosing ? n + 1 : n );

    QPointF* px = pointsX.data();
    QPointF* py = pointsY.data();
    const QPointF* p = points.constData();

    double t = 0.0;

    px[0].rx() = py[0].rx() = t;
    px[0].ry() = p[0].x();
    py[0].ry() = p[0].y();

    int numParamPoints = 1;
    for ( int i = 1; i < n; i++ )
    {
        const double td = param( points[i - 1], points[i] );
        if ( td > 0.0 )
        {
            t += td;

            px[numParamPoints].rx() = py[numParamPoints].rx() = t;

            px[numParamPoints].ry() = p[i].x();
            py[numParamPoints].ry() = p[i].y();

            numParamPoints++;
        }
    }

    if ( isClosing )
    {
        const double td = param( points[n - 1], points[0] );

        if ( td > 0.0 )
        {
            t += td;

            px[numParamPoints].rx() = py[numParamPoints].rx() = t;

            px[numParamPoints].ry() = p[0].x();
            py[numParamPoints].ry() = p[0].y();

            numParamPoints++;
        }
    }

    if ( pointsX.size() != numParamPoints )
    {
        pointsX.resize( numParamPoints );
        pointsY.resize( numParamPoints );
    }

    const QVector< double > slopesX = spline->slopes( pointsX );
    const QVector< double > slopesY = spline->slopes( pointsY );

    const double* mx = slopesX.constData();
    const double* my = slopesY.constData();

    // we don't need it anymore
    pointsX.clear();
    pointsY.clear();

    SplineStore store;
    store.init( isClosing ? n : n - 1 );
    store.start( points[0].x(), points[0].y() );

    int j = 0;

    for ( int i = 0; i < n - 1; i++ )
    {
        const QPointF& p1 = p[i];
        const QPointF& p2 = p[i + 1];

        const double td = param( p1, p2 );

        if ( td != 0.0 )
        {
            const double t3 = td / 3.0;

            const double cx1 = p1.x() + mx[j] * t3;
            const double cy1 = p1.y() + my[j] * t3;

            const double cx2 = p2.x() - mx[j + 1] * t3;
            const double cy2 = p2.y() - my[j + 1] * t3;

            store.addCubic( cx1, cy1, cx2, cy2, p2.x(), p2.y() );

            j++;
        }
        else
        {
            // setting control points to the ends
            store.addCubic( p1.x(), p1.y(), p2.x(), p2.y(), p2.x(), p2.y() );
        }
    }

    if ( isClosing )
    {
        const QPointF& p1 = p[n - 1];
        const QPointF& p2 = p[0];

        const double td = param( p1, p2 );

        if ( td != 0.0 )
        {
            const double t3 = td / 3.0;

            const double cx1 = p1.x() + mx[j] * t3;
            const double cy1 = p1.y() + my[j] * t3;

            const double cx2 = p2.x() - mx[0] * t3;
            const double cy2 = p2.y() - my[0] * t3;

            store.addCubic( cx1, cy1, cx2, cy2, p2.x(), p2.y() );
        }
        else
        {
            store.addCubic( p1.x(), p1.y(), p2.x(), p2.y(), p2.x(), p2.y() );
        }

        store.end();
    }

    return store;
}

template< QwtSplinePolynomial toPolynomial( const QPointF&, double, const QPointF&, double ) >
static QPolygonF qwtPolygonParametric( double distance,
    const QPolygonF& points, const QVector< double >& values, bool withNodes )
{
    QPolygonF fittedPoints;

    const QPointF* p = points.constData();
    const double* v = values.constData();

    fittedPoints += p[0];
    double t = distance;

    const int n = points.size();

    for ( int i = 0; i < n - 1; i++ )
    {
        const QPointF& p1 = p[i];
        const QPointF& p2 = p[i + 1];

        const QwtSplinePolynomial polynomial = toPolynomial( p1, v[i], p2, v[i + 1] );

        const double l = p2.x() - p1.x();

        while ( t < l )
        {
            fittedPoints += QPointF( p1.x() + t, p1.y() + polynomial.valueAt( t ) );
            t += distance;
        }

        if ( withNodes )
        {
            if ( qFuzzyCompare( fittedPoints.last().x(), p2.x() ) )
                fittedPoints.last() = p2;
            else
                fittedPoints += p2;
        }
        else
        {
            t -= l;
        }
    }

    return fittedPoints;
}

class QwtSpline::PrivateData
{
  public:
    PrivateData()
        : boundaryType( QwtSpline::ConditionalBoundaries )
    {
        parametrization = new QwtSplineParametrization(
            QwtSplineParametrization::ParameterChordal );

        // parabolic runout at both ends

        boundaryConditions[0].type = QwtSpline::Clamped3;
        boundaryConditions[0].value = 0.0;

        boundaryConditions[1].type = QwtSpline::Clamped3;
        boundaryConditions[1].value = 0.0;
    }

    ~PrivateData()
    {
        delete parametrization;
    }

    QwtSplineParametrization* parametrization;
    QwtSpline::BoundaryType boundaryType;

    struct
    {
        int type;
        double value;

    } boundaryConditions[2];
};

/*!
   \fn QPainterPath QwtSpline::painterPath( const QPolygonF &points ) const

   Approximates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   \param points Control points
   \return Painter path, that can be rendered by QPainter

   \sa polygon(), QwtBezier
 */

/*!
   \brief Interpolate a curve by a polygon

   Interpolates a polygon piecewise with Bezier curves
   interpolating them in a 2nd pass by polygons.

   The interpolation is based on "Piecewise Linear Approximation of Bézier Curves"
   by Roger Willcocks ( http://www.rops.org )

   \param points Control points
   \param tolerance Maximum for the accepted error of the approximation

   \return polygon approximating the interpolating polynomials

   \sa bezierControlLines(), QwtBezier
 */
QPolygonF QwtSpline::polygon( const QPolygonF& points, double tolerance ) const
{
    if ( tolerance <= 0.0 )
        return QPolygonF();

    const QPainterPath path = painterPath( points );
    const int n = path.elementCount();
    if ( n == 0 )
        return QPolygonF();

    const QPainterPath::Element el = path.elementAt( 0 );
    if ( el.type != QPainterPath::MoveToElement )
        return QPolygonF();

    QPointF p1( el.x, el.y );

    QPolygonF polygon;
    QwtBezier bezier( tolerance );

    for ( int i = 1; i < n; i += 3 )
    {
        const QPainterPath::Element el1 = path.elementAt( i );
        const QPainterPath::Element el2 = path.elementAt( i + 1 );
        const QPainterPath::Element el3 = path.elementAt( i + 2 );

        const QPointF cp1( el1.x, el1.y );
        const QPointF cp2( el2.x, el2.y );
        const QPointF p2( el3.x, el3.y );

        bezier.appendToPolygon( p1, cp1, cp2, p2, polygon );

        p1 = p2;
    }

    return polygon;
}

/*!
   \brief Constructor

   The default setting is a non closing spline with chordal parametrization

   \sa setParametrization(), setBoundaryType()
 */
QwtSpline::QwtSpline()
{
    m_data = new PrivateData;
}

//! Destructor
QwtSpline::~QwtSpline()
{
    delete m_data;
}

/*!
   The locality of an spline interpolation identifies how many adjacent
   polynomials are affected, when changing the position of one point.

   A locality of 'n' means, that changing the coordinates of a point
   has an effect on 'n' leading and 'n' following polynomials.
   Those polynomials can be calculated from a local subpolygon.

   A value of 0 means, that the interpolation is not local and any modification
   of the polygon requires to recalculate all polynomials ( f.e cubic splines ).

   \return Order of locality
 */
uint QwtSpline::locality() const
{
    return 0;
}

/*!
   Define the parametrization for a parametric spline approximation
   The default setting is a chordal parametrization.

   \param type Type of parametrization, usually one of QwtSplineParametrization::Type
   \sa parametrization()
 */
void QwtSpline::setParametrization( int type )
{
    if ( m_data->parametrization->type() != type )
    {
        delete m_data->parametrization;
        m_data->parametrization = new QwtSplineParametrization( type );
    }
}

/*!
   Define the parametrization for a parametric spline approximation
   The default setting is a chordal parametrization.

   \param parametrization Parametrization
   \sa parametrization()
 */
void QwtSpline::setParametrization( QwtSplineParametrization* parametrization )
{
    if ( ( parametrization != NULL ) && ( m_data->parametrization != parametrization ) )
    {
        delete m_data->parametrization;
        m_data->parametrization = parametrization;
    }
}

/*!
   \return parametrization
   \sa setParametrization()
 */
const QwtSplineParametrization* QwtSpline::parametrization() const
{
    return m_data->parametrization;
}

/*!
   Define the boundary type for the endpoints of the approximating
   spline.

   \param boundaryType Boundary type
   \sa boundaryType()
 */
void QwtSpline::setBoundaryType( BoundaryType boundaryType )
{
    m_data->boundaryType = boundaryType;
}

/*!
   \return Boundary type
   \sa setBoundaryType()
 */
QwtSpline::BoundaryType QwtSpline::boundaryType() const
{
    return m_data->boundaryType;
}

/*!
   \brief Define the condition for an endpoint of the spline

   \param position At the beginning or the end of the spline
   \param condition Condition

   \sa BoundaryCondition, QwtSplineC2::BoundaryCondition, boundaryCondition()
 */
void QwtSpline::setBoundaryCondition( BoundaryPosition position, int condition )
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        m_data->boundaryConditions[position].type = condition;
}

/*!
   \return Condition for an endpoint of the spline
   \param position At the beginning or the end of the spline

   \sa setBoundaryCondition(), boundaryValue(), setBoundaryConditions()
 */
int QwtSpline::boundaryCondition( BoundaryPosition position ) const
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        return m_data->boundaryConditions[position].type;

    return m_data->boundaryConditions[0].type; // should never happen
}

/*!
   \brief Define the boundary value

   The boundary value is an parameter used in combination with
   the boundary condition. Its meaning depends on the condition.

   \param position At the beginning or the end of the spline
   \param value Value used for the condition at the end point

   \sa boundaryValue(), setBoundaryCondition()
 */
void QwtSpline::setBoundaryValue( BoundaryPosition position, double value )
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        m_data->boundaryConditions[position].value = value;
}

/*!
   \return Boundary value
   \param position At the beginning or the end of the spline

   \sa setBoundaryValue(), boundaryCondition()
 */
double QwtSpline::boundaryValue( BoundaryPosition position ) const
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        return m_data->boundaryConditions[position].value;

    return m_data->boundaryConditions[0].value; // should never happen
}

/*!
   \brief Define the condition at the endpoints of a spline

   \param condition Condition
   \param valueBegin Used for the condition at the beginning of te spline
   \param valueEnd Used for the condition at the end of te spline

   \sa BoundaryCondition, QwtSplineC2::BoundaryCondition,
      testBoundaryCondition(), setBoundaryValue()
 */
void QwtSpline::setBoundaryConditions(
    int condition, double valueBegin, double valueEnd )
{
    setBoundaryCondition( QwtSpline::AtBeginning, condition );
    setBoundaryValue( QwtSpline::AtBeginning, valueBegin );

    setBoundaryCondition( QwtSpline::AtEnd, condition );
    setBoundaryValue( QwtSpline::AtEnd, valueEnd );
}

//! \brief Constructor
QwtSplineInterpolating::QwtSplineInterpolating()
{
}

//! Destructor
QwtSplineInterpolating::~QwtSplineInterpolating()
{
}

/*! \fn QVector<QLineF> QwtSplineInterpolating::bezierControlLines( const QPolygonF &points ) const

   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns the 2 control points of each curve as QLineF.

   \param points Control points
   \return Control points of the interpolating Bezier curves
 */

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   The implementation calculates the Bezier control lines first
   and converts them into painter path elements in an additional loop.

   \param points Control points
   \return Painter path, that can be rendered by QPainter

   \note Derived spline classes might overload painterPath() to avoid
        the extra loops for converting results into a QPainterPath

   \sa bezierControlLines()
 */
QPainterPath QwtSplineInterpolating::painterPath( const QPolygonF& points ) const
{
    const int n = points.size();

    QPainterPath path;
    if ( n == 0 )
        return path;

    if ( n == 1 )
    {
        path.moveTo( points[0] );
        return path;
    }

    if ( n == 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QVector< QLineF > controlLines = bezierControlLines( points );
    if ( controlLines.size() < n - 1 )
        return path;

    const QPointF* p = points.constData();
    const QLineF* l = controlLines.constData();

    path.moveTo( p[0] );
    for ( int i = 0; i < n - 1; i++ )
        path.cubicTo( l[i].p1(), l[i].p2(), p[i + 1] );

    if ( ( boundaryType() == QwtSpline::ClosedPolygon )
        && ( controlLines.size() >= n ) )
    {
        path.cubicTo( l[n - 1].p1(), l[n - 1].p2(), p[0] );
        path.closeSubpath();
    }

    return path;
}

/*!
   \brief Interpolate a curve by a polygon

   Interpolates a polygon piecewise with Bezier curves
   approximating them by polygons.

   The approximation is based on "Piecewise Linear Approximation of Bézier Curves"
   by Roger Willcocks ( http://www.rops.org )

   \param points Control points
   \param tolerance Maximum for the accepted error of the approximation

   \return polygon approximating the interpolating polynomials

   \sa bezierControlLines(), QwtSplineBezier::toPolygon()
 */
QPolygonF QwtSplineInterpolating::polygon(
    const QPolygonF& points, double tolerance ) const
{
    if ( tolerance <= 0.0 )
        return QPolygonF();

    const QVector< QLineF > controlLines = bezierControlLines( points );
    if ( controlLines.isEmpty() )
        return QPolygonF();

    const bool isClosed = boundaryType() == QwtSpline::ClosedPolygon;

    QwtBezier bezier( tolerance );

    const QPointF* p = points.constData();
    const QLineF* cl = controlLines.constData();

    const int n = controlLines.size();

    QPolygonF polygon;

    for ( int i = 0; i < n - 1; i++ )
    {
        const QLineF& l = cl[i];
        bezier.appendToPolygon( p[i], l.p1(), l.p2(), p[i + 1], polygon );
    }

    const QPointF& pn = isClosed ? p[0] : p[n];
    const QLineF& l = cl[n - 1];

    bezier.appendToPolygon( p[n - 1], l.p1(), l.p2(), pn, polygon );

    return polygon;
}

/*!
   \brief Find an interpolated polygon with "equidistant" points

   When withNodes is disabled all points of the resulting polygon
   will be equidistant according to the parametrization.

   When withNodes is enabled the resulting polygon will also include
   the control points and the interpolated points are always aligned to
   the control point before ( points[i] + i * distance ).

   The implementation calculates bezier curves first and calculates
   the interpolated points in a second run.

   \param points Control nodes of the spline
   \param distance Distance between 2 points according
                  to the parametrization
   \param withNodes When true, also add the control
                   nodes ( even if not being equidistant )

   \return Interpolating polygon

   \sa bezierControlLines()
 */
QPolygonF QwtSplineInterpolating::equidistantPolygon( const QPolygonF& points,
    double distance, bool withNodes ) const
{
    if ( distance <= 0.0 )
        return QPolygonF();

    const int n = points.size();
    if ( n <= 1 )
        return points;

    if ( n == 2 )
    {
        // TODO
        return points;
    }

    QPolygonF path;

    const QVector< QLineF > controlLines = bezierControlLines( points );

    if ( controlLines.size() < n - 1 )
        return path;

    path += points.first();
    double t = distance;

    const QPointF* p = points.constData();
    const QLineF* cl = controlLines.constData();

    const QwtSplineParametrization* param = parametrization();

    for ( int i = 0; i < n - 1; i++ )
    {
        const double l = param->valueIncrement( p[i], p[i + 1] );

        while ( t < l )
        {
            path += QwtBezier::pointAt( p[i], cl[i].p1(),
                cl[i].p2(), p[i + 1], t / l );

            t += distance;
        }

        if ( withNodes )
        {
            if ( qFuzzyCompare( path.last().x(), p[i + 1].x() ) )
                path.last() = p[i + 1];
            else
                path += p[i + 1];

            t = distance;
        }
        else
        {
            t -= l;
        }
    }

    if ( ( boundaryType() == QwtSpline::ClosedPolygon )
        && ( controlLines.size() >= n ) )
    {
        const double l = param->valueIncrement( p[n - 1], p[0] );

        while ( t < l )
        {
            path += QwtBezier::pointAt( p[n - 1], cl[n - 1].p1(),
                cl[n - 1].p2(), p[0], t / l );

            t += distance;
        }

        if ( qFuzzyCompare( path.last().x(), p[0].x() ) )
            path.last() = p[0];
        else
            path += p[0];
    }

    return path;
}

//! Constructor
QwtSplineG1::QwtSplineG1()
{
}

//! Destructor
QwtSplineG1::~QwtSplineG1()
{
}

/*!
   \brief Constructor

   The default setting is a non closing spline with no parametrization
   ( QwtSplineParametrization::ParameterX ).

   \sa QwtSpline::setParametrization(),
      QwtSpline::setBoundaryType()
 */
QwtSplineC1::QwtSplineC1()
{
    setParametrization( QwtSplineParametrization::ParameterX );
}

//! Destructor
QwtSplineC1::~QwtSplineC1()
{
}

/*!
   \param points Control points
   \param slopeNext Value of the first derivative at the second point

   \return value of the first derivative at the first point
   \sa slopeAtEnd(), QwtSpline::boundaryCondition(), QwtSpline::boundaryValue()
 */
double QwtSplineC1::slopeAtBeginning( const QPolygonF& points, double slopeNext ) const
{
    if ( points.size() < 2 )
        return 0.0;

    return QwtSplineC1P::slopeBoundary(
        boundaryCondition( QwtSpline::AtBeginning ),
        boundaryValue( QwtSpline::AtBeginning ),
        points[0], points[1], slopeNext );
}

/*!
   \param points Control points
   \param slopeBefore Value of the first derivative at the point before the last one

   \return value of the first derivative at the last point
   \sa slopeAtBeginning(), QwtSpline::boundaryCondition(), QwtSpline::boundaryValue()
 */
double QwtSplineC1::slopeAtEnd( const QPolygonF& points, double slopeBefore ) const
{
    const int n = points.size();

    const QPointF p1( points[n - 1].x(), -points[n - 1].y() );
    const QPointF p2( points[n - 2].x(), -points[n - 2].y() );

    const int condition = boundaryCondition( QwtSpline::AtEnd );

    double value = boundaryValue( QwtSpline::AtEnd );
    if ( condition != QwtSpline::LinearRunout )
    {
        // beside LinearRunout the boundaryValue is a slope or curvature
        // and needs to be inverted too
        value = -value;
    }

    const double slope = QwtSplineC1P::slopeBoundary( condition, value, p1, p2, -slopeBefore );
    return -slope;
}

/*! \fn QVector<double> QwtSplineC1::slopes( const QPolygonF &points ) const

   \brief Find the first derivative at the control points

   \param points Control nodes of the spline
   \return Vector with the values of the 2nd derivate at the control points

   \note The x coordinates need to be increasing or decreasing
 */

/*!
   \brief Calculate an interpolated painter path

   Interpolates a polygon piecewise into cubic Bezier curves
   and returns them as QPainterPath.

   The implementation calculates the slopes at the control points
   and converts them into painter path elements in an additional loop.

   \param points Control points
   \return QPainterPath Painter path, that can be rendered by QPainter

   \note Derived spline classes might overload painterPath() to avoid
        the extra loops for converting results into a QPainterPath
 */
QPainterPath QwtSplineC1::painterPath( const QPolygonF& points ) const
{
    const int n = points.size();
    if ( n <= 2 )
        return QwtSplineInterpolating::painterPath( points );

    using namespace QwtSplineC1P;

    PathStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterX:
        {
            store = qwtSplineC1PathParamX< PathStore >( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterY:
        {
            store = qwtSplineC1PathParamY< PathStore >( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            store = qwtSplineC1PathParametric< PathStore >(
                this, points, paramUniform() );
            break;
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            store = qwtSplineC1PathParametric< PathStore >(
                this, points, paramCentripetal() );
            break;
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            store = qwtSplineC1PathParametric< PathStore >(
                this, points, paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplineC1PathParametric< PathStore >(
                this, points, param( parametrization() ) );
        }
    }

    return store.path;
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns the 2 control points of each curve as QLineF.

   \param points Control points
   \return Control points of the interpolating Bezier curves
 */
QVector< QLineF > QwtSplineC1::bezierControlLines( const QPolygonF& points ) const
{
    using namespace QwtSplineC1P;

    const int n = points.size();
    if ( n <= 2 )
        return QVector< QLineF >();

    ControlPointsStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterX:
        {
            store = qwtSplineC1PathParamX< ControlPointsStore >( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterY:
        {
            store = qwtSplineC1PathParamY< ControlPointsStore >( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            store = qwtSplineC1PathParametric< ControlPointsStore >(
                this, points, paramUniform() );
            break;
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            store = qwtSplineC1PathParametric< ControlPointsStore >(
                this, points, paramCentripetal() );
            break;
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            store = qwtSplineC1PathParametric< ControlPointsStore >(
                this, points, paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplineC1PathParametric< ControlPointsStore >(
                this, points, param( parametrization() ) );
        }
    }

    return store.controlPoints;
}

/*!
   \brief Find an interpolated polygon with "equidistant" points

   The implementation is optimzed for non parametric curves
   ( QwtSplineParametrization::ParameterX ) and falls back to
   QwtSpline::equidistantPolygon() otherwise.

   \param points Control nodes of the spline
   \param distance Distance between 2 points according
                  to the parametrization
   \param withNodes When true, also add the control
                   nodes ( even if not being equidistant )

   \return Interpolating polygon

   \sa QwtSpline::equidistantPolygon()
 */
QPolygonF QwtSplineC1::equidistantPolygon( const QPolygonF& points,
    double distance, bool withNodes ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        if ( points.size() > 2 )
        {
            const QVector< double > m = slopes( points );
            if ( m.size() != points.size() )
                return QPolygonF();

            return qwtPolygonParametric< QwtSplinePolynomial::fromSlopes >(
                distance, points, m, withNodes );
        }
    }

    return QwtSplineInterpolating::equidistantPolygon( points, distance, withNodes );
}

/*!
   \brief Calculate the interpolating polynomials for a non parametric spline

   C1 spline interpolations are based on finding values for the first
   derivates at the control points. The interpolating polynomials can
   be calculated from the the first derivates using QwtSplinePolynomial::fromSlopes().

   The default implementation is a two pass calculation. In derived classes it
   might be overloaded by a one pass implementation.

   \param points Control points
   \return Interpolating polynomials

   \note The x coordinates need to be increasing or decreasing
 */
QVector< QwtSplinePolynomial > QwtSplineC1::polynomials(
    const QPolygonF& points ) const
{
    QVector< QwtSplinePolynomial > polynomials;

    const QVector< double > m = slopes( points );
    if ( m.size() < 2 )
        return polynomials;

    polynomials.reserve( m.size() - 1 );
    for ( int i = 1; i < m.size(); i++ )
    {
        polynomials += QwtSplinePolynomial::fromSlopes(
            points[i - 1], m[i - 1], points[i], m[i] );
    }

    return polynomials;
}

/*!
   \brief Constructor

   The default setting is a non closing spline with no parametrization
   ( QwtSplineParametrization::ParameterX ).

   \sa QwtSpline::setParametrization(), QwtSpline::setBoundaryType()
 */
QwtSplineC2::QwtSplineC2()
{
}

//! Destructor
QwtSplineC2::~QwtSplineC2()
{
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns them as QPainterPath.

   \param points Control points
   \return Painter path, that can be rendered by QPainter

   \note The implementation simply calls QwtSplineC1::painterPath(), but is
        intended to be replaced by a one pass calculation some day.
 */
QPainterPath QwtSplineC2::painterPath( const QPolygonF& points ) const
{
    // could be implemented from curvatures without the extra
    // loop for calculating the slopes vector. TODO ...

    return QwtSplineC1::painterPath( points );
}

/*!
   \brief Interpolate a curve with Bezier curves

   Interpolates a polygon piecewise with cubic Bezier curves
   and returns the 2 control points of each curve as QLineF.

   \param points Control points
   \return Control points of the interpolating Bezier curves

   \note The implementation simply calls QwtSplineC1::bezierControlLines(),
        but is intended to be replaced by a more efficient implementation
        that builds the polynomials by the curvatures some day.
 */
QVector< QLineF > QwtSplineC2::bezierControlLines( const QPolygonF& points ) const
{
    // could be implemented from curvatures without the extra
    // loop for calculating the slopes vector. TODO ...

    return QwtSplineC1::bezierControlLines( points );
}

/*!
   \brief Find an interpolated polygon with "equidistant" points

   The implementation is optimzed for non parametric curves
   ( QwtSplineParametrization::ParameterX ) and falls back to
   QwtSpline::equidistantPolygon() otherwise.

   \param points Control nodes of the spline
   \param distance Distance between 2 points according
                  to the parametrization
   \param withNodes When true, also add the control
                   nodes ( even if not being equidistant )

   \return Interpolating polygon

   \sa QwtSpline::equidistantPolygon()
 */
QPolygonF QwtSplineC2::equidistantPolygon( const QPolygonF& points,
    double distance, bool withNodes ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        if ( points.size() > 2 )
        {
            const QVector< double > cv = curvatures( points );
            if ( cv.size() != points.size() )
                return QPolygonF();

            return qwtPolygonParametric< QwtSplinePolynomial::fromCurvatures >(
                distance, points, cv, withNodes );
        }
    }

    return QwtSplineInterpolating::equidistantPolygon( points, distance, withNodes );
}

/*! \fn QVector<double> QwtSplineC2::curvatures( const QPolygonF &points ) const

   \brief Find the second derivative at the control points

   \param points Control nodes of the spline
   \return Vector with the values of the 2nd derivate at the control points

   \sa slopes()
   \note The x coordinates need to be increasing or decreasing
 */

/*!
   \brief Find the first derivative at the control points

   An implementation calculating the 2nd derivatives and then building
   the slopes in a 2nd loop. QwtSplineCubic overloads it with a more
   performant implementation doing it in one loop.

   \param points Control nodes of the spline
   \return Vector with the values of the 1nd derivate at the control points

   \sa curvatures()

   \note The x coordinates need to be increasing or decreasing
 */
QVector< double > QwtSplineC2::slopes( const QPolygonF& points ) const
{
    const QVector< double > curvatures = this->curvatures( points );
    if ( curvatures.size() < 2 )
        return QVector< double >();

    QVector< double > slopes( curvatures.size() );

    const double* cv = curvatures.constData();
    double* m = slopes.data();

    const int n = points.size();
    const QPointF* p = points.constData();

    QwtSplinePolynomial polynomial;

    for ( int i = 0; i < n - 1; i++ )
    {
        polynomial = QwtSplinePolynomial::fromCurvatures( p[i], cv[i], p[i + 1], cv[i + 1] );
        m[i] = polynomial.c1;
    }

    m[n - 1] = polynomial.slopeAt( p[n - 1].x() - p[n - 2].x() );

    return slopes;
}

/*!
   \brief Calculate the interpolating polynomials for a non parametric spline

   C2 spline interpolations are based on finding values for the second
   derivates of f at the control points. The interpolating polynomials can
   be calculated from the the second derivates using QwtSplinePolynomial::fromCurvatures.

   The default implementation is a 2 pass calculation. In derived classes it
   might be overloaded by a one pass implementation.

   \param points Control points
   \return Interpolating polynomials

   \note The x coordinates need to be increasing or decreasing
 */
QVector< QwtSplinePolynomial > QwtSplineC2::polynomials( const QPolygonF& points ) const
{
    QVector< QwtSplinePolynomial > polynomials;

    const QVector< double > curvatures = this->curvatures( points );
    if ( curvatures.size() < 2 )
        return polynomials;

    const QPointF* p = points.constData();
    const double* cv = curvatures.constData();
    const int n = curvatures.size();
    polynomials.reserve( n - 1 );

    for ( int i = 1; i < n; i++ )
    {
        polynomials += QwtSplinePolynomial::fromCurvatures(
            p[i - 1], cv[i - 1], p[i], cv[i] );
    }

    return polynomials;
}
