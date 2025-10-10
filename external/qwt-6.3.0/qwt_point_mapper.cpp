/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_point_mapper.h"
#include "qwt_scale_map.h"
#include "qwt_pixel_matrix.h"
#include "qwt_series_data.h"
#include "qwt_math.h"

#include <qpolygon.h>
#include <qimage.h>
#include <qpen.h>
#include <qpainter.h>

#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>

#if !defined( QT_NO_QFUTURE )
#define QWT_USE_THREADS 1
#endif

static QRectF qwtInvalidRect( 0.0, 0.0, -1.0, -1.0 );

static inline int qwtRoundValue( double value )
{
    return qRound( value );
}

static inline double qwtRoundValueF( double value )
{
#if 1
    // MS Windows and at least IRIX does not have C99's nearbyint() function
    return ( value >= 0.0 ) ? std::floor( value + 0.5 ) : std::ceil( value - 0.5 );
#else
    return nearbyint( value );
#endif
}

static Qt::Orientation qwtProbeOrientation(
    const QwtSeriesData< QPointF >* series, int from, int to )
{
    if ( to - from < 20 )
    {
        // not enough points to "have an orientation"
        return Qt::Horizontal;
    }

    const double x0 = series->sample( from ).x();
    const double xn = series->sample( to ).x();

    if ( x0 == xn )
        return Qt::Vertical;

    const int step = ( to - from ) / 10;
    const bool isIncreasing = xn > x0;

    double x1 = x0;
    for ( int i = from + step; i < to; i += step )
    {
        const double x2 = series->sample( i ).x();
        if ( x2 != x1 )
        {
            if ( ( x2 > x1 ) != isIncreasing )
                return Qt::Vertical;
        }

        x1 = x2;
    }

    return Qt::Horizontal;
}

namespace
{
    template< class Polygon, class Point >
    class QwtPolygonQuadrupelX
    {
      public:
        inline void start( int x, int y )
        {
            m_x0 = x;
            m_y1 = m_yMin = m_yMax = m_y2 = y;
        }

        inline bool append( int x, int y )
        {
            if ( m_x0 != x )
                return false;

            if ( y < m_yMin )
                m_yMin = y;
            else if ( y > m_yMax )
                m_yMax = y;

            m_y2 = y;

            return true;
        }

        inline void flush( Polygon& polyline )
        {
            appendTo( m_y1, polyline );

            if ( m_y2 > m_y1 )
                qSwap( m_yMin, m_yMax );

            if ( m_yMax != m_y1 )
                appendTo( m_yMax, polyline );

            if ( m_yMin != m_yMax )
                appendTo( m_yMin, polyline );

            if ( m_y2 != m_yMin )
                appendTo( m_y2, polyline );
        }

      private:
        inline void appendTo( int y, Polygon& polyline )
        {
            polyline += Point( m_x0, y );
        }

      private:
        int m_x0, m_y1, m_yMin, m_yMax, m_y2;
    };

    template< class Polygon, class Point >
    class QwtPolygonQuadrupelY
    {
      public:
        inline void start( int x, int y )
        {
            m_y0 = y;
            m_x1 = m_xMin = m_xMax = m_x2 = x;
        }

        inline bool append( int x, int y )
        {
            if ( m_y0 != y )
                return false;

            if ( x < m_xMin )
                m_xMin = x;
            else if ( x > m_xMax )
                m_xMax = x;

            m_x2 = x;

            return true;
        }

        inline void flush( Polygon& polyline )
        {
            appendTo( m_x1, polyline );

            if ( m_x2 > m_x1 )
                qSwap( m_xMin, m_xMax );

            if ( m_xMax != m_x1 )
                appendTo( m_xMax, polyline );

            if ( m_xMin != m_xMax )
                appendTo( m_xMin, polyline );

            if ( m_x2 != m_xMin )
                appendTo( m_x2, polyline );
        }

      private:
        inline void appendTo( int x, Polygon& polyline )
        {
            polyline += Point( x, m_y0 );
        }

        int m_y0, m_x1, m_xMin, m_xMax, m_x2;
    };
}

template< class Polygon, class Point, class PolygonQuadrupel >
static Polygon qwtMapPointsQuad( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to )
{
    const QPointF sample0 = series->sample( from );

    PolygonQuadrupel q;
    q.start( qwtRoundValue( xMap.transform( sample0.x() ) ),
        qwtRoundValue( yMap.transform( sample0.y() ) ) );

    Polygon polyline;
    for ( int i = from; i <= to; i++ )
    {
        const QPointF sample = series->sample( i );

        const int x = qwtRoundValue( xMap.transform( sample.x() ) );
        const int y = qwtRoundValue( yMap.transform( sample.y() ) );

        if ( !q.append( x, y ) )
        {
            q.flush( polyline );
            q.start( x, y );
        }
    }
    q.flush( polyline );

    return polyline;
}

template< class Polygon, class Point, class PolygonQuadrupel >
static Polygon qwtMapPointsQuad( const Polygon& polyline )
{
    const int numPoints = polyline.size();

    if ( numPoints < 3 )
        return polyline;

    const Point* points = polyline.constData();

    Polygon polylineXY;

    PolygonQuadrupel q;
    q.start( points[0].x(), points[0].y() );

    for ( int i = 0; i < numPoints; i++ )
    {
        const int x = points[i].x();
        const int y = points[i].y();

        if ( !q.append( x, y ) )
        {
            q.flush( polylineXY );
            q.start( x, y );
        }
    }
    q.flush( polylineXY );

    return polylineXY;
}


template< class Polygon, class Point >
static Polygon qwtMapPointsQuad( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to )
{
    Polygon polyline;
    if ( from > to )
        return polyline;

    /*
        probing some values, to decide if it is better
        to start with x or y coordinates
     */
    const Qt::Orientation orientation = qwtProbeOrientation( series, from, to );

    if ( orientation == Qt::Horizontal )
    {
        polyline = qwtMapPointsQuad< Polygon, Point,
            QwtPolygonQuadrupelY< Polygon, Point > >( xMap, yMap, series, from, to );

        polyline = qwtMapPointsQuad< Polygon, Point,
            QwtPolygonQuadrupelX< Polygon, Point > >( polyline );
    }
    else
    {
        polyline = qwtMapPointsQuad< Polygon, Point,
            QwtPolygonQuadrupelX< Polygon, Point > >( xMap, yMap, series, from, to );

        polyline = qwtMapPointsQuad< Polygon, Point,
            QwtPolygonQuadrupelY< Polygon, Point > >( polyline );
    }

    return polyline;
}

// Helper class to work around the 5 parameters
// limitation of QtConcurrent::run()
class QwtDotsCommand
{
  public:
    const QwtSeriesData< QPointF >* series;
    int from;
    int to;
    QRgb rgb;
};

static void qwtRenderDots(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtDotsCommand& command, const QPoint& pos, QImage* image )
{
    const QRgb rgb = command.rgb;
    QRgb* bits = reinterpret_cast< QRgb* >( image->bits() );

    const int w = image->width();
    const int h = image->height();

    const int x0 = pos.x();
    const int y0 = pos.y();

    for ( int i = command.from; i <= command.to; i++ )
    {
        const QPointF sample = command.series->sample( i );

        const int x = static_cast< int >( xMap.transform( sample.x() ) + 0.5 ) - x0;
        const int y = static_cast< int >( yMap.transform( sample.y() ) + 0.5 ) - y0;

        if ( x >= 0 && x < w && y >= 0 && y < h )
            bits[ y * w + x ] = rgb;
    }
}

// some functors, so that the compile can inline
struct QwtRoundI
{
    inline int operator()( double value ) const
    {
        return qwtRoundValue( value );
    }
};

struct QwtRoundF
{
    inline double operator()( double value ) const
    {
        return qwtRoundValueF( value );
    }
};

struct QwtNoRoundF
{
    inline double operator()( double value ) const
    {
        return value;
    }
};

// mapping points without any filtering - beside checking
// the bounding rectangle

template< class Polygon, class Point, class Round >
static inline Polygon qwtToPoints(
    const QRectF& boundingRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series,
    int from, int to, Round round )
{
    Polygon polyline( to - from + 1 );
    Point* points = polyline.data();

    int numPoints = 0;

    if ( boundingRect.isValid() )
    {
        // iterating over all values
        // filtering out all points outside of
        // the bounding rectangle

        for ( int i = from; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            const double x = xMap.transform( sample.x() );
            const double y = yMap.transform( sample.y() );

            if ( boundingRect.contains( x, y ) )
            {
                points[ numPoints ].rx() = round( x );
                points[ numPoints ].ry() = round( y );

                numPoints++;
            }
        }

        polyline.resize( numPoints );
    }
    else
    {
        // simply iterating over all values
        // without any filtering

        for ( int i = from; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            const double x = xMap.transform( sample.x() );
            const double y = yMap.transform( sample.y() );

            points[ numPoints ].rx() = round( x );
            points[ numPoints ].ry() = round( y );

            numPoints++;
        }
    }

    return polyline;
}

static inline QPolygon qwtToPointsI(
    const QRectF& boundingRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series,
    int from, int to )
{
    return qwtToPoints< QPolygon, QPoint >(
        boundingRect, xMap, yMap, series, from, to, QwtRoundI() );
}

template< class Round >
static inline QPolygonF qwtToPointsF(
    const QRectF& boundingRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series,
    int from, int to, Round round )
{
    return qwtToPoints< QPolygonF, QPointF >(
        boundingRect, xMap, yMap, series, from, to, round );
}

// Mapping points with filtering out consecutive
// points mapped to the same position

template< class Polygon, class Point, class Round >
static inline Polygon qwtToPolylineFiltered(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series,
    int from, int to, Round round )
{
    // in curves with many points consecutive points
    // are often mapped to the same position. As this might
    // result in empty lines ( or symbols hidden by others )
    // we try to filter them out

    Polygon polyline( to - from + 1 );
    Point* points = polyline.data();

    const QPointF sample0 = series->sample( from );

    points[0].rx() = round( xMap.transform( sample0.x() ) );
    points[0].ry() = round( yMap.transform( sample0.y() ) );

    int pos = 0;
    for ( int i = from + 1; i <= to; i++ )
    {
        const QPointF sample = series->sample( i );

        const Point p( round( xMap.transform( sample.x() ) ),
            round( yMap.transform( sample.y() ) ) );

        if ( points[pos] != p )
            points[++pos] = p;
    }

    polyline.resize( pos + 1 );
    return polyline;
}

static inline QPolygon qwtToPolylineFilteredI(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series,
    int from, int to )
{
    return qwtToPolylineFiltered< QPolygon, QPoint >(
        xMap, yMap, series, from, to, QwtRoundI() );
}

template< class Round >
static inline QPolygonF qwtToPolylineFilteredF(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series,
    int from, int to, Round round )
{
    return qwtToPolylineFiltered< QPolygonF, QPointF >(
        xMap, yMap, series, from, to, round );
}

template< class Polygon, class Point >
static inline Polygon qwtToPointsFiltered(
    const QRectF& boundingRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to )
{
    // F.e. in scatter plots ( no connecting lines ) we
    // can sort out all duplicates ( not only consecutive points )

    Polygon polygon( to - from + 1 );
    Point* points = polygon.data();

    QwtPixelMatrix pixelMatrix( boundingRect.toAlignedRect() );

    int numPoints = 0;
    for ( int i = from; i <= to; i++ )
    {
        const QPointF sample = series->sample( i );

        const int x = qwtRoundValue( xMap.transform( sample.x() ) );
        const int y = qwtRoundValue( yMap.transform( sample.y() ) );

        if ( pixelMatrix.testAndSetPixel( x, y, true ) == false )
        {
            points[ numPoints ].rx() = x;
            points[ numPoints ].ry() = y;

            numPoints++;
        }
    }

    polygon.resize( numPoints );
    return polygon;
}

static inline QPolygon qwtToPointsFilteredI(
    const QRectF& boundingRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to )
{
    return qwtToPointsFiltered< QPolygon, QPoint >(
        boundingRect, xMap, yMap, series, from, to );
}

static inline QPolygonF qwtToPointsFilteredF(
    const QRectF& boundingRect,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to )
{
    return qwtToPointsFiltered< QPolygonF, QPointF >(
        boundingRect, xMap, yMap, series, from, to );
}

class QwtPointMapper::PrivateData
{
  public:
    PrivateData()
        : boundingRect( qwtInvalidRect )
    {
    }

    QRectF boundingRect;
    QwtPointMapper::TransformationFlags flags;
};

//! Constructor
QwtPointMapper::QwtPointMapper()
{
    m_data = new PrivateData();
}

//! Destructor
QwtPointMapper::~QwtPointMapper()
{
    delete m_data;
}

/*!
   Set the flags affecting the transformation process

   \param flags Flags
   \sa flags(), setFlag()
 */
void QwtPointMapper::setFlags( TransformationFlags flags )
{
    m_data->flags = flags;
}

/*!
   \return Flags affecting the transformation process
   \sa setFlags(), setFlag()
 */
QwtPointMapper::TransformationFlags QwtPointMapper::flags() const
{
    return m_data->flags;
}

/*!
   Modify a flag affecting the transformation process

   \param flag Flag type
   \param on Value

   \sa flag(), setFlags()
 */
void QwtPointMapper::setFlag( TransformationFlag flag, bool on )
{
    if ( on )
        m_data->flags |= flag;
    else
        m_data->flags &= ~flag;
}

/*!
   \return True, when the flag is set
   \param flag Flag type
   \sa setFlag(), setFlags()
 */
bool QwtPointMapper::testFlag( TransformationFlag flag ) const
{
    return m_data->flags & flag;
}

/*!
   Set a bounding rectangle for the point mapping algorithm

   A valid bounding rectangle can be used for optimizations

   \param rect Bounding rectangle
   \sa boundingRect()
 */
void QwtPointMapper::setBoundingRect( const QRectF& rect )
{
    m_data->boundingRect = rect;
}

/*!
   \return Bounding rectangle
   \sa setBoundingRect()
 */
QRectF QwtPointMapper::boundingRect() const
{
    return m_data->boundingRect;
}

/*!
   \brief Translate a series of points into a QPolygonF

   When the WeedOutPoints flag is enabled consecutive points,
   that are mapped to the same position will be one point.

   When RoundPoints is set all points are rounded to integers
   but returned as PolygonF - what only makes sense
   when the further processing of the values need a QPolygonF.

   When RoundPoints & WeedOutIntermediatePoints is enabled an even more
   aggressive weeding algorithm is enabled.

   \param xMap x map
   \param yMap y map
   \param series Series of points to be mapped
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted

   \return Translated polygon
 */
QPolygonF QwtPointMapper::toPolygonF(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to ) const
{
    QPolygonF polyline;

    if ( m_data->flags & RoundPoints )
    {
        if ( m_data->flags & WeedOutIntermediatePoints )
        {
            polyline = qwtMapPointsQuad< QPolygonF, QPointF >(
                xMap, yMap, series, from, to );
        }
        else if ( m_data->flags & WeedOutPoints )
        {
            polyline = qwtToPolylineFilteredF(
                xMap, yMap, series, from, to, QwtRoundF() );
        }
        else
        {
            polyline = qwtToPointsF( qwtInvalidRect,
                xMap, yMap, series, from, to, QwtRoundF() );
        }
    }
    else
    {
        if ( m_data->flags & WeedOutPoints )
        {
            polyline = qwtToPolylineFilteredF(
                xMap, yMap, series, from, to, QwtNoRoundF() );
        }
        else
        {
            polyline = qwtToPointsF( qwtInvalidRect,
                xMap, yMap, series, from, to, QwtNoRoundF() );
        }
    }

    return polyline;
}

/*!
   \brief Translate a series of points into a QPolygon

   When the WeedOutPoints flag is enabled consecutive points,
   that are mapped to the same position will be one point.

   \param xMap x map
   \param yMap y map
   \param series Series of points to be mapped
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted

   \return Translated polygon
 */
QPolygon QwtPointMapper::toPolygon(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to ) const
{
    QPolygon polyline;

    if ( m_data->flags & WeedOutIntermediatePoints )
    {
        // TODO WeedOutIntermediatePointsY ...
        polyline = qwtMapPointsQuad< QPolygon, QPoint >(
            xMap, yMap, series, from, to );
    }
    else if ( m_data->flags & WeedOutPoints )
    {
        polyline = qwtToPolylineFilteredI(
            xMap, yMap, series, from, to );
    }
    else
    {
        polyline = qwtToPointsI(
            qwtInvalidRect, xMap, yMap, series, from, to );
    }

    return polyline;
}

/*!
   \brief Translate a series into a QPolygonF

   - WeedOutPoints & RoundPoints & boundingRect().isValid()
    All points that are mapped to the same position
    will be one point. Points outside of the bounding
    rectangle are ignored.

   - WeedOutPoints & RoundPoints & !boundingRect().isValid()
    All consecutive points that are mapped to the same position
    will one point

   - WeedOutPoints & !RoundPoints
    All consecutive points that are mapped to the same position
    will one point

   - !WeedOutPoints & boundingRect().isValid()
    Points outside of the bounding rectangle are ignored.

   When RoundPoints is set all points are rounded to integers
   but returned as PolygonF - what only makes sense
   when the further processing of the values need a QPolygonF.

   \param xMap x map
   \param yMap y map
   \param series Series of points to be mapped
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted

   \return Translated polygon
 */
QPolygonF QwtPointMapper::toPointsF(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to ) const
{
    QPolygonF points;

    if ( m_data->flags & WeedOutPoints )
    {
        if ( m_data->flags & RoundPoints )
        {
            if ( m_data->boundingRect.isValid() )
            {
                points = qwtToPointsFilteredF( m_data->boundingRect,
                    xMap, yMap, series, from, to );
            }
            else
            {
                // without a bounding rectangle all we can
                // do is to filter out duplicates of
                // consecutive points

                points = qwtToPolylineFilteredF(
                    xMap, yMap, series, from, to, QwtRoundF() );
            }
        }
        else
        {
            // when rounding is not allowed we can't use
            // qwtToPointsFilteredF

            points = qwtToPolylineFilteredF(
                xMap, yMap, series, from, to, QwtNoRoundF() );
        }
    }
    else
    {
        if ( m_data->flags & RoundPoints )
        {
            points = qwtToPointsF( m_data->boundingRect,
                xMap, yMap, series, from, to, QwtRoundF() );
        }
        else
        {
            points = qwtToPointsF( m_data->boundingRect,
                xMap, yMap, series, from, to, QwtNoRoundF() );
        }
    }

    return points;
}

/*!
   \brief Translate a series of points into a QPolygon

   - WeedOutPoints & boundingRect().isValid()
    All points that are mapped to the same position
    will be one point. Points outside of the bounding
    rectangle are ignored.

   - WeedOutPoints & !boundingRect().isValid()
    All consecutive points that are mapped to the same position
    will one point

   - !WeedOutPoints & boundingRect().isValid()
    Points outside of the bounding rectangle are ignored.

   \param xMap x map
   \param yMap y map
   \param series Series of points to be mapped
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted

   \return Translated polygon
 */
QPolygon QwtPointMapper::toPoints(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to ) const
{
    QPolygon points;

    if ( m_data->flags & WeedOutPoints )
    {
        if ( m_data->boundingRect.isValid() )
        {
            points = qwtToPointsFilteredI( m_data->boundingRect,
                xMap, yMap, series, from, to );
        }
        else
        {
            // when we don't have the bounding rectangle all
            // we can do is to filter out consecutive duplicates

            points = qwtToPolylineFilteredI(
                xMap, yMap, series, from, to );
        }
    }
    else
    {
        points = qwtToPointsI(
            m_data->boundingRect, xMap, yMap, series, from, to );
    }

    return points;
}


/*!
   \brief Translate a series into a QImage

   \param xMap x map
   \param yMap y map
   \param series Series of points to be mapped
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted
   \param pen Pen used for drawing a point
             of the image, where a point is mapped to
   \param antialiased True, when the dots should be displayed
                     antialiased
   \param numThreads Number of threads to be used for rendering.
                   If numThreads is set to 0, the system specific
                   ideal thread count is used.

   \return Image displaying the series
 */
QImage QwtPointMapper::toImage(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QwtSeriesData< QPointF >* series, int from, int to,
    const QPen& pen, bool antialiased, uint numThreads ) const
{
    Q_UNUSED( antialiased )

#if QWT_USE_THREADS
    if ( numThreads == 0 )
        numThreads = QThread::idealThreadCount();

    if ( numThreads <= 0 )
        numThreads = 1;
#else
    Q_UNUSED( numThreads )
#endif

    // a very special optimization for scatter plots
    // where every sample is mapped to one pixel only.

    const QRect rect = m_data->boundingRect.toAlignedRect();

    QImage image( rect.size(), QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    if ( pen.width() <= 1 && pen.color().alpha() == 255 )
    {
        QwtDotsCommand command;
        command.series = series;
        command.rgb = pen.color().rgba();

#if QWT_USE_THREADS
        const int numPoints = ( to - from + 1 ) / numThreads;

        QList< QFuture< void > > futures;
        for ( uint i = 0; i < numThreads; i++ )
        {
            const QPoint pos = rect.topLeft();

            const int index0 = from + i * numPoints;
            if ( i == numThreads - 1 )
            {
                command.from = index0;
                command.to = to;

                qwtRenderDots( xMap, yMap, command, pos, &image );
            }
            else
            {
                command.from = index0;
                command.to = index0 + numPoints - 1;

                futures += QtConcurrent::run( &qwtRenderDots,
                    xMap, yMap, command, pos, &image );
            }
        }
        for ( int i = 0; i < futures.size(); i++ )
            futures[i].waitForFinished();
#else
        command.from = from;
        command.to = to;

        qwtRenderDots( xMap, yMap, command, rect.topLeft(), &image );
#endif
    }
    else
    {
        // fallback implementation: to be replaced later by
        // setting the pixels of the image like above, TODO ...

        QPainter painter( &image );
        painter.setPen( pen );
        painter.setRenderHint( QPainter::Antialiasing, antialiased );

        const int chunkSize = 1000;
        for ( int i = from; i <= to; i += chunkSize )
        {
            const int indexTo = qMin( i + chunkSize - 1, to );
            const QPolygon points = toPoints(
                xMap, yMap, series, i, indexTo );

            painter.drawPoints( points );
        }
    }

    return image;
}
