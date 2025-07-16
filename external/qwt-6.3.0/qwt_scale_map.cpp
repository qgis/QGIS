/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_scale_map.h"
#include "qwt_math.h"

#include <qrect.h>
#include <qdebug.h>

/*!
   \brief Constructor

   The scale and paint device intervals are both set to [0,1].
 */
QwtScaleMap::QwtScaleMap()
    : m_s1( 0.0 )
    , m_s2( 1.0 )
    , m_p1( 0.0 )
    , m_p2( 1.0 )
    , m_cnv( 1.0 )
    , m_ts1( 0.0 )
    , m_transform( NULL )
{
}

//! Copy constructor
QwtScaleMap::QwtScaleMap( const QwtScaleMap& other )
    : m_s1( other.m_s1 )
    , m_s2( other.m_s2 )
    , m_p1( other.m_p1 )
    , m_p2( other.m_p2 )
    , m_cnv( other.m_cnv )
    , m_ts1( other.m_ts1 )
    , m_transform( NULL )
{
    if ( other.m_transform )
        m_transform = other.m_transform->copy();
}

/*!
   Destructor
 */
QwtScaleMap::~QwtScaleMap()
{
    delete m_transform;
}

//! Assignment operator
QwtScaleMap& QwtScaleMap::operator=( const QwtScaleMap& other )
{
    m_s1 = other.m_s1;
    m_s2 = other.m_s2;
    m_p1 = other.m_p1;
    m_p2 = other.m_p2;
    m_cnv = other.m_cnv;
    m_ts1 = other.m_ts1;

    delete m_transform;
    m_transform = NULL;

    if ( other.m_transform )
        m_transform = other.m_transform->copy();

    return *this;
}

/*!
   Initialize the map with a transformation
 */
void QwtScaleMap::setTransformation( QwtTransform* transform )
{
    if ( transform != m_transform )
    {
        delete m_transform;
        m_transform = transform;
    }

    setScaleInterval( m_s1, m_s2 );
}

//! Get the transformation
const QwtTransform* QwtScaleMap::transformation() const
{
    return m_transform;
}

/*!
   \brief Specify the borders of the scale interval
   \param s1 first border
   \param s2 second border
   \warning scales might be aligned to
           transformation depending boundaries
 */
void QwtScaleMap::setScaleInterval( double s1, double s2 )
{
    m_s1 = s1;
    m_s2 = s2;

    if ( m_transform )
    {
        m_s1 = m_transform->bounded( m_s1 );
        m_s2 = m_transform->bounded( m_s2 );
    }

    updateFactor();
}

/*!
   \brief Specify the borders of the paint device interval
   \param p1 first border
   \param p2 second border
 */
void QwtScaleMap::setPaintInterval( double p1, double p2 )
{
    m_p1 = p1;
    m_p2 = p2;

    updateFactor();
}

void QwtScaleMap::updateFactor()
{
    m_ts1 = m_s1;
    double ts2 = m_s2;

    if ( m_transform )
    {
        m_ts1 = m_transform->transform( m_ts1 );
        ts2 = m_transform->transform( ts2 );
    }

    m_cnv = 1.0;
    if ( m_ts1 != ts2 )
        m_cnv = ( m_p2 - m_p1 ) / ( ts2 - m_ts1 );
}

/*!
   Transform a rectangle from scale to paint coordinates

   \param xMap X map
   \param yMap Y map
   \param rect Rectangle in scale coordinates
   \return Rectangle in paint coordinates

   \sa invTransform()
 */
QRectF QwtScaleMap::transform( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& rect )
{
    double x1 = xMap.transform( rect.left() );
    double x2 = xMap.transform( rect.right() );
    double y1 = yMap.transform( rect.top() );
    double y2 = yMap.transform( rect.bottom() );

    if ( x2 < x1 )
        qSwap( x1, x2 );
    if ( y2 < y1 )
        qSwap( y1, y2 );

    if ( qwtFuzzyCompare( x1, 0.0, x2 - x1 ) == 0 )
        x1 = 0.0;
    if ( qwtFuzzyCompare( x2, 0.0, x2 - x1 ) == 0 )
        x2 = 0.0;
    if ( qwtFuzzyCompare( y1, 0.0, y2 - y1 ) == 0 )
        y1 = 0.0;
    if ( qwtFuzzyCompare( y2, 0.0, y2 - y1 ) == 0 )
        y2 = 0.0;

    return QRectF( x1, y1, x2 - x1 + 1, y2 - y1 + 1 );
}

/*!
   Transform a rectangle from paint to scale coordinates

   \param xMap X map
   \param yMap Y map
   \param pos Position in paint coordinates
   \return Position in scale coordinates
   \sa transform()
 */
QPointF QwtScaleMap::invTransform( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QPointF& pos )
{
    return QPointF(
        xMap.invTransform( pos.x() ),
        yMap.invTransform( pos.y() )
        );
}

/*!
   Transform a point from scale to paint coordinates

   \param xMap X map
   \param yMap Y map
   \param pos Position in scale coordinates
   \return Position in paint coordinates

   \sa invTransform()
 */
QPointF QwtScaleMap::transform( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QPointF& pos )
{
    return QPointF(
        xMap.transform( pos.x() ),
        yMap.transform( pos.y() )
        );
}

/*!
   Transform a rectangle from paint to scale coordinates

   \param xMap X map
   \param yMap Y map
   \param rect Rectangle in paint coordinates
   \return Rectangle in scale coordinates
   \sa transform()
 */
QRectF QwtScaleMap::invTransform( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& rect )
{
    const double x1 = xMap.invTransform( rect.left() );
    const double x2 = xMap.invTransform( rect.right() - 1 );
    const double y1 = yMap.invTransform( rect.top() );
    const double y2 = yMap.invTransform( rect.bottom() - 1 );

    const QRectF r( x1, y1, x2 - x1, y2 - y1 );
    return r.normalized();
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<( QDebug debug, const QwtScaleMap& map )
{
    debug.nospace() << "QwtScaleMap("
                    << map.transformation()
                    << ", s:" << map.s1() << "->" << map.s2()
                    << ", p:" << map.p1() << "->" << map.p2()
                    << ")";

    return debug.space();
}

#endif
