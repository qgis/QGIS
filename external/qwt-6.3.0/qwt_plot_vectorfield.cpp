/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_vectorfield.h"
#include "qwt_vectorfield_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qdebug.h>
#include <cstdlib>
#include <limits>

#define DEBUG_RENDER 0

#if DEBUG_RENDER
#include <qelapsedtimer.h>
#endif


static inline double qwtVector2Radians( double vx, double vy )
{
    if ( vx == 0.0 )
        return ( vy >= 0 ) ? M_PI_2 : 3 * M_PI_2;

    return std::atan2( vy, vx );
}

static inline double qwtVector2Magnitude( double vx, double vy )
{
    return sqrt( vx * vx + vy * vy );
}

static QwtInterval qwtMagnitudeRange(
    const QwtSeriesData< QwtVectorFieldSample >* series )
{
    if ( series->size() == 0 )
        return QwtInterval( 0, 1 );

    const QwtVectorFieldSample s0 = series->sample( 0 );

    double min = s0.vx * s0.vx + s0.vy * s0.vy;
    double max = min;

    for ( uint i = 1; i < series->size(); i++ )
    {
        const QwtVectorFieldSample s = series->sample( i );
        const double l = s.vx * s.vx + s.vy * s.vy;

        if ( l < min )
            min = l;

        if ( l > max )
            max = l;
    }

    min = std::sqrt( min );
    max = std::sqrt( max );

    if ( max == min )
        max += 1.0;

    return QwtInterval( min, max );
}

static inline QTransform qwtSymbolTransformation(
    const QTransform& oldTransform, double x, double y,
    double vx, double vy, double magnitude )
{
    QTransform transform = oldTransform;

    if ( !transform.isIdentity() )
    {
        transform.translate( x, y );

        const double radians = qwtVector2Radians( vx, vy );
        transform.rotateRadians( radians );
    }
    else
    {
        /*
            When starting with no transformation ( f.e on screen )
            the matrix can be found without having to use
            trigonometric functions
         */

        qreal sin, cos;
        if ( magnitude == 0.0 )
        {
            // something
            sin = 1.0;
            cos = 0.0;
        }
        else
        {
            sin = vy / magnitude;
            cos = vx / magnitude;
        }

        transform.setMatrix( cos, sin, 0.0, -sin, cos, 0.0, x, y, 1.0 );
    }

    return transform;
}

namespace
{
    class FilterMatrix
    {
      public:
        class Entry
        {
          public:
            inline void addSample( double sx, double sy,
                double svx, double svy )
            {
                x += sx;
                y += sy;

                vx += svx;
                vy += svy;

                count++;
            }

            quint32 count;

            // screen positions -> float is good enough
            float x;
            float y;
            float vx;
            float vy;
        };

        FilterMatrix( const QRectF& dataRect,
            const QRectF& canvasRect, const QSizeF& cellSize )
        {
            m_dx = cellSize.width();
            m_dy = cellSize.height();

            m_x0 = dataRect.x();
            if ( m_x0 < canvasRect.x() )
                m_x0 += int( ( canvasRect.x() - m_x0 ) / m_dx ) * m_dx;

            m_y0 = dataRect.y();
            if ( m_y0 < canvasRect.y() )
                m_y0 += int( ( canvasRect.y() - m_y0 ) / m_dy ) * m_dy;

            m_numColumns = canvasRect.width() / m_dx + 1;
            m_numRows = canvasRect.height() / m_dy + 1;

#if 1
            /*
                limit column and row count to a maximum of 1000000,
                so that memory usage is not an issue
             */
            if ( m_numColumns > 1000 )
            {
                m_dx = canvasRect.width() / 1000;
                m_numColumns = canvasRect.width() / m_dx + 1;
            }

            if ( m_numRows > 1000 )
            {
                m_dy = canvasRect.height() / 1000;
                m_numRows = canvasRect.height() / m_dx + 1;
            }
#endif

            m_x1 = m_x0 + m_numColumns * m_dx;
            m_y1 = m_y0 + m_numRows * m_dy;

            m_entries = ( Entry* )::calloc( m_numRows * m_numColumns, sizeof( Entry ) );
            if ( m_entries == NULL )
            {
                qWarning() << "QwtPlotVectorField: raster for filtering too fine - running out of memory";
            }
        }

        ~FilterMatrix()
        {
            if ( m_entries )
                std::free( m_entries );
        }

        inline int numColumns() const
        {
            return m_numColumns;
        }

        inline int numRows() const
        {
            return m_numRows;
        }

        inline void addSample( double x, double y,
            double u, double v )
        {
            if ( x >= m_x0 && x < m_x1
                && y >= m_y0 && y < m_y1 )
            {
                Entry& entry = m_entries[ indexOf( x, y ) ];
                entry.addSample( x, y, u, v );
            }
        }

        const FilterMatrix::Entry* entries() const
        {
            return m_entries;
        }

      private:
        inline int indexOf( qreal x, qreal y ) const
        {
            const int col = ( x - m_x0 ) / m_dx;
            const int row = ( y - m_y0 ) / m_dy;

            return row * m_numColumns + col;
        }

        qreal m_x0, m_x1, m_y0, m_y1, m_dx, m_dy;
        int m_numColumns;
        int m_numRows;

        Entry* m_entries;
    };
}

class QwtPlotVectorField::PrivateData
{
  public:
    PrivateData()
        : pen( Qt::black )
        , brush( Qt::black )
        , indicatorOrigin( QwtPlotVectorField::OriginHead )
        , magnitudeScaleFactor( 1.0 )
        , rasterSize( 20, 20 )
        , minArrowLength( 0.0 )
        , maxArrowLength( std::numeric_limits< short >::max() )
        , magnitudeModes( MagnitudeAsLength )
    {
        colorMap = NULL;
        symbol = new QwtVectorFieldThinArrow();
    }

    ~PrivateData()
    {
        delete colorMap;
        delete symbol;
    }

    QPen pen;
    QBrush brush;

    IndicatorOrigin indicatorOrigin;
    QwtVectorFieldSymbol* symbol;
    QwtColorMap* colorMap;

    /*
        Stores the range of magnitudes to be used for the color map.
        If invalid (min=max or negative values), the range is determined
        from the data samples themselves.
     */
    QwtInterval magnitudeRange;
    QwtInterval boundingMagnitudeRange;

    qreal magnitudeScaleFactor;
    QSizeF rasterSize;

    double minArrowLength;
    double maxArrowLength;

    PaintAttributes paintAttributes;
    MagnitudeModes magnitudeModes;
};

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotVectorField::QwtPlotVectorField( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotVectorField::QwtPlotVectorField( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotVectorField::~QwtPlotVectorField()
{
    delete m_data;
}

/*!
   \brief Initialize data members
 */
void QwtPlotVectorField::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    m_data = new PrivateData;
    setData( new QwtVectorFieldData() );

    setZ( 20.0 );
}

/*!
   Assign a pen

   \param pen New pen
   \sa pen(), brush()

   \note the pen is ignored in MagnitudeAsColor mode
 */
void QwtPlotVectorField::setPen( const QPen& pen )
{
    if ( m_data->pen != pen )
    {
        m_data->pen = pen;

        itemChanged();
        legendChanged();
    }
}

/*!
   \return Pen used to draw the lines
   \sa setPen(), brush()
 */
QPen QwtPlotVectorField::pen() const
{
    return m_data->pen;
}

/*!
   \brief Assign a brush.

   \param brush New brush
   \sa brush(), pen()

   \note the brush is ignored in MagnitudeAsColor mode
 */
void QwtPlotVectorField::setBrush( const QBrush& brush )
{
    if ( m_data->brush != brush )
    {
        m_data->brush = brush;

        itemChanged();
        legendChanged();
    }
}

/*!
   \return Brush used to fill the symbol
   \sa setBrush(), pen()
 */
QBrush QwtPlotVectorField::brush() const
{
    return m_data->brush;
}

/*!
   Set the origin for the symbols/arrows

   \param origin Origin
   \sa indicatorOrigin()
 */
void QwtPlotVectorField::setIndicatorOrigin( IndicatorOrigin origin )
{
    m_data->indicatorOrigin = origin;
    if ( m_data->indicatorOrigin != origin )
    {
        m_data->indicatorOrigin = origin;
        itemChanged();
    }
}

//! \return origin for the symbols/arrows
QwtPlotVectorField::IndicatorOrigin QwtPlotVectorField::indicatorOrigin() const
{
    return m_data->indicatorOrigin;
}

/*!
   \brief Set the magnitudeScaleFactor

   The length of the arrow in screen coordinate units is calculated by
   scaling the magnitude by the magnitudeScaleFactor.

   \param factor Scale factor

   \sa magnitudeScaleFactor(), arrowLength()
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
void QwtPlotVectorField::setMagnitudeScaleFactor( double factor )
{
    if ( factor != m_data->magnitudeScaleFactor )
    {
        m_data->magnitudeScaleFactor = factor;
        itemChanged();
    }
}

/*!
   \return Scale factor used to calculate the arrow length from the magnitude

   The length of the arrow in screen coordinate units is calculated by
   scaling the magnitude by the magnitudeScaleFactor.

   Default implementation simply scales the vector using the magnitudeScaleFactor
   property.  Re-implement this function to provide special handling for
   zero/non-zero magnitude arrows, or impose minimum/maximum arrow length limits.

   \return Length of arrow to be drawn in dependence of vector magnitude.
   \sa magnitudeScaleFactor
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
double QwtPlotVectorField::magnitudeScaleFactor() const
{
    return m_data->magnitudeScaleFactor;
}

/*!
   Set the raster size used for filtering samples

   \sa rasterSize(), QwtPlotVectorField::FilterVectors
 */
void QwtPlotVectorField::setRasterSize( const QSizeF& size )
{
    if ( size != m_data->rasterSize )
    {
        m_data->rasterSize = size;
        itemChanged();
    }
}

/*!
   \return raster size used for filtering samples
   \sa setRasterSize(), QwtPlotVectorField::FilterVectors
 */
QSizeF QwtPlotVectorField::rasterSize() const
{
    return m_data->rasterSize;
}

/*!
   Specify an attribute how to draw the curve

   \param attribute Paint attribute
   \param on On/Off
   \sa testPaintAttribute()
 */
void QwtPlotVectorField::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    PaintAttributes attributes = m_data->paintAttributes;

    if ( on )
        attributes |= attribute;
    else
        attributes &= ~attribute;

    if ( m_data->paintAttributes != attributes )
    {
        m_data->paintAttributes = attributes;
        itemChanged();
    }
}

/*!
    \return True, when attribute is enabled
    \sa PaintAttribute, setPaintAttribute()
 */
bool QwtPlotVectorField::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

//! \return QwtPlotItem::Rtti_PlotField
int QwtPlotVectorField::rtti() const
{
    return QwtPlotItem::Rtti_PlotVectorField;
}

/*!
   Sets a new arrow symbol (implementation of arrow drawing code).

   \param symbol Arrow symbol

   \sa symbol(), drawSymbol()
   \note Ownership is transferred to QwtPlotVectorField.
 */
void QwtPlotVectorField::setSymbol( QwtVectorFieldSymbol* symbol )
{
    if ( m_data->symbol == symbol )
        return;

    delete m_data->symbol;
    m_data->symbol = symbol;

    itemChanged();
    legendChanged();
}

/*!
   \return arrow symbol
   \sa setSymbol(), drawSymbol()
 */
const QwtVectorFieldSymbol* QwtPlotVectorField::symbol() const
{
    return m_data->symbol;
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of points
 */
void QwtPlotVectorField::setSamples( const QVector< QwtVectorFieldSample >& samples )
{
    setData( new QwtVectorFieldData( samples ) );
}

/*!
   Assign a series of samples

   setSamples() is just a wrapper for setData() without any additional
   value - beside that it is easier to find for the developer.

   \param data Data
   \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
 */
void QwtPlotVectorField::setSamples( QwtVectorFieldData* data )
{
    setData( data );
}

/*!
   Change the color map

   The color map is used to map the magnitude of a sample into
   a color using a known range for the magnitudes.

   \param colorMap Color Map

   \sa colorMap(), magnitudeRange()
 */
void QwtPlotVectorField::setColorMap( QwtColorMap* colorMap )
{
    if ( colorMap == NULL )
        return;

    if ( colorMap != m_data->colorMap )
    {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }

    legendChanged();
    itemChanged();
}

/*!
   \return Color Map used for mapping the intensity values to colors
   \sa setColorMap()
 */
const QwtColorMap* QwtPlotVectorField::colorMap() const
{
    return m_data->colorMap;
}

/*!
   Specify a mode how to represent the magnitude a n arrow/symbol

   \param mode Mode
   \param on On/Off
   \sa testMagnitudeMode()
 */
void QwtPlotVectorField::setMagnitudeMode( MagnitudeMode mode, bool on )
{
    if ( on == testMagnitudeMode( mode ) )
        return;

    if ( on )
        m_data->magnitudeModes |= mode;
    else
        m_data->magnitudeModes &= ~mode;

    itemChanged();
}

/*!
    \return True, when mode is enabled
    \sa MagnitudeMode, setMagnitudeMode()
 */
bool QwtPlotVectorField::testMagnitudeMode( MagnitudeMode mode ) const
{
    return m_data->magnitudeModes & mode;
}

/*!
   Sets the min/max magnitudes to be used for color map lookups.

   If invalid (min=max=0 or negative values), the range is determined from
   the current range of magnitudes in the vector samples.

   \sa magnitudeRange(), colorMap()
 */
void QwtPlotVectorField::setMagnitudeRange( const QwtInterval& magnitudeRange )
{
    if ( m_data->magnitudeRange != magnitudeRange )
    {
        m_data->magnitudeRange = magnitudeRange;
        itemChanged();
    }
}

/*!
    \return min/max magnitudes to be used for color map lookups
   \sa setMagnitudeRange(), colorMap()
 */
QwtInterval QwtPlotVectorField::magnitudeRange() const
{
    return m_data->magnitudeRange;
}

/*!
   Set a minimum for the arrow length of non zero vectors

   \param length Minimum for the arrow length in pixels

   \sa minArrowLength(), setMaxArrowLength(), arrowLength()
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
void QwtPlotVectorField::setMinArrowLength( double length )
{
    length = qMax( length, 0.0 );

    if ( m_data->minArrowLength != length )
    {
        m_data->minArrowLength = length;
        itemChanged();
    }
}

/*!
   \return minimum for the arrow length of non zero vectors

   \sa setMinArrowLength(), maxArrowLength(), arrowLength()
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
double QwtPlotVectorField::minArrowLength() const
{
    return m_data->minArrowLength;
}

/*!
   Set a maximum for the arrow length

   \param length Maximum for the arrow length in pixels

   \sa maxArrowLength(), setMinArrowLength(), arrowLength()
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
void QwtPlotVectorField::setMaxArrowLength( double length )
{
    length = qMax( length, 0.0 );

    if ( m_data->maxArrowLength != length )
    {
        m_data->maxArrowLength = length;
        itemChanged();
    }
}

/*!
   \return maximum for the arrow length

   \sa setMinArrowLength(), maxArrowLength(), arrowLength()
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
double QwtPlotVectorField::maxArrowLength() const
{
    return m_data->maxArrowLength;
}

/*!
   Computes length of the arrow in screen coordinate units based on its magnitude.

   Default implementation simply scales the vector using the magnitudeScaleFactor()
   If the result is not null, the length is then bounded into the interval
   [ minArrowLength(), maxArrowLength() ].

   Re-implement this function to provide special handling for
   zero/non-zero magnitude arrows, or impose minimum/maximum arrow length limits.

   \param magnitude Magnitude
   \return Length of arrow to be drawn in dependence of vector magnitude.

   \sa magnitudeScaleFactor, minArrowLength(), maxArrowLength()
   \note Has no effect when QwtPlotVectorField::MagnitudeAsLength is not enabled
 */
double QwtPlotVectorField::arrowLength( double magnitude ) const
{
#if 0
    /*
       Normalize magnitude with respect to value range.  Then, magnitudeScaleFactor
       is the number of pixels to draw for a vector of length equal to
       magnitudeRange.maxValue(). The relative scaling ensures that change of data
       samples of very different magnitudes will always lead to a reasonable
       display on screen.
     */
    const QwtVectorFieldData* vectorData = dynamic_cast< const QwtVectorFieldData* >( data() );
    if ( m_data->magnitudeRange.maxValue() > 0 )
        magnitude /= m_data->magnitudeRange.maxValue();
#endif

    double length = magnitude * m_data->magnitudeScaleFactor;

    if ( length > 0.0 )
        length = qBound( m_data->minArrowLength, length, m_data->maxArrowLength );

    return length;
}

QRectF QwtPlotVectorField::boundingRect() const
{
#if 0
    /*
        The bounding rectangle of the samples comes from the origins
        only, but as we know the scaling factor for the magnitude
        ( qwtVector2Magnitude ) here, we could try to include it ?
     */
#endif

    return QwtPlotSeriesItem::boundingRect();
}

/*!
   \return Icon representing the vector fields on the legend

   \param index Index of the legend entry ( ignored as there is only one )
   \param size Icon size

   \sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 */
QwtGraphic QwtPlotVectorField::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    QwtGraphic icon;
    icon.setDefaultSize( size );

    if ( size.isEmpty() )
        return icon;

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    painter.translate( -size.width(), -0.5 * size.height() );

    painter.setPen( m_data->pen );
    painter.setBrush( m_data->brush );

    m_data->symbol->setLength( size.width() - 2 );
    m_data->symbol->paint( &painter );

    return icon;
}

/*!
   Draw a subset of the points

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.
 */
void QwtPlotVectorField::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

#if DEBUG_RENDER
    QElapsedTimer timer;
    timer.start();
#endif

    drawSymbols( painter, xMap, yMap, canvasRect, from, to );

#if DEBUG_RENDER
    qDebug() << timer.elapsed();
#endif
}

/*!
   Draw symbols

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted

   \sa setSymbol(), drawSymbol(), drawSeries()
 */
void QwtPlotVectorField::drawSymbols( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );
    const bool doClip = false;

    const bool isInvertingX = xMap.isInverting();
    const bool isInvertingY = yMap.isInverting();

    const QwtSeriesData< QwtVectorFieldSample >* series = data();

    if ( m_data->magnitudeModes & MagnitudeAsColor )
    {
        // user input error, can't draw without color map
        // TODO: Discuss! Without colormap, silently fall back to uniform colors?
        if ( m_data->colorMap == NULL)
            return;
    }
    else
    {
        painter->setPen( m_data->pen );
        painter->setBrush( m_data->brush );
    }

    if ( ( m_data->paintAttributes & FilterVectors ) && !m_data->rasterSize.isEmpty() )
    {
        const QRectF dataRect = QwtScaleMap::transform(
            xMap, yMap, boundingRect() );

        // TODO: Discuss. How to handle raster size when switching from screen to print size!
        //       DPI-aware adjustment of rastersize? Or make "rastersize in screen coordinate"
        //       or "rastersize in plotcoordinetes" a user option?
#if 1
        // define filter matrix based on screen/print coordinates
        FilterMatrix matrix( dataRect, canvasRect, m_data->rasterSize );
#else
        // define filter matrix based on real coordinates

        // get scale factor from real coordinates to screen coordinates
        double xScale = 1;
        if (xMap.sDist() != 0)
            xScale = xMap.pDist() / xMap.sDist();

        double yScale = 1;
        if (yMap.sDist() != 0)
            yScale = yMap.pDist() / yMap.sDist();

        QSizeF canvasRasterSize(xScale * m_data->rasterSize.width(), yScale * m_data->rasterSize.height() );
        FilterMatrix matrix( dataRect, canvasRect, canvasRasterSize );
#endif

        for ( int i = from; i <= to; i++ )
        {
            const QwtVectorFieldSample sample = series->sample( i );
            if ( !sample.isNull() )
            {
                matrix.addSample( xMap.transform( sample.x ),
                    yMap.transform( sample.y ), sample.vx, sample.vy );
            }
        }

        const int numEntries = matrix.numRows() * matrix.numColumns();
        const FilterMatrix::Entry* entries = matrix.entries();

        for ( int i = 0; i < numEntries; i++ )
        {
            const FilterMatrix::Entry& entry = entries[i];

            if ( entry.count == 0 )
                continue;

            double xi = entry.x / entry.count;
            double yi = entry.y / entry.count;

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            const double vx = entry.vx / entry.count;
            const double vy = entry.vy / entry.count;

            drawSymbol( painter, xi, yi,
                isInvertingX ? -vx : vx, isInvertingY ? -vy : vy );
        }
    }
    else
    {
        for ( int i = from; i <= to; i++ )
        {
            const QwtVectorFieldSample sample = series->sample( i );

            // arrows with zero length are never drawn
            if ( sample.isNull() )
                continue;

            double xi = xMap.transform( sample.x );
            double yi = yMap.transform( sample.y );

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            if ( doClip )
            {
                if ( !canvasRect.contains( xi, yi ) )
                    continue;
            }

            drawSymbol( painter, xi, yi,
                isInvertingX ? -sample.vx : sample.vx,
                isInvertingY ? -sample.vy : sample.vy );
        }
    }
}

/*!
   Draw a arrow/symbols at a specific position

   x, y, are paint device coordinates, while vx, vy are from
   the corresponding sample.

   \sa setSymbol(), drawSeries()
 */
void QwtPlotVectorField::drawSymbol( QPainter* painter,
    double x, double y, double vx, double vy ) const
{
    const double magnitude = qwtVector2Magnitude( vx, vy );

    const QTransform oldTransform = painter->transform();

    QTransform transform = qwtSymbolTransformation( oldTransform,
        x, y, vx, vy, magnitude );

    QwtVectorFieldSymbol* symbol = m_data->symbol;

    double length = 0.0;

    if ( m_data->magnitudeModes & MagnitudeAsLength )
    {
        length = arrowLength( magnitude );
    }

    symbol->setLength( length );

    if( m_data->indicatorOrigin == OriginTail )
    {
        const qreal dx = symbol->length();
        transform.translate( dx, 0.0 );
    }
    else if ( m_data->indicatorOrigin == OriginCenter )
    {
        const qreal dx = symbol->length();
        transform.translate( 0.5 * dx, 0.0 );
    }

    if ( m_data->magnitudeModes & MagnitudeAsColor )
    {
        // Determine color for arrow if colored by magnitude.

        QwtInterval range = m_data->magnitudeRange;

        if ( !range.isValid() )
        {
            if ( !m_data->boundingMagnitudeRange.isValid() )
                m_data->boundingMagnitudeRange = qwtMagnitudeRange( data() );

            range = m_data->boundingMagnitudeRange;
        }

        const QColor c = m_data->colorMap->rgb( range, magnitude );

#if 1
        painter->setBrush( c );
        painter->setPen( c );
#endif
    }

    painter->setWorldTransform( transform, false );
    symbol->paint( painter );
    painter->setWorldTransform( oldTransform, false );
}

void QwtPlotVectorField::dataChanged()
{
    m_data->boundingMagnitudeRange.invalidate();
    QwtPlotSeriesItem::dataChanged();
}
