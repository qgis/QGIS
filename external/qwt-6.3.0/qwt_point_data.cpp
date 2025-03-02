/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_point_data.h"

/*!
   Constructor

   \param size Number of points
   \param interval Bounding interval for the points

   \sa setInterval(), setSize()
 */
QwtSyntheticPointData::QwtSyntheticPointData(
        size_t size, const QwtInterval& interval )
    : m_size( size )
    , m_interval( interval )
{
}

/*!
   Change the number of points

   \param size Number of points
   \sa size(), setInterval()
 */
void QwtSyntheticPointData::setSize( size_t size )
{
    m_size = size;
}

/*!
   \return Number of points
   \sa setSize(), interval()
 */
size_t QwtSyntheticPointData::size() const
{
    return m_size;
}

/*!
   Set the bounding interval

   \param interval Interval
   \sa interval(), setSize()
 */
void QwtSyntheticPointData::setInterval( const QwtInterval& interval )
{
    m_interval = interval.normalized();
}

/*!
   \return Bounding interval
   \sa setInterval(), size()
 */
QwtInterval QwtSyntheticPointData::interval() const
{
    return m_interval;
}

/*!
   Set a the "rectangle of interest"

   QwtPlotSeriesItem defines the current area of the plot canvas
   as "rect of interest" ( QwtPlotSeriesItem::updateScaleDiv() ).

   If interval().isValid() == false the x values are calculated
   in the interval rect.left() -> rect.right().

   \sa rectOfInterest()
 */
void QwtSyntheticPointData::setRectOfInterest( const QRectF& rect )
{
    m_rectOfInterest = rect;
    m_intervalOfInterest = QwtInterval(
        rect.left(), rect.right() ).normalized();
}

/*!
   \return "rectangle of interest"
   \sa setRectOfInterest()
 */
QRectF QwtSyntheticPointData::rectOfInterest() const
{
    return m_rectOfInterest;
}

/*!
   \brief Calculate the bounding rectangle

   This implementation iterates over all points, what could often
   be implemented much faster using the characteristics of the series.
   When there are many points it is recommended to overload and
   reimplement this method using the characteristics of the series
   ( if possible ).

   \return Bounding rectangle
 */
QRectF QwtSyntheticPointData::boundingRect() const
{
    if ( m_size == 0 ||
        !( m_interval.isValid() || m_intervalOfInterest.isValid() ) )
    {
        return QRectF( 1.0, 1.0, -2.0, -2.0 ); // something invalid
    }

    return qwtBoundingRect( *this );
}

/*!
   Calculate the point from an index

   \param index Index
   \return QPointF(x(index), y(x(index)));

   \warning For invalid indices ( index < 0 || index >= size() )
            (0, 0) is returned.
 */
QPointF QwtSyntheticPointData::sample( size_t index ) const
{
    if ( index >= m_size )
        return QPointF( 0, 0 );

    const double xValue = x( index );
    const double yValue = y( xValue );

    return QPointF( xValue, yValue );
}

/*!
   Calculate a x-value from an index

   x values are calculated by dividing an interval into
   equidistant steps. If !interval().isValid() the
   interval is calculated from the "rectangle of interest".

   \param index Index of the requested point
   \return Calculated x coordinate

   \sa interval(), rectOfInterest(), y()
 */
double QwtSyntheticPointData::x( uint index ) const
{
    const QwtInterval& interval = m_interval.isValid() ?
        m_interval : m_intervalOfInterest;

    if ( !interval.isValid() )
        return 0.0;

    if ( m_size <= 1 )
        return interval.minValue();

    const double dx = interval.width() / ( m_size - 1 );
    return interval.minValue() + index * dx;
}
