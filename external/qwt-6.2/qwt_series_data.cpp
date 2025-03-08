/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_series_data.h"
#include "qwt_point_polar.h"

static inline QRectF qwtBoundingRect( const QPointF& sample )
{
    return QRectF( sample.x(), sample.y(), 0.0, 0.0 );
}

static inline QRectF qwtBoundingRect( const QwtPoint3D& sample )
{
    return QRectF( sample.x(), sample.y(), 0.0, 0.0 );
}

static inline QRectF qwtBoundingRect( const QwtPointPolar& sample )
{
    return QRectF( sample.azimuth(), sample.radius(), 0.0, 0.0 );
}

static inline QRectF qwtBoundingRect( const QwtIntervalSample& sample )
{
    return QRectF( sample.interval.minValue(), sample.value,
        sample.interval.maxValue() - sample.interval.minValue(), 0.0 );
}

static inline QRectF qwtBoundingRect( const QwtSetSample& sample )
{
    if ( sample.set.empty() )
        return QRectF( sample.value, 0.0, 0.0, -1.0 );

    double minY = sample.set[0];
    double maxY = sample.set[0];

    for ( int i = 1; i < sample.set.size(); i++ )
    {
        if ( sample.set[i] < minY )
            minY = sample.set[i];

        if ( sample.set[i] > maxY )
            maxY = sample.set[i];
    }

    return QRectF( sample.value, minY, 0.0, maxY - minY );
}

static inline QRectF qwtBoundingRect( const QwtOHLCSample& sample )
{
    const QwtInterval interval = sample.boundingInterval();
    return QRectF( interval.minValue(), sample.time, interval.width(), 0.0 );
}

static inline QRectF qwtBoundingRect( const QwtVectorFieldSample& sample )
{
    /*
        When displaying a sample as an arrow its length will be
        proportional to the magnitude - but not the same.
        As the factor between length and magnitude is not known
        we can't include vx/vy into the bounding rectangle.
     */

    return QRectF( sample.x, sample.y, 0, 0 );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */

template< class T >
QRectF qwtBoundingRectT( const QwtSeriesData< T >& series, int from, int to )
{
    QRectF boundingRect( 1.0, 1.0, -2.0, -2.0 ); // invalid;

    if ( from < 0 )
        from = 0;

    if ( to < 0 )
        to = series.size() - 1;

    if ( to < from )
        return boundingRect;

    int i;
    for ( i = from; i <= to; i++ )
    {
        const QRectF rect = qwtBoundingRect( series.sample( i ) );
        if ( rect.width() >= 0.0 && rect.height() >= 0.0 )
        {
            boundingRect = rect;
            i++;
            break;
        }
    }

    for ( ; i <= to; i++ )
    {
        const QRectF rect = qwtBoundingRect( series.sample( i ) );
        if ( rect.width() >= 0.0 && rect.height() >= 0.0 )
        {
            boundingRect.setLeft( qMin( boundingRect.left(), rect.left() ) );
            boundingRect.setRight( qMax( boundingRect.right(), rect.right() ) );
            boundingRect.setTop( qMin( boundingRect.top(), rect.top() ) );
            boundingRect.setBottom( qMax( boundingRect.bottom(), rect.bottom() ) );
        }
    }

    return boundingRect;
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect( const QwtSeriesData< QPointF >& series, int from, int to )
{
    return qwtBoundingRectT< QPointF >( series, from, to );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect(
    const QwtSeriesData< QwtPoint3D >& series, int from, int to )
{
    return qwtBoundingRectT< QwtPoint3D >( series, from, to );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   The horizontal coordinates represent the azimuth, the
   vertical coordinates the radius.

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect(
    const QwtSeriesData< QwtPointPolar >& series, int from, int to )
{
    return qwtBoundingRectT< QwtPointPolar >( series, from, to );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect(
    const QwtSeriesData< QwtIntervalSample >& series, int from, int to )
{
    return qwtBoundingRectT< QwtIntervalSample >( series, from, to );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect(
    const QwtSeriesData< QwtOHLCSample >& series, int from, int to )
{
    return qwtBoundingRectT< QwtOHLCSample >( series, from, to );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect(
    const QwtSeriesData< QwtSetSample >& series, int from, int to )
{
    return qwtBoundingRectT< QwtSetSample >( series, from, to );
}

/*!
   \brief Calculate the bounding rectangle of a series subset

   Slow implementation, that iterates over the series.

   \param series Series
   \param from Index of the first sample, <= 0 means from the beginning
   \param to Index of the last sample, < 0 means to the end

   \return Bounding rectangle
 */
QRectF qwtBoundingRect(
    const QwtSeriesData< QwtVectorFieldSample >& series, int from, int to )
{
    return qwtBoundingRectT< QwtVectorFieldSample >( series, from, to );
}

/*!
   Constructor
   \param samples Samples
 */
QwtPointSeriesData::QwtPointSeriesData( const QVector< QPointF >& samples )
    : QwtArraySeriesData< QPointF >( samples )
{
}

/*!
   \brief Calculate the bounding rectangle

   The bounding rectangle is calculated once by iterating over all
   points and is stored for all following requests.

   \return Bounding rectangle
 */
QRectF QwtPointSeriesData::boundingRect() const
{
    if ( cachedBoundingRect.width() < 0.0 )
        cachedBoundingRect = qwtBoundingRect( *this );

    return cachedBoundingRect;
}

/*!
   Constructor
   \param samples Samples
 */
QwtPoint3DSeriesData::QwtPoint3DSeriesData(
        const QVector< QwtPoint3D >& samples )
    : QwtArraySeriesData< QwtPoint3D >( samples )
{
}

/*!
   \brief Calculate the bounding rectangle

   The bounding rectangle is calculated once by iterating over all
   points and is stored for all following requests.

   \return Bounding rectangle
 */
QRectF QwtPoint3DSeriesData::boundingRect() const
{
    if ( cachedBoundingRect.width() < 0.0 )
        cachedBoundingRect = qwtBoundingRect( *this );

    return cachedBoundingRect;
}

/*!
   Constructor
   \param samples Samples
 */
QwtIntervalSeriesData::QwtIntervalSeriesData(
        const QVector< QwtIntervalSample >& samples )
    : QwtArraySeriesData< QwtIntervalSample >( samples )
{
}

/*!
   \brief Calculate the bounding rectangle

   The bounding rectangle is calculated once by iterating over all
   points and is stored for all following requests.

   \return Bounding rectangle
 */
QRectF QwtIntervalSeriesData::boundingRect() const
{
    if ( cachedBoundingRect.width() < 0.0 )
        cachedBoundingRect = qwtBoundingRect( *this );

    return cachedBoundingRect;
}

/*!
   Constructor
   \param samples Samples
 */
QwtVectorFieldData::QwtVectorFieldData(
        const QVector< QwtVectorFieldSample >& samples )
    : QwtArraySeriesData< QwtVectorFieldSample >( samples )
{
}

/*!
   \brief Calculate the bounding rectangle

   The bounding rectangle is calculated once by iterating over all
   points and is stored for all following requests.

   \return Bounding rectangle
 */
QRectF QwtVectorFieldData::boundingRect() const
{
    if ( cachedBoundingRect.width() < 0.0 )
        cachedBoundingRect = qwtBoundingRect( *this );

    return cachedBoundingRect;
}

/*!
   Constructor
   \param samples Samples
 */
QwtSetSeriesData::QwtSetSeriesData( const QVector< QwtSetSample >& samples )
    : QwtArraySeriesData< QwtSetSample >( samples )
{
}

/*!
   \brief Calculate the bounding rectangle

   The bounding rectangle is calculated once by iterating over all
   points and is stored for all following requests.

   \return Bounding rectangle
 */
QRectF QwtSetSeriesData::boundingRect() const
{
    if ( cachedBoundingRect.width() < 0.0 )
        cachedBoundingRect = qwtBoundingRect( *this );

    return cachedBoundingRect;
}

/*!
   Constructor
   \param samples Samples
 */
QwtTradingChartData::QwtTradingChartData( const QVector< QwtOHLCSample >& samples )
    : QwtArraySeriesData< QwtOHLCSample >( samples )
{
}

/*!
   \brief Calculate the bounding rectangle

   The bounding rectangle is calculated once by iterating over all
   points and is stored for all following requests.

   \return Bounding rectangle
 */
QRectF QwtTradingChartData::boundingRect() const
{
    if ( cachedBoundingRect.width() < 0.0 )
        cachedBoundingRect = qwtBoundingRect( *this );

    return cachedBoundingRect;
}
