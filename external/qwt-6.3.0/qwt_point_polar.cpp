/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_point_polar.h"
#include "qwt_math.h"

#if QT_VERSION >= 0x050200

static QwtPointPolar qwtPointToPolar( const QPointF& point )
{
    return QwtPointPolar( point );
}

#endif

namespace
{
    static const struct RegisterQwtPointPolar
    {
        inline RegisterQwtPointPolar()
        {
            qRegisterMetaType< QwtPointPolar >();

#if QT_VERSION >= 0x050200
            QMetaType::registerConverter< QPointF, QwtPointPolar >( qwtPointToPolar );
            QMetaType::registerConverter< QwtPointPolar, QPointF >( &QwtPointPolar::toPoint );
#endif
        }

    } qwtRegisterQwtPointPolar;
}

/*!
   Convert and assign values from a point in Cartesian coordinates

   \param p Point in Cartesian coordinates
   \sa setPoint(), toPoint()
 */
QwtPointPolar::QwtPointPolar( const QPointF& p )
{
    m_radius = std::sqrt( qwtSqr( p.x() ) + qwtSqr( p.y() ) );
    m_azimuth = std::atan2( p.y(), p.x() );
}

/*!
   Convert and assign values from a point in Cartesian coordinates
   \param p Point in Cartesian coordinates
 */
void QwtPointPolar::setPoint( const QPointF& p )
{
    m_radius = std::sqrt( qwtSqr( p.x() ) + qwtSqr( p.y() ) );
    m_azimuth = std::atan2( p.y(), p.x() );
}

/*!
   Convert and return values in Cartesian coordinates

   \return Converted point in Cartesian coordinates

   \note Invalid or null points will be returned as QPointF(0.0, 0.0)
   \sa isValid(), isNull()
 */
QPointF QwtPointPolar::toPoint() const
{
    if ( m_radius <= 0.0 )
        return QPointF( 0.0, 0.0 );

    const double x = m_radius * std::cos( m_azimuth );
    const double y = m_radius * std::sin( m_azimuth );

    return QPointF( x, y );
}

/*!
    \brief Compare 2 points

    Two points are equal to each other if radius and
    azimuth-coordinates are the same. Points are not equal, when
    the azimuth differs, but other.azimuth() == azimuth() % (2 * PI).

    \return True if the point is equal to other; otherwise return false.

    \sa normalized()
 */
bool QwtPointPolar::operator==( const QwtPointPolar& other ) const
{
    return m_radius == other.m_radius && m_azimuth == other.m_azimuth;
}

/*!
    Compare 2 points

    Two points are equal to each other if radius and
    azimuth-coordinates are the same. Points are not equal, when
    the azimuth differs, but other.azimuth() == azimuth() % (2 * PI).

    \return True if the point is not equal to other; otherwise return false.
    \sa normalized()
 */
bool QwtPointPolar::operator!=( const QwtPointPolar& other ) const
{
    return m_radius != other.m_radius || m_azimuth != other.m_azimuth;
}

/*!
   Normalize radius and azimuth

   When the radius is < 0.0 it is set to 0.0. The azimuth is
   a value >= 0.0 and < 2 * M_PI.

   \return Normalized point
 */
QwtPointPolar QwtPointPolar::normalized() const
{
    const double radius = qwtMaxF( m_radius, 0.0 );

    double azimuth = m_azimuth;
    if ( azimuth < -2.0 * M_PI || azimuth >= 2 * M_PI )
        azimuth = std::fmod( m_azimuth, 2 * M_PI );

    if ( azimuth < 0.0 )
        azimuth += 2 * M_PI;

    return QwtPointPolar( azimuth, radius );
}

#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<( QDebug debug, const QwtPointPolar& point )
{
    debug.nospace() << "QwtPointPolar("
                    << point.azimuth() << "," << point.radius() << ")";

    return debug.space();
}

#endif

