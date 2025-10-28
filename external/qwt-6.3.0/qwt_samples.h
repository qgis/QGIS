/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SAMPLES_H
#define QWT_SAMPLES_H

#include "qwt_global.h"
#include "qwt_interval.h"

#include <qvector.h>
#include <qrect.h>

//! \brief A sample of the types (x1-x2, y) or (x, y1-y2)
class QWT_EXPORT QwtIntervalSample
{
  public:
    QwtIntervalSample();
    QwtIntervalSample( double, const QwtInterval& );
    QwtIntervalSample( double value, double min, double max );

    bool operator==( const QwtIntervalSample& ) const;
    bool operator!=( const QwtIntervalSample& ) const;

    //! Value
    double value;

    //! Interval
    QwtInterval interval;
};

/*!
   Constructor
   The value is set to 0.0, the interval is invalid
 */
inline QwtIntervalSample::QwtIntervalSample()
    : value( 0.0 )
{
}

//! Constructor
inline QwtIntervalSample::QwtIntervalSample( double v, const QwtInterval& intv )
    : value( v )
    , interval( intv )
{
}

//! Constructor
inline QwtIntervalSample::QwtIntervalSample( double v, double min, double max )
    : value( v )
    , interval( min, max )
{
}

//! Compare operator
inline bool QwtIntervalSample::operator==( const QwtIntervalSample& other ) const
{
    return value == other.value && interval == other.interval;
}

//! Compare operator
inline bool QwtIntervalSample::operator!=( const QwtIntervalSample& other ) const
{
    return !( *this == other );
}

//! \brief A sample of the types (x1...xn, y) or (x, y1..yn)
class QWT_EXPORT QwtSetSample
{
  public:
    QwtSetSample();
    explicit QwtSetSample( double, const QVector< double >& = QVector< double >( ) );

    bool operator==( const QwtSetSample& other ) const;
    bool operator!=( const QwtSetSample& other ) const;

    double added() const;

    //! value
    double value;

    //! Vector of values associated to value
    QVector< double > set;
};

/*!
   Constructor
   The value is set to 0.0
 */
inline QwtSetSample::QwtSetSample()
    : value( 0.0 )
{
}

/*!
   Constructor

   \param v Value
   \param s Set of values
 */
inline QwtSetSample::QwtSetSample( double v, const QVector< double >& s )
    : value( v )
    , set( s )
{
}

//! Compare operator
inline bool QwtSetSample::operator==( const QwtSetSample& other ) const
{
    return value == other.value && set == other.set;
}

//! Compare operator
inline bool QwtSetSample::operator!=( const QwtSetSample& other ) const
{
    return !( *this == other );
}

//! \return All values of the set added
inline double QwtSetSample::added() const
{
    double y = 0.0;
    for ( int i = 0; i < set.size(); i++ )
        y += set[i];

    return y;
}

/*!
   \brief Open-High-Low-Close sample used in financial charts

   In financial charts the movement of a price in a time interval is often
   represented by the opening/closing prices and the lowest/highest prices
   in this interval.

   \sa QwtTradingChartData
 */
class QWT_EXPORT QwtOHLCSample
{
  public:
    QwtOHLCSample( double time = 0.0,
        double open = 0.0, double high = 0.0,
        double low = 0.0, double close = 0.0 );

    QwtInterval boundingInterval() const;

    bool isValid() const;

    /*!
       Time of the sample, usually a number representing
       a specific interval - like a day.
     */
    double time;

    //! Opening price
    double open;

    //! Highest price
    double high;

    //! Lowest price
    double low;

    //! Closing price
    double close;
};

/*!
   Constructor

   \param t Time value
   \param o Open value
   \param h High value
   \param l Low value
   \param c Close value
 */
inline QwtOHLCSample::QwtOHLCSample(
        double t, double o, double h, double l, double c )
    : time( t )
    , open( o )
    , high( h )
    , low( l )
    , close( c )
{
}

/*!
   \brief Check if a sample is valid

   A sample is valid, when all of the following checks are true:

   - low <= high
   - low <= open <= high
   - low <= close <= high

   \return True, when the sample is valid
 */
inline bool QwtOHLCSample::isValid() const
{
    return ( low <= high )
           && ( open >= low )
           && ( open <= high )
           && ( close >= low )
           && ( close <= high );
}

/*!
   \brief Calculate the bounding interval of the OHLC values

   For valid samples the limits of this interval are always low/high.

   \return Bounding interval
   \sa isValid()
 */
inline QwtInterval QwtOHLCSample::boundingInterval() const
{
    double minY = open;
    minY = qMin( minY, high );
    minY = qMin( minY, low );
    minY = qMin( minY, close );

    double maxY = open;
    maxY = qMax( maxY, high );
    maxY = qMax( maxY, low );
    maxY = qMax( maxY, close );

    return QwtInterval( minY, maxY );
}

/*!
   \brief Sample used in vector fields

   A vector field sample is a position and a vector - usually
   representing some direction and magnitude - attached to this position.

   \sa QwtVectorFieldData
 */
class QWT_EXPORT QwtVectorFieldSample
{
  public:
    QwtVectorFieldSample( double x = 0.0, double y = 0.0,
        double vx = 0.0, double vy = 0.0 );

    QwtVectorFieldSample( const QPointF& pos,
        double vx = 0.0, double vy = 0.0 );

    QPointF pos() const;

    bool isNull() const;

    //! x coordinate of the position
    double x;

    //! y coordinate of the position
    double y;

    //! x coordinate of the vector
    double vx;

    //! y coordinate of the vector
    double vy;
};

/*!
   \brief Constructor

   \param posX x coordinate of the position
   \param posY y coordinate of the position
   \param vectorX x coordinate of the vector
   \param vectorY y coordinate of the vector
 */
inline QwtVectorFieldSample::QwtVectorFieldSample(
        double posX, double posY, double vectorX, double vectorY )
    : x( posX )
    , y( posY )
    , vx( vectorX )
    , vy( vectorY )
{
}

/*!
   \brief Constructor

   \param pos Position
   \param vectorX x coordinate of the vector
   \param vectorY y coordinate of the vector
 */
inline QwtVectorFieldSample::QwtVectorFieldSample(
        const QPointF& pos, double vectorX, double vectorY )
    : x( pos.x() )
    , y( pos.y() )
    , vx( vectorX )
    , vy( vectorY )
{
}

//! \return x/y coordinates as QPointF
inline QPointF QwtVectorFieldSample::pos() const
{
    return QPointF( x, y );
}

//! \return true, if vx and vy are 0
inline bool QwtVectorFieldSample::isNull() const
{
    return ( vx == 0.0 ) && ( vy == 0.0 );
}

#endif
