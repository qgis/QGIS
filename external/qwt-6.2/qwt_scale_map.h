/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SCALE_MAP_H
#define QWT_SCALE_MAP_H

#include "qwt_global.h"
#include "qwt_transform.h"

class QPointF;
class QRectF;

/*!
   \brief A scale map

   QwtScaleMap offers transformations from the coordinate system
   of a scale into the linear coordinate system of a paint device
   and vice versa.
 */
class QWT_EXPORT QwtScaleMap
{
  public:
    QwtScaleMap();
    QwtScaleMap( const QwtScaleMap& );

    ~QwtScaleMap();

    QwtScaleMap& operator=( const QwtScaleMap& );

    void setTransformation( QwtTransform* );
    const QwtTransform* transformation() const;

    void setPaintInterval( double p1, double p2 );
    void setScaleInterval( double s1, double s2 );

    double transform( double s ) const;
    double invTransform( double p ) const;

    double p1() const;
    double p2() const;

    double s1() const;
    double s2() const;

    double pDist() const;
    double sDist() const;

    static QRectF transform( const QwtScaleMap&,
        const QwtScaleMap&, const QRectF& );

    static QRectF invTransform( const QwtScaleMap&,
        const QwtScaleMap&, const QRectF& );

    static QPointF transform( const QwtScaleMap&,
        const QwtScaleMap&, const QPointF& );

    static QPointF invTransform( const QwtScaleMap&,
        const QwtScaleMap&, const QPointF& );

    bool isInverting() const;

  private:
    void updateFactor();

    double m_s1, m_s2;     // scale interval boundaries
    double m_p1, m_p2;     // paint device interval boundaries

    double m_cnv;       // conversion factor
    double m_ts1;

    QwtTransform* m_transform;
};

/*!
    \return First border of the scale interval
 */
inline double QwtScaleMap::s1() const
{
    return m_s1;
}

/*!
    \return Second border of the scale interval
 */
inline double QwtScaleMap::s2() const
{
    return m_s2;
}

/*!
    \return First border of the paint interval
 */
inline double QwtScaleMap::p1() const
{
    return m_p1;
}

/*!
    \return Second border of the paint interval
 */
inline double QwtScaleMap::p2() const
{
    return m_p2;
}

/*!
    \return qwtAbs(p2() - p1())
 */
inline double QwtScaleMap::pDist() const
{
    return qAbs( m_p2 - m_p1 );
}

/*!
    \return qwtAbs(s2() - s1())
 */
inline double QwtScaleMap::sDist() const
{
    return qAbs( m_s2 - m_s1 );
}

/*!
   Transform a point related to the scale interval into an point
   related to the interval of the paint device

   \param s Value relative to the coordinates of the scale
   \return Transformed value

   \sa invTransform()
 */
inline double QwtScaleMap::transform( double s ) const
{
    if ( m_transform )
        s = m_transform->transform( s );

    return m_p1 + ( s - m_ts1 ) * m_cnv;
}

/*!
   Transform an paint device value into a value in the
   interval of the scale.

   \param p Value relative to the coordinates of the paint device
   \return Transformed value

   \sa transform()
 */
inline double QwtScaleMap::invTransform( double p ) const
{
    double s = m_ts1 + ( p - m_p1 ) / m_cnv;
    if ( m_transform )
        s = m_transform->invTransform( s );

    return s;
}

//! \return True, when ( p1() < p2() ) != ( s1() < s2() )
inline bool QwtScaleMap::isInverting() const
{
    return ( ( m_p1 < m_p2 ) != ( m_s1 < m_s2 ) );
}

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<( QDebug, const QwtScaleMap& );
#endif

#endif
