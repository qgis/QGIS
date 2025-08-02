/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_clipper.h"
#include "qwt_point_polar.h"
#include "qwt_interval.h"
#include "qwt_math.h"

#include <qpolygon.h>
#include <qrect.h>

#include <algorithm>

namespace QwtClip
{
    // some templates used for inlining
    template< class Point, typename T > class LeftEdge;
    template< class Point, typename T > class RightEdge;
    template< class Point, typename T > class TopEdge;
    template< class Point, typename T > class BottomEdge;
}

template< class Point, typename Value >
class QwtClip::LeftEdge
{
  public:
    inline LeftEdge( Value x1, Value, Value, Value ):
        m_x1( x1 )
    {
    }

    inline bool isInside( const Point& p  ) const
    {
        return p.x() >= m_x1;
    }

    inline Point intersection( const Point& p1, const Point& p2 ) const
    {
        double dy = ( p1.y() - p2.y() ) / double( p1.x() - p2.x() );
        return Point( m_x1, static_cast< Value >( p2.y() + ( m_x1 - p2.x() ) * dy ) );
    }
  private:
    const Value m_x1;
};

template< class Point, typename Value >
class QwtClip::RightEdge
{
  public:
    inline RightEdge( Value, Value x2, Value, Value ):
        m_x2( x2 )
    {
    }

    inline bool isInside( const Point& p  ) const
    {
        return p.x() <= m_x2;
    }

    inline Point intersection( const Point& p1, const Point& p2 ) const
    {
        double dy = ( p1.y() - p2.y() ) / double( p1.x() - p2.x() );
        return Point( m_x2, static_cast< Value >( p2.y() + ( m_x2 - p2.x() ) * dy ) );
    }

  private:
    const Value m_x2;
};

template< class Point, typename Value >
class QwtClip::TopEdge
{
  public:
    inline TopEdge( Value, Value, Value y1, Value ):
        m_y1( y1 )
    {
    }

    inline bool isInside( const Point& p  ) const
    {
        return p.y() >= m_y1;
    }

    inline Point intersection( const Point& p1, const Point& p2 ) const
    {
        double dx = ( p1.x() - p2.x() ) / double( p1.y() - p2.y() );
        return Point( static_cast< Value >( p2.x() + ( m_y1 - p2.y() ) * dx ), m_y1 );
    }

  private:
    const Value m_y1;
};

template< class Point, typename Value >
class QwtClip::BottomEdge
{
  public:
    inline BottomEdge( Value, Value, Value, Value y2 ):
        m_y2( y2 )
    {
    }

    inline bool isInside( const Point& p ) const
    {
        return p.y() <= m_y2;
    }

    inline Point intersection( const Point& p1, const Point& p2 ) const
    {
        double dx = ( p1.x() - p2.x() ) / double( p1.y() - p2.y() );
        return Point( static_cast< Value >( p2.x() + ( m_y2 - p2.y() ) * dx ), m_y2 );
    }

  private:
    const Value m_y2;
};

using namespace QwtClip;

template< class Polygon, class Rect, typename T >
class QwtPolygonClipper
{
    typedef typename Polygon::value_type Point;
  public:
    explicit QwtPolygonClipper( const Rect& clipRect ):
        m_clipRect( clipRect )
    {
    }

    void clipPolygon( Polygon& points1, bool closePolygon ) const
    {
#if 0
        if ( m_clipRect.contains( points1.boundingRect() ) )
            return polygon;
#endif

        Polygon points2;
        points2.reserve( qMin( 256, points1.size() ) );

        clipEdge< LeftEdge< Point, T > >( closePolygon, points1, points2 );
        clipEdge< RightEdge< Point, T > >( closePolygon, points2, points1 );
        clipEdge< TopEdge< Point, T > >( closePolygon, points1, points2 );
        clipEdge< BottomEdge< Point, T > >( closePolygon, points2, points1 );
    }

  private:
    template< class Edge >
    inline void clipEdge( bool closePolygon,
        const Polygon& points, Polygon& clippedPoints ) const
    {
        clippedPoints.clear();

        if ( points.size() < 2 )
        {
            if ( points.size() == 1 )
                clippedPoints += points[0];

            return;
        }

        const Edge edge( m_clipRect.x(), m_clipRect.x() + m_clipRect.width(),
            m_clipRect.y(), m_clipRect.y() + m_clipRect.height() );

        if ( !closePolygon )
        {
            const Point& p1 = points.first();

            if ( edge.isInside( p1 ) )
                clippedPoints += p1;
        }
        else
        {
            const Point& p1 = points.first();
            const Point& p2 = points.last();

            if ( edge.isInside( p1 ) )
            {
                if ( !edge.isInside( p2 ) )
                    clippedPoints += edge.intersection( p1, p2 );

                clippedPoints += p1;
            }
            else if ( edge.isInside( p2 ) )
            {
                clippedPoints += edge.intersection( p1, p2 );
            }
        }

        const uint nPoints = points.size();
        const Point* p = points.constData();

        for ( uint i = 1; i < nPoints; i++ )
        {
            const Point& p1 = p[i];
            const Point& p2 = p[i - 1];

            if ( edge.isInside( p1 ) )
            {
                if ( !edge.isInside( p2 ) )
                    clippedPoints += edge.intersection( p1, p2 );

                clippedPoints += p1;
            }
            else if ( edge.isInside( p2 ) )
            {
                clippedPoints += edge.intersection( p1, p2 );
            }
        }
    }

    const Rect m_clipRect;
};

class QwtCircleClipper
{
  public:
    explicit QwtCircleClipper( const QRectF& r );
    QVector< QwtInterval > clipCircle( const QPointF&, double radius ) const;

  private:
    enum Edge
    {
        Left,
        Top,
        Right,
        Bottom,

        NEdges
    };

    QVector< QPointF > cuttingPoints(
        Edge, const QPointF& pos, double radius ) const;

    double toAngle( const QPointF&, const QPointF& ) const;

    const QRectF m_rect;
};


QwtCircleClipper::QwtCircleClipper( const QRectF& r )
    : m_rect( r )
{
}

QVector< QwtInterval > QwtCircleClipper::clipCircle(
    const QPointF& pos, double radius ) const
{
    // using QVarLengthArray TODO ...

    QVector< QPointF > points;
    for ( int edge = 0; edge < NEdges; edge++ )
        points += cuttingPoints( static_cast< Edge >( edge ), pos, radius );

    QVector< QwtInterval > intv;
    if ( points.size() <= 0 )
    {
        QRectF cRect( 0, 0, 2 * radius, 2 * radius );
        cRect.moveCenter( pos );
        if ( m_rect.contains( cRect ) )
            intv += QwtInterval( 0.0, 2 * M_PI );
    }
    else
    {
        QVector< double > angles;
        angles.reserve( points.size() );

        for ( int i = 0; i < points.size(); i++ )
            angles += toAngle( pos, points[i] );

        std::sort( angles.begin(), angles.end() );

        const int in = m_rect.contains( qwtPolar2Pos( pos, radius,
            angles[0] + ( angles[1] - angles[0] ) / 2 ) );

        intv.reserve( angles.size() / 2 );
        if ( in )
        {
            for ( int i = 0; i < angles.size() - 1; i += 2 )
                intv += QwtInterval( angles[i], angles[i + 1] );
        }
        else
        {
            for ( int i = 1; i < angles.size() - 1; i += 2 )
                intv += QwtInterval( angles[i], angles[i + 1] );

            intv += QwtInterval( angles.last(), angles.first() );
        }
    }

    return intv;
}

double QwtCircleClipper::toAngle(
    const QPointF& from, const QPointF& to ) const
{
    if ( from.x() == to.x() )
        return from.y() <= to.y() ? M_PI / 2.0 : 3 * M_PI / 2.0;

    const double m = qAbs( ( to.y() - from.y() ) / ( to.x() - from.x() ) );

    double angle = std::atan( m );
    if ( to.x() > from.x() )
    {
        if ( to.y() > from.y() )
            angle = 2 * M_PI - angle;
    }
    else
    {
        if ( to.y() > from.y() )
            angle = M_PI + angle;
        else
            angle = M_PI - angle;
    }

    return angle;
}

QVector< QPointF > QwtCircleClipper::cuttingPoints(
    Edge edge, const QPointF& pos, double radius ) const
{
    QVector< QPointF > points;

    if ( edge == Left || edge == Right )
    {
        const double x = ( edge == Left ) ? m_rect.left() : m_rect.right();
        if ( qAbs( pos.x() - x ) < radius )
        {
            const double off = std::sqrt( qwtSqr( radius ) - qwtSqr( pos.x() - x ) );
            const double m_y1 = pos.y() + off;
            if ( m_y1 >= m_rect.top() && m_y1 <= m_rect.bottom() )
                points += QPointF( x, m_y1 );

            const double m_y2 = pos.y() - off;
            if ( m_y2 >= m_rect.top() && m_y2 <= m_rect.bottom() )
                points += QPointF( x, m_y2 );
        }
    }
    else
    {
        const double y = ( edge == Top ) ? m_rect.top() : m_rect.bottom();
        if ( qAbs( pos.y() - y ) < radius )
        {
            const double off = std::sqrt( qwtSqr( radius ) - qwtSqr( pos.y() - y ) );
            const double x1 = pos.x() + off;
            if ( x1 >= m_rect.left() && x1 <= m_rect.right() )
                points += QPointF( x1, y );

            const double m_x2 = pos.x() - off;
            if ( m_x2 >= m_rect.left() && m_x2 <= m_rect.right() )
                points += QPointF( m_x2, y );
        }
    }
    return points;
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon IN/OUT
   \param closePolygon True, when the polygon is closed
 */
void QwtClipper::clipPolygon(
    const QRectF& clipRect, QPolygon& polygon, bool closePolygon )
{
    const int minX = qCeil( clipRect.left() );
    const int maxX = qFloor( clipRect.right() );
    const int minY = qCeil( clipRect.top() );
    const int maxY = qFloor( clipRect.bottom() );

    const QRect r( minX, minY, maxX - minX, maxY - minY );

    QwtPolygonClipper< QPolygon, QRect, int > clipper( r );
    clipper.clipPolygon( polygon, closePolygon );
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon IN/OUT
   \param closePolygon True, when the polygon is closed
 */
void QwtClipper::clipPolygon(
    const QRect& clipRect, QPolygon& polygon, bool closePolygon )
{
    QwtPolygonClipper< QPolygon, QRect, int > clipper( clipRect );
    clipper.clipPolygon( polygon, closePolygon );
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon IN/OUT
   \param closePolygon True, when the polygon is closed
 */
void QwtClipper::clipPolygonF(
    const QRectF& clipRect, QPolygonF& polygon, bool closePolygon )
{
    QwtPolygonClipper< QPolygonF, QRectF, double > clipper( clipRect );
    clipper.clipPolygon( polygon, closePolygon );
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon
   \param closePolygon True, when the polygon is closed

   \return Clipped polygon
 */
QPolygon QwtClipper::clippedPolygon(
    const QRectF& clipRect, const QPolygon& polygon, bool closePolygon )
{
    QPolygon points( polygon );
    clipPolygon( clipRect, points, closePolygon );

    return points;
}
/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon
   \param closePolygon True, when the polygon is closed

   \return Clipped polygon
 */
QPolygon QwtClipper::clippedPolygon(
    const QRect& clipRect, const QPolygon& polygon, bool closePolygon )
{
    QPolygon points( polygon );
    clipPolygon( clipRect, points, closePolygon );

    return points;
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon
   \param closePolygon True, when the polygon is closed

   \return Clipped polygon
 */
QPolygonF QwtClipper::clippedPolygonF(
    const QRectF& clipRect, const QPolygonF& polygon, bool closePolygon )
{
    QPolygonF points( polygon );
    clipPolygonF( clipRect, points, closePolygon );

    return points;
}

/*!
   Circle clipping

   clipCircle() divides a circle into intervals of angles representing arcs
   of the circle. When the circle is completely inside the clip rectangle
   an interval [0.0, 2 * M_PI] is returned.

   \param clipRect Clip rectangle
   \param center Center of the circle
   \param radius Radius of the circle

   \return Arcs of the circle
 */
QVector< QwtInterval > QwtClipper::clipCircle( const QRectF& clipRect,
    const QPointF& center, double radius )
{
    QwtCircleClipper clipper( clipRect );
    return clipper.clipCircle( center, radius );
}
