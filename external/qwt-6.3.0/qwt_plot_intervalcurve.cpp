/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_intervalcurve.h"
#include "qwt_interval_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include "qwt_graphic.h"
#include "qwt_text.h"

#include <qpainter.h>
#include <qmath.h>
#include <cstring>

static inline bool qwtIsHSampleInside( const QwtIntervalSample& sample,
    double xMin, double xMax, double yMin, double yMax )
{
    const double y = sample.value;
    const double x1 = sample.interval.minValue();
    const double x2 = sample.interval.maxValue();

    const bool isOffScreen = ( y < yMin ) || ( y > yMax )
        || ( x1 < xMin && x2 < xMin ) || ( x1 > xMax && x2 > xMax );

    return !isOffScreen;
}

static inline bool qwtIsVSampleInside( const QwtIntervalSample& sample,
    double xMin, double xMax, double yMin, double yMax )
{
    const double x = sample.value;
    const double y1 = sample.interval.minValue();
    const double y2 = sample.interval.maxValue();

    const bool isOffScreen = ( x < xMin ) || ( x > xMax )
        || ( y1 < yMin && y2 < yMin ) || ( y1 > yMax && y2 > yMax );

    return !isOffScreen;
}

static inline int qwtInterpolated( int from, int to, double ratio )
{
    return qRound( from + ratio * ( to - from ) );
}

static int qwtStartIndex( int from, double min, double max,
    const QwtSeriesData< QwtIntervalSample >* samples )
{
    /*
        As we know, that the value coordinates are montonically
        increasing/decreasing we can find the first sample before
        entering the interval using a binary search algo.
     */

    struct lessThan
    {
        inline bool operator()( const double value,
            const QwtIntervalSample& sample ) const
        {
            return value < sample.value;
        }
    };

    struct greaterThan
    {
        inline bool operator()( const double value,
            const QwtIntervalSample& sample ) const
        {
            return value > sample.value;
        }
    };

    int idx;

    if ( samples->firstSample().value < samples->lastSample().value )
    {
        idx = qwtUpperSampleIndex< QwtIntervalSample >(
            *samples, min, lessThan() );
    }
    else
    {
        idx = qwtUpperSampleIndex< QwtIntervalSample >(
            *samples, max, greaterThan() );
    }

    if ( idx >= 0 )
    {
        idx = qMax( from, idx );
        if ( idx > 0 )
        {
            // we need to fill the area before the first sample
            idx--;
        }
    }

    return idx;
}

namespace
{
    class LinesRenderer
    {
      public:
        LinesRenderer( Qt::Orientation,
            const QwtScaleMap&, const QwtScaleMap&, const QRectF& );

        void addSamples( const QwtSeriesData< QwtIntervalSample >*, int from, int to );

        QVector< QLine > fillLines;
        QVector< QLine > borderLines;

      private:
        bool addSample( const QwtIntervalSample& );
        void flush();

        bool append( int value, int min, int max );

        void addFillLineAt( int value, int min, int max );
        void addFillLines( int value, int min, int max );

        void addBorderLineAt( int value, int min, int max );
        void addBorderLine( int, int, int, int );

        const bool m_vertical;

        const QwtScaleMap& m_valueMap;
        const QwtScaleMap& m_intervalMap;

        const bool m_inverting;
        bool m_pending;

        // all ints are in paint device coordinates

        int m_intvMin, m_intvMax;   // interval boundaries of the canvas
        int m_valueMin, m_valueMax; // value boundaries of the canvas

        int m_value; // value of the last sample being processed

        struct
        {
            inline void reset( int value )
            {
                min = max = last = value;
            }

            inline void extend( int value )
            {
                if ( value < min )
                    min = value;
                else if ( value > max )
                    max = value;

                last = value;
            }

            int min, max, last;

        } m_lower, m_upper;
    };

    LinesRenderer::LinesRenderer( Qt::Orientation orientation,
            const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect )
        : m_vertical( orientation == Qt::Vertical )
        , m_valueMap( m_vertical ? xMap : yMap )
        , m_intervalMap( m_vertical ? yMap : xMap )
        , m_inverting( m_intervalMap.isInverting() )
        , m_pending( false )
        , m_value( 0 )
    {
        QRectF r = rect; 
        if ( !m_vertical )
            r.setSize( QSizeF( r.height(), r.width() ) );

        m_intvMin = qFloor( r.top() );
        m_intvMax = qCeil( r.bottom() );

        m_valueMin = qFloor( r.left() );
        m_valueMax = qCeil( r.right() );

        const int steps = m_valueMax - m_valueMin + 1;

        // one line for each pixel
        fillLines.reserve( steps );

        /*
            for both border lines:
                one line ( min/max ) of the samples lying on the same pixel
                one line connecting the pixels
         */
        borderLines.reserve( 4 * steps );
    }

    void LinesRenderer::addSamples(
        const QwtSeriesData< QwtIntervalSample >* samples, int from, int to )
    {
        const double v1 = m_valueMap.invTransform( m_valueMin );
        const double v2 = m_valueMap.invTransform( m_valueMax );

        from = qwtStartIndex( from, qMin( v1, v2 ), qMax( v1, v2 ), samples );

        if ( from >= 0 )
        {
            for ( int i = from; i <= to; i++ )
            {
                if ( addSample( samples->sample( i ) ) )
                    break;
            }
        }

        flush();
    }

    inline bool LinesRenderer::addSample( const QwtIntervalSample& sample )
    {
        const double value = qRound( m_valueMap.transform( sample.value ) );

        const int min = qRound( m_intervalMap.transform( sample.interval.minValue() ) );
        const int max = qRound( m_intervalMap.transform( sample.interval.maxValue() ) );

        return m_inverting ? append( value, max, min ) : append( value, max, min );
    }

    inline bool LinesRenderer::append(
        const int value, const int lower, const int upper )
    {
        if ( !m_pending )
        {
            m_value = value;
            m_lower.reset( lower );
            m_upper.reset( upper );

            m_pending = true;

            return false;
        }

        if ( value != m_value )
        {
            addFillLines( value, lower, upper );
            addBorderLineAt( m_value, m_lower.min, m_lower.max );
            addBorderLineAt( m_value, m_upper.min, m_upper.max );

            addBorderLine( m_value, m_lower.last, value, lower );
            addBorderLine( m_value, m_upper.last, value, upper );

            if ( ( value > m_value ) ? ( value > m_valueMax ) : ( value < m_valueMin ) )
            {
                m_pending = false;
                return true;
            }

            m_value = value;

            m_lower.reset( lower );
            m_upper.reset( upper );
        }
        else
        {
            m_lower.extend( lower );
            m_upper.extend( upper );
        }

        return false;
    }

    inline void LinesRenderer::flush()
    {
        if ( m_pending )
        {
            addFillLineAt( m_value, m_lower.min, m_upper.max );
            addBorderLineAt( m_value, m_lower.min, m_lower.max );
            addBorderLineAt( m_value, m_upper.min, m_upper.max );
        }
    }

    inline void LinesRenderer::addFillLines(
        const int value, const int lower, const int upper )
    {
        addFillLineAt( m_value, m_lower.min, m_upper.max );

        const double delta = value - m_value;

        if ( value > m_value )
        {
            for ( int v = m_value + 1; v < value; v++ )
            {
                const double ratio = ( v - m_value ) / delta;

                addFillLineAt( v,
                    qwtInterpolated( m_lower.last, lower, ratio ),
                    qwtInterpolated( m_upper.last, upper, ratio ) );
            }
        }
        else
        {
            for ( int v = m_value - 1; v > value; v-- )
            {
                const double ratio = ( v - m_value ) / delta;

                addFillLineAt( v,
                    qwtInterpolated( m_lower.last, lower, ratio ),
                    qwtInterpolated( m_upper.last, upper, ratio ) );
            }
        }
    }

    inline void LinesRenderer::addFillLineAt( int value, int min, int max )
    {
        if ( ( min != max ) && ( max > m_intvMin ) && ( min < m_intvMax ) )
        {
            min = qMax( min, m_intvMin );
            max = qMin( max, m_intvMax );

            if( m_vertical )
                fillLines += QLine( value, min, value, max );
            else
                fillLines += QLine( min, value, max, value );
        }
    }

    inline void LinesRenderer::addBorderLineAt( int value, int min, int max )
    {
        if ( min != max )
            addBorderLine( value, min, value, max );
    }

    inline void LinesRenderer::addBorderLine( int x1, int y1, int x2, int y2 )
    {
        if( m_vertical )
            borderLines += QLine( x1, y1, x2, y2 );
        else
            borderLines += QLine( y1, x1, y2, x2 );
    }
}

static void qwtDrawTubeLines(
    const QwtPlotIntervalCurve* curve, QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to)
{
    LinesRenderer renderer( curve->orientation(), xMap, yMap, canvasRect );
    renderer.addSamples( curve->data(), from, to );

    if ( curve->brush().style() != Qt::NoBrush )
    {
        painter->save();

        painter->setPen( curve->brush().color() );
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->drawLines( renderer.fillLines );

        painter->restore();
    }

    if ( curve->pen().style() != Qt::NoPen )
    {
        painter->save();

        painter->setPen( curve->pen() );
        painter->drawLines( renderer.borderLines );

        painter->restore();
    }
}

static void qwtDrawTube(
    const QwtPlotIntervalCurve* curve, QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to )
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    painter->save();

    const size_t size = to - from + 1;
    QPolygonF polygon( 2 * size );
    QPointF* points = polygon.data();

    for ( uint i = 0; i < size; i++ )
    {
        QPointF& minValue = points[i];
        QPointF& maxValue = points[2 * size - 1 - i];

        const QwtIntervalSample intervalSample = curve->sample( from + i );
        if ( curve->orientation() == Qt::Vertical )
        {
            double x = xMap.transform( intervalSample.value );
            double y1 = yMap.transform( intervalSample.interval.minValue() );
            double y2 = yMap.transform( intervalSample.interval.maxValue() );

            if ( doAlign )
            {
                x = qRound( x );
                y1 = qRound( y1 );
                y2 = qRound( y2 );
            }

            minValue.rx() = x;
            minValue.ry() = y1;
            maxValue.rx() = x;
            maxValue.ry() = y2;
        }
        else
        {
            double y = yMap.transform( intervalSample.value );
            double x1 = xMap.transform( intervalSample.interval.minValue() );
            double x2 = xMap.transform( intervalSample.interval.maxValue() );

            if ( doAlign )
            {
                y = qRound( y );
                x1 = qRound( x1 );
                x2 = qRound( x2 );
            }

            minValue.rx() = x1;
            minValue.ry() = y;
            maxValue.rx() = x2;
            maxValue.ry() = y;
        }
    }

    const bool doClip = curve->testPaintAttribute( QwtPlotIntervalCurve::ClipPolygons );

    if ( curve->brush().style() != Qt::NoBrush )
    {
        painter->setPen( QPen( Qt::NoPen ) );
        painter->setBrush( curve->brush() );

        if ( doClip )
        {
            const qreal m = 1.0;
            const QPolygonF p = QwtClipper::clippedPolygonF(
                canvasRect.adjusted( -m, -m, m, m ), polygon, true );

            QwtPainter::drawPolygon( painter, p );
        }
        else
        {
            QwtPainter::drawPolygon( painter, polygon );
        }
    }

    if ( curve->pen().style() != Qt::NoPen )
    {
        painter->setPen( curve->pen() );
        painter->setBrush( Qt::NoBrush );

        if ( doClip )
        {
            qreal pw = QwtPainter::effectivePenWidth( painter->pen() );
            const QRectF clipRect = canvasRect.adjusted( -pw, -pw, pw, pw );

            QPolygonF p( size );

            std::memcpy( p.data(), points, size * sizeof( QPointF ) );
            QwtPainter::drawPolyline( painter,
                QwtClipper::clippedPolygonF( clipRect, p ) );

            std::memcpy( p.data(), points + size, size * sizeof( QPointF ) );
            QwtPainter::drawPolyline( painter,
                QwtClipper::clippedPolygonF( clipRect, p ) );
        }
        else
        {
            QwtPainter::drawPolyline( painter, points, size );
            QwtPainter::drawPolyline( painter, points + size, size );
        }
    }

    painter->restore();
}

class QwtPlotIntervalCurve::PrivateData
{
  public:
    PrivateData():
        style( QwtPlotIntervalCurve::Tube ),
        symbol( NULL ),
        pen( Qt::black ),
        brush( Qt::white )
    {
        paintAttributes = QwtPlotIntervalCurve::ClipPolygons;
        paintAttributes |= QwtPlotIntervalCurve::ClipSymbol;

        pen.setCapStyle( Qt::FlatCap );
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtPlotIntervalCurve::CurveStyle style;
    const QwtIntervalSymbol* symbol;

    QPen pen;
    QBrush brush;

    QwtPlotIntervalCurve::PaintAttributes paintAttributes;
};

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotIntervalCurve::QwtPlotIntervalCurve( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotIntervalCurve::QwtPlotIntervalCurve( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotIntervalCurve::~QwtPlotIntervalCurve()
{
    delete m_data;
}

//! Initialize internal members
void QwtPlotIntervalCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );

    m_data = new PrivateData;
    setData( new QwtIntervalSeriesData() );

    setZ( 19.0 );
}

//! \return QwtPlotItem::Rtti_PlotIntervalCurve
int QwtPlotIntervalCurve::rtti() const
{
    return QwtPlotIntervalCurve::Rtti_PlotIntervalCurve;
}

/*!
   Specify an attribute how to draw the curve

   \param attribute Paint attribute
   \param on On/Off
   \sa testPaintAttribute()
 */
void QwtPlotIntervalCurve::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/*!
    \return True, when attribute is enabled
    \sa PaintAttribute, setPaintAttribute()
 */
bool QwtPlotIntervalCurve::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of samples
 */
void QwtPlotIntervalCurve::setSamples(
    const QVector< QwtIntervalSample >& samples )
{
    setData( new QwtIntervalSeriesData( samples ) );
}

/*!
   Assign a series of samples

   setSamples() is just a wrapper for setData() without any additional
   value - beside that it is easier to find for the developer.

   \param data Data
   \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
 */
void QwtPlotIntervalCurve::setSamples(
    QwtSeriesData< QwtIntervalSample >* data )
{
    setData( data );
}

/*!
   Set the curve's drawing style

   \param style Curve style
   \sa CurveStyle, style()
 */
void QwtPlotIntervalCurve::setStyle( CurveStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/*!
    \return Style of the curve
    \sa setStyle()
 */
QwtPlotIntervalCurve::CurveStyle QwtPlotIntervalCurve::style() const
{
    return m_data->style;
}

/*!
   Assign a symbol.

   \param symbol Symbol
   \sa symbol()
 */
void QwtPlotIntervalCurve::setSymbol( const QwtIntervalSymbol* symbol )
{
    if ( symbol != m_data->symbol )
    {
        delete m_data->symbol;
        m_data->symbol = symbol;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Current symbol or NULL, when no symbol has been assigned
   \sa setSymbol()
 */
const QwtIntervalSymbol* QwtPlotIntervalCurve::symbol() const
{
    return m_data->symbol;
}

/*!
   Build and assign a pen

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotIntervalCurve::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   \brief Assign a pen
   \param pen New pen
   \sa pen(), brush()
 */
void QwtPlotIntervalCurve::setPen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
    \return Pen used to draw the lines
    \sa setPen(), brush()
 */
const QPen& QwtPlotIntervalCurve::pen() const
{
    return m_data->pen;
}

/*!
   Assign a brush.

   The brush is used to fill the area in Tube style().

   \param brush Brush
   \sa brush(), pen(), setStyle(), CurveStyle
 */
void QwtPlotIntervalCurve::setBrush( const QBrush& brush )
{
    if ( brush != m_data->brush )
    {
        m_data->brush = brush;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Brush used to fill the area in Tube style()
   \sa setBrush(), setStyle(), CurveStyle
 */
const QBrush& QwtPlotIntervalCurve::brush() const
{
    return m_data->brush;
}

/*!
   \return Bounding rectangle of all samples.
   For an empty series the rectangle is invalid.
 */
QRectF QwtPlotIntervalCurve::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if ( orientation() == Qt::Vertical )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/*!
   Draw a subset of the samples

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

   \sa drawTube(), drawSymbols()
 */
void QwtPlotIntervalCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    if ( m_data->style == Tube )
        drawTube( painter, xMap, yMap, canvasRect, from, to );

    if ( m_data->symbol &&
        ( m_data->symbol->style() != QwtIntervalSymbol::NoSymbol ) )
    {
        drawSymbols( painter, *m_data->symbol,
            xMap, yMap, canvasRect, from, to );
    }
}

/*!
   Draw a tube

   Builds 2 curves from the upper and lower limits of the intervals
   and draws them with the pen(). The area between the curves is
   filled with the brush().

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

   \sa drawSeries(), drawSymbols()
 */
void QwtPlotIntervalCurve::drawTube(
    QPainter* painter, const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const
{
    if ( ( m_data->pen.style() == Qt::NoPen ) &&
        ( m_data->brush.style() == Qt::NoBrush ) )
    {
        return;
    }

    const bool useLines = testPaintAttribute( TubeAsLines )
        && QwtPainter::roundingAlignment( painter );

    if ( useLines )
        qwtDrawTubeLines( this, painter, xMap, yMap, canvasRect, from, to );
    else
        qwtDrawTube( this, painter, xMap, yMap, canvasRect, from, to );
}

/*!
   Draw symbols for a subset of the samples

   \param painter Painter
   \param symbol Interval symbol
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted

   \sa setSymbol(), drawSeries(), drawTube()
 */
void QwtPlotIntervalCurve::drawSymbols(
    QPainter* painter, const QwtIntervalSymbol& symbol,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    painter->save();

    QPen pen = symbol.pen();
    pen.setCapStyle( Qt::FlatCap );

    painter->setPen( pen );
    painter->setBrush( symbol.brush() );

    const QRectF tr = QwtScaleMap::invTransform( xMap, yMap, canvasRect );

    const double xMin = tr.left();
    const double xMax = tr.right();
    const double yMin = tr.top();
    const double yMax = tr.bottom();

    const bool doClip = m_data->paintAttributes & ClipSymbol;

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample s = sample( i );

        if ( orientation() == Qt::Vertical )
        {
            if ( !doClip || qwtIsVSampleInside( s, xMin, xMax, yMin, yMax ) )
            {
                const double x = xMap.transform( s.value );
                const double y1 = yMap.transform( s.interval.minValue() );
                const double y2 = yMap.transform( s.interval.maxValue() );

                symbol.draw( painter, orientation(),
                    QPointF( x, y1 ), QPointF( x, y2 ) );
            }
        }
        else
        {
            if ( !doClip || qwtIsHSampleInside( s, xMin, xMax, yMin, yMax ) )
            {
                const double y = yMap.transform( s.value );
                const double x1 = xMap.transform( s.interval.minValue() );
                const double x2 = xMap.transform( s.interval.maxValue() );

                symbol.draw( painter, orientation(),
                    QPointF( x1, y ), QPointF( x2, y ) );
            }
        }
    }

    painter->restore();
}

/*!
   \return Icon for the legend

   In case of Tube style() the icon is a plain rectangle filled with the brush().
   If a symbol is assigned it is scaled to size.

   \param index Index of the legend entry
               ( ignored as there is only one )
   \param size Icon size

   \sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 */
QwtGraphic QwtPlotIntervalCurve::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    if ( size.isEmpty() )
        return QwtGraphic();

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    if ( m_data->style == Tube )
    {
        QRectF r( 0, 0, size.width(), size.height() );
        painter.fillRect( r, m_data->brush );
    }

    if ( m_data->symbol &&
        ( m_data->symbol->style() != QwtIntervalSymbol::NoSymbol ) )
    {
        QPen pen = m_data->symbol->pen();
        pen.setWidthF( pen.widthF() );
        pen.setCapStyle( Qt::FlatCap );

        painter.setPen( pen );
        painter.setBrush( m_data->symbol->brush() );

        if ( orientation() == Qt::Vertical )
        {
            const double x = 0.5 * size.width();

            m_data->symbol->draw( &painter, orientation(),
                QPointF( x, 0 ), QPointF( x, size.height() - 1.0 ) );
        }
        else
        {
            const double y = 0.5 * size.height();

            m_data->symbol->draw( &painter, orientation(),
                QPointF( 0.0, y ), QPointF( size.width() - 1.0, y ) );
        }
    }

    return icon;
}
