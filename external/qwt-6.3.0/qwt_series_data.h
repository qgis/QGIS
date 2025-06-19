/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SERIES_DATA_H
#define QWT_SERIES_DATA_H

#include "qwt_global.h"
#include "qwt_samples.h"
#include "qwt_point_3d.h"

#include <qvector.h>
#include <qrect.h>

class QwtPointPolar;

/*!
   \brief Abstract interface for iterating over samples

   Qwt offers several implementations of the QwtSeriesData API,
   but in situations, where data of an application specific format
   needs to be displayed, without having to copy it, it is recommended
   to implement an individual data access.

   A subclass of QwtSeriesData<QPointF> must implement:

   - size()\n
     Should return number of data points.

   - sample()\n
     Should return values x and y values of the sample at specific position
     as QPointF object.

   - boundingRect()\n
     Should return the bounding rectangle of the data series.
     It is used for autoscaling and might help certain algorithms for displaying
     the data. You can use qwtBoundingRect() for an implementation
     but often it is possible to implement a more efficient algorithm
     depending on the characteristics of the series.
     The member cachedBoundingRect is intended for caching the calculated rectangle.

 */
template< typename T >
class QwtSeriesData
{
  public:
    //! Constructor
    QwtSeriesData();

    //! Destructor
    virtual ~QwtSeriesData();

#ifndef QWT_PYTHON_WRAPPER

    //! \return Number of samples
    virtual size_t size() const = 0;

    /*!
       Return a sample
       \param i Index
       \return Sample at position i
     */
    virtual T sample( size_t i ) const = 0;

#else
    // Needed for generating the python bindings, but not for using them !
    virtual size_t size() const { return 0; }
    virtual T sample( size_t i ) const { return T(); }
#endif

    /*!
        Returns the first sample in the list.
        \warning This function assumes that the list isn't empty.
     */
    inline T firstSample() const { return sample( 0 ); }

    /*!
        Returns the first sample in the list.
        \warning This function assumes that the list isn't empty.
     */
    inline T lastSample() const { return sample( size() - 1 ); }

    /*!
       Calculate the bounding rect of all samples

       The bounding rect is necessary for autoscaling and can be used
       for a couple of painting optimizations.

       qwtBoundingRect(...) offers slow implementations iterating
       over the samples. For large sets it is recommended to implement
       something faster.

       \return Bounding rectangle
     */
     virtual QRectF boundingRect() const
     {
        if ( cachedBoundingRect.width() < 0.0 )
            cachedBoundingRect = qwtBoundingRect( *this );

        return cachedBoundingRect;
     }

    /*!
       Set a the "rect of interest"

       QwtPlotSeriesItem defines the current area of the plot canvas
       as "rectangle of interest" ( QwtPlotSeriesItem::updateScaleDiv() ).
       It can be used to implement different levels of details.

       The default implementation does nothing.

       \param rect Rectangle of interest
     */
    virtual void setRectOfInterest( const QRectF& rect );

  protected:
    //! Can be used to cache a calculated bounding rectangle
    mutable QRectF cachedBoundingRect;

  private:
    QwtSeriesData< T >& operator=( const QwtSeriesData< T >& );
};

template< typename T >
QwtSeriesData< T >::QwtSeriesData()
    : cachedBoundingRect( 0.0, 0.0, -1.0, -1.0 )
{
}

template< typename T >
QwtSeriesData< T >::~QwtSeriesData()
{
}

template< typename T >
void QwtSeriesData< T >::setRectOfInterest( const QRectF& )
{
}

/*!
   \brief Template class for data, that is organized as QVector

   QVector uses implicit data sharing and can be
   passed around as argument efficiently.
 */
template< typename T >
class QwtArraySeriesData : public QwtSeriesData< T >
{
  public:
    //! Constructor
    QwtArraySeriesData();

    /*!
       Constructor
       \param samples Array of samples
     */
    explicit QwtArraySeriesData( const QVector< T >& samples );

    /*!
       Assign an array of samples
       \param samples Array of samples
     */
    void setSamples( const QVector< T >& samples );

    //! \return Array of samples
    const QVector< T > samples() const;

    //! \return Number of samples
    virtual size_t size() const QWT_OVERRIDE;

    /*!
       \return Sample at a specific position

       \param index Index
       \return Sample at position index
     */
    virtual T sample( size_t index ) const QWT_OVERRIDE;

  protected:
    //! Vector of samples
    QVector< T > m_samples;
};

template< typename T >
QwtArraySeriesData< T >::QwtArraySeriesData()
{
}

template< typename T >
QwtArraySeriesData< T >::QwtArraySeriesData( const QVector< T >& samples )
    : m_samples( samples )
{
}

template< typename T >
void QwtArraySeriesData< T >::setSamples( const QVector< T >& samples )
{
    QwtSeriesData< T >::cachedBoundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    m_samples = samples;
}

template< typename T >
const QVector< T > QwtArraySeriesData< T >::samples() const
{
    return m_samples;
}

template< typename T >
size_t QwtArraySeriesData< T >::size() const
{
    return m_samples.size();
}

template< typename T >
T QwtArraySeriesData< T >::sample( size_t i ) const
{
    return m_samples[ static_cast< int >( i ) ];
}

//! Interface for iterating over an array of points
typedef QwtArraySeriesData< QPointF > QwtPointSeriesData;

//! Interface for iterating over an array of 3D points
typedef QwtArraySeriesData< QwtPoint3D > QwtPoint3DSeriesData;

//! Interface for iterating over an array of intervals
typedef QwtArraySeriesData< QwtIntervalSample > QwtIntervalSeriesData;

//! Interface for iterating over an array of samples
typedef QwtArraySeriesData< QwtSetSample > QwtSetSeriesData;

//! Interface for iterating over an array of vector field samples
typedef QwtArraySeriesData< QwtVectorFieldSample > QwtVectorFieldData;

//!  Interface for iterating over an array of OHLC samples
typedef QwtArraySeriesData< QwtOHLCSample > QwtTradingChartData;

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QPointF >&, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QwtPoint3D >&, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QwtPointPolar >&, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QwtIntervalSample >&, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QwtSetSample >&, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QwtOHLCSample >&, int from = 0, int to = -1 );

QWT_EXPORT QRectF qwtBoundingRect(
    const QwtSeriesData< QwtVectorFieldSample >&, int from = 0, int to = -1 );

/*!
    Binary search for a sorted series of samples

    qwtUpperSampleIndex returns the index of sample that is the upper bound
    of value. Is the the value smaller than the smallest value the return
    value will be 0. Is the value greater or equal than the largest
    value the return value will be -1.

   \par Example
    The following example shows finds a point of curve from an x
    coordinate
    \code
 #include <qwt_series_data.h>
 #include <qwt_plot_curve.h>

      struct compareX
      {
          inline bool operator()( const double x, const QPointF &pos ) const
          {
              return ( x < pos.x() );
          }
      };

      QLineF curveLineAt( const QwtPlotCurve *curve, double x )
      {
          int index = qwtUpperSampleIndex<QPointF>(
 * curve->data(), x, compareX() );

          if ( index == -1 &&
              x == curve->sample( curve->dataSize() - 1 ).x() )
          {
              // the last sample is excluded from qwtUpperSampleIndex
              index = curve->dataSize() - 1;
          }

          QLineF line; // invalid
          if ( index > 0 )
          {
              line.setP1( curve->sample( index - 1 ) );
              line.setP2( curve->sample( index ) );
          }

          return line;
      }

    \endcode
   \endpar

   \param series Series of samples
   \param value Value
   \param lessThan Compare operation

   \note The samples must be sorted according to the order specified
        by the lessThan object
 */
template< typename T, typename LessThan >
inline int qwtUpperSampleIndex( const QwtSeriesData< T >& series,
    double value, LessThan lessThan  )
{
    const int indexMax = series.size() - 1;

    if ( indexMax < 0 || !lessThan( value, series.sample( indexMax ) ) )
        return -1;

    int indexMin = 0;
    int n = indexMax;

    while ( n > 0 )
    {
        const int half = n >> 1;
        const int indexMid = indexMin + half;

        if ( lessThan( value, series.sample( indexMid ) ) )
        {
            n = half;
        }
        else
        {
            indexMin = indexMid + 1;
            n -= half + 1;
        }
    }

    return indexMin;
}

#endif
