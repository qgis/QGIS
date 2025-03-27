/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_tradingcurve.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>

static inline bool qwtIsSampleInside( const QwtOHLCSample& sample,
    double tMin, double tMax, double vMin, double vMax )
{
    const double t = sample.time;
    const QwtInterval interval = sample.boundingInterval();

    const bool isOffScreen = ( t < tMin ) || ( t > tMax )
        || ( interval.maxValue() < vMin ) || ( interval.minValue() > vMax );

    return !isOffScreen;
}

class QwtPlotTradingCurve::PrivateData
{
  public:
    PrivateData()
        : symbolStyle( QwtPlotTradingCurve::CandleStick )
        , symbolExtent( 0.6 )
        , minSymbolWidth( 2.0 )
        , maxSymbolWidth( -1.0 )
        , paintAttributes( QwtPlotTradingCurve::ClipSymbols )
    {
        symbolBrush[0] = QBrush( Qt::white );
        symbolBrush[1] = QBrush( Qt::black );
    }

    QwtPlotTradingCurve::SymbolStyle symbolStyle;
    double symbolExtent;
    double minSymbolWidth;
    double maxSymbolWidth;

    QPen symbolPen;
    QBrush symbolBrush[2]; // Increasing/Decreasing

    QwtPlotTradingCurve::PaintAttributes paintAttributes;
};

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotTradingCurve::QwtPlotTradingCurve( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotTradingCurve::QwtPlotTradingCurve( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotTradingCurve::~QwtPlotTradingCurve()
{
    delete m_data;
}

//! Initialize internal members
void QwtPlotTradingCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );

    m_data = new PrivateData;
    setData( new QwtTradingChartData() );

    setZ( 19.0 );
}

//! \return QwtPlotItem::Rtti_PlotTradingCurve
int QwtPlotTradingCurve::rtti() const
{
    return QwtPlotTradingCurve::Rtti_PlotTradingCurve;
}

/*!
   Specify an attribute how to draw the curve

   \param attribute Paint attribute
   \param on On/Off
   \sa testPaintAttribute()
 */
void QwtPlotTradingCurve::setPaintAttribute(
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
bool QwtPlotTradingCurve::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of samples

   \sa QwtPlotSeriesItem::setData()
 */
void QwtPlotTradingCurve::setSamples(
    const QVector< QwtOHLCSample >& samples )
{
    setData( new QwtTradingChartData( samples ) );
}

/*!
   Assign a series of samples

   setSamples() is just a wrapper for setData() without any additional
   value - beside that it is easier to find for the developer.

   \param data Data
   \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
 */
void QwtPlotTradingCurve::setSamples(
    QwtSeriesData< QwtOHLCSample >* data )
{
    setData( data );
}

/*!
   Set the symbol style

   \param style Symbol style

   \sa symbolStyle(), setSymbolExtent(),
      setSymbolPen(), setSymbolBrush()
 */
void QwtPlotTradingCurve::setSymbolStyle( SymbolStyle style )
{
    if ( style != m_data->symbolStyle )
    {
        m_data->symbolStyle = style;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Symbol style
   \sa setSymbolStyle(), symbolExtent(), symbolPen(), symbolBrush()
 */
QwtPlotTradingCurve::SymbolStyle QwtPlotTradingCurve::symbolStyle() const
{
    return m_data->symbolStyle;
}

/*!
   Build and assign the symbol pen

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotTradingCurve::setSymbolPen(
    const QColor& color, qreal width, Qt::PenStyle style )
{
    setSymbolPen( QPen( color, width, style ) );
}

/*!
   \brief Set the symbol pen

   The symbol pen is used for rendering the lines of the
   bar or candlestick symbols

   \sa symbolPen(), setSymbolBrush()
 */
void QwtPlotTradingCurve::setSymbolPen( const QPen& pen )
{
    if ( pen != m_data->symbolPen )
    {
        m_data->symbolPen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Symbol pen
   \sa setSymbolPen(), symbolBrush()
 */
QPen QwtPlotTradingCurve::symbolPen() const
{
    return m_data->symbolPen;
}

/*!
   Set the symbol brush

   \param direction Direction type
   \param brush Brush used to fill the body of all candlestick
               symbols with the direction

   \sa symbolBrush(), setSymbolPen()
 */
void QwtPlotTradingCurve::setSymbolBrush(
    Direction direction, const QBrush& brush )
{
    // silencing -Wtautological-constant-out-of-range-compare
    const int index = static_cast< int >( direction );
    if ( index < 0 || index >= 2 )
        return;

    if ( brush != m_data->symbolBrush[ index ] )
    {
        m_data->symbolBrush[ index ] = brush;

        legendChanged();
        itemChanged();
    }
}

/*!
   \param direction
   \return Brush used to fill the body of all candlestick
          symbols with the direction

   \sa setSymbolPen(), symbolBrush()
 */
QBrush QwtPlotTradingCurve::symbolBrush( Direction direction ) const
{
    const int index = static_cast< int >( direction );
    if ( index < 0 || index >= 2 )
        return QBrush();

    return m_data->symbolBrush[ index ];
}

/*!
   \brief Set the extent of the symbol

   The width of the symbol is given in scale coordinates. When painting
   a symbol the width is scaled into paint device coordinates
   by scaledSymbolWidth(). The scaled width is bounded by
   minSymbolWidth(), maxSymbolWidth()

   \param extent Symbol width in scale coordinates

   \sa symbolExtent(), scaledSymbolWidth(),
      setMinSymbolWidth(), setMaxSymbolWidth()
 */
void QwtPlotTradingCurve::setSymbolExtent( double extent )
{
    extent = qwtMaxF( 0.0, extent );
    if ( extent != m_data->symbolExtent )
    {
        m_data->symbolExtent = extent;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Extent of a symbol in scale coordinates
   \sa setSymbolExtent(), scaledSymbolWidth(),
      minSymbolWidth(), maxSymbolWidth()
 */
double QwtPlotTradingCurve::symbolExtent() const
{
    return m_data->symbolExtent;
}

/*!
   Set a minimum for the symbol width

   \param width Width in paint device coordinates
   \sa minSymbolWidth(), setMaxSymbolWidth(), setSymbolExtent()
 */
void QwtPlotTradingCurve::setMinSymbolWidth( double width )
{
    width = qwtMaxF( width, 0.0 );
    if ( width != m_data->minSymbolWidth )
    {
        m_data->minSymbolWidth = width;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Minmum for the symbol width
   \sa setMinSymbolWidth(), maxSymbolWidth(), symbolExtent()
 */
double QwtPlotTradingCurve::minSymbolWidth() const
{
    return m_data->minSymbolWidth;
}

/*!
   Set a maximum for the symbol width

   A value <= 0.0 means an unlimited width

   \param width Width in paint device coordinates
   \sa maxSymbolWidth(), setMinSymbolWidth(), setSymbolExtent()
 */
void QwtPlotTradingCurve::setMaxSymbolWidth( double width )
{
    if ( width != m_data->maxSymbolWidth )
    {
        m_data->maxSymbolWidth = width;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Maximum for the symbol width
   \sa setMaxSymbolWidth(), minSymbolWidth(), symbolExtent()
 */
double QwtPlotTradingCurve::maxSymbolWidth() const
{
    return m_data->maxSymbolWidth;
}

/*!
   \return Bounding rectangle of all samples.
   For an empty series the rectangle is invalid.
 */
QRectF QwtPlotTradingCurve::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem::boundingRect();
    if ( orientation() == Qt::Vertical )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/*!
   Draw an interval of the curve

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

   \sa drawSymbols()
 */
void QwtPlotTradingCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    painter->save();

    if ( m_data->symbolStyle != QwtPlotTradingCurve::NoSymbol )
        drawSymbols( painter, xMap, yMap, canvasRect, from, to );

    painter->restore();
}

/*!
   Draw symbols

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted

   \sa drawSeries()
 */
void QwtPlotTradingCurve::drawSymbols( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const QRectF tr = QwtScaleMap::invTransform( xMap, yMap, canvasRect );

    const QwtScaleMap* timeMap, * valueMap;
    double tMin, tMax, vMin, vMax;

    const Qt::Orientation orient = orientation();
    if ( orient == Qt::Vertical )
    {
        timeMap = &xMap;
        valueMap = &yMap;

        tMin = tr.left();
        tMax = tr.right();
        vMin = tr.top();
        vMax = tr.bottom();
    }
    else
    {
        timeMap = &yMap;
        valueMap = &xMap;

        vMin = tr.left();
        vMax = tr.right();
        tMin = tr.top();
        tMax = tr.bottom();
    }

    const bool inverted = timeMap->isInverting();
    const bool doClip = m_data->paintAttributes & ClipSymbols;
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double symbolWidth = scaledSymbolWidth( xMap, yMap, canvasRect );
    if ( doAlign )
        symbolWidth = std::floor( 0.5 * symbolWidth ) * 2.0;

    QPen pen = m_data->symbolPen;
    pen.setCapStyle( Qt::FlatCap );

    painter->setPen( pen );

    for ( int i = from; i <= to; i++ )
    {
        const QwtOHLCSample s = sample( i );

        if ( !doClip || qwtIsSampleInside( s, tMin, tMax, vMin, vMax ) )
        {
            QwtOHLCSample translatedSample;

            translatedSample.time = timeMap->transform( s.time );
            translatedSample.open = valueMap->transform( s.open );
            translatedSample.high = valueMap->transform( s.high );
            translatedSample.low = valueMap->transform( s.low );
            translatedSample.close = valueMap->transform( s.close );

            const int brushIndex = ( s.open < s.close )
                ? QwtPlotTradingCurve::Increasing
                : QwtPlotTradingCurve::Decreasing;

            if ( doAlign )
            {
                translatedSample.time = qRound( translatedSample.time );
                translatedSample.open = qRound( translatedSample.open );
                translatedSample.high = qRound( translatedSample.high );
                translatedSample.low = qRound( translatedSample.low );
                translatedSample.close = qRound( translatedSample.close );
            }

            switch( m_data->symbolStyle )
            {
                case Bar:
                {
                    drawBar( painter, translatedSample,
                        orient, inverted, symbolWidth );
                    break;
                }
                case CandleStick:
                {
                    painter->setBrush( m_data->symbolBrush[ brushIndex ] );
                    drawCandleStick( painter, translatedSample,
                        orient, symbolWidth );
                    break;
                }
                default:
                {
                    if ( m_data->symbolStyle >= UserSymbol )
                    {
                        painter->setBrush( m_data->symbolBrush[ brushIndex ] );
                        drawUserSymbol( painter, m_data->symbolStyle,
                            translatedSample, orient, inverted, symbolWidth );
                    }
                }
            }
        }
    }
}

/*!
   \brief Draw a symbol for a symbol style >= UserSymbol

   The implementation does nothing and is intended to be overloaded

   \param painter Qt painter, initialized with pen/brush
   \param symbolStyle Symbol style
   \param sample Samples already translated into paint device coordinates
   \param orientation Vertical or horizontal
   \param inverted True, when the opposite scale
                  ( Qt::Vertical: x, Qt::Horizontal: y ) is increasing
                  in the opposite direction as QPainter coordinates.
   \param symbolWidth Width of the symbol in paint device coordinates
 */
void QwtPlotTradingCurve::drawUserSymbol( QPainter* painter,
    SymbolStyle symbolStyle, const QwtOHLCSample& sample,
    Qt::Orientation orientation, bool inverted, double symbolWidth ) const
{
    Q_UNUSED( painter )
    Q_UNUSED( symbolStyle )
    Q_UNUSED( orientation )
    Q_UNUSED( inverted )
    Q_UNUSED( symbolWidth )
    Q_UNUSED( sample )
}

/*!
   \brief Draw a bar

   \param painter Qt painter, initialized with pen/brush
   \param sample Sample, already translated into paint device coordinates
   \param orientation Vertical or horizontal
   \param inverted When inverted is false the open tick is painted
                  to the left/top, otherwise it is painted right/bottom.
                  The close tick is painted in the opposite direction
                  of the open tick.
                  painted in the opposite d
                  opposite direction.
   \param width Width or height of the candle, depending on the orientation

   \sa Bar
 */
void QwtPlotTradingCurve::drawBar( QPainter* painter,
    const QwtOHLCSample& sample, Qt::Orientation orientation,
    bool inverted, double width ) const
{
    double w2 = 0.5 * width;
    if ( inverted )
        w2 *= -1;

    if ( orientation == Qt::Vertical )
    {
        QwtPainter::drawLine( painter,
            sample.time, sample.low, sample.time, sample.high );

        QwtPainter::drawLine( painter,
            sample.time - w2, sample.open, sample.time, sample.open );
        QwtPainter::drawLine( painter,
            sample.time + w2, sample.close, sample.time, sample.close );
    }
    else
    {
        QwtPainter::drawLine( painter, sample.low, sample.time,
            sample.high, sample.time );
        QwtPainter::drawLine( painter,
            sample.open, sample.time - w2, sample.open, sample.time );
        QwtPainter::drawLine( painter,
            sample.close, sample.time + w2, sample.close, sample.time );
    }
}

/*!
   \brief Draw a candle stick

   \param painter Qt painter, initialized with pen/brush
   \param sample Samples already translated into paint device coordinates
   \param orientation Vertical or horizontal
   \param width Width or height of the candle, depending on the orientation

   \sa CandleStick
 */
void QwtPlotTradingCurve::drawCandleStick( QPainter* painter,
    const QwtOHLCSample& sample, Qt::Orientation orientation,
    double width ) const
{
    const double t = sample.time;
    const double v1 = qwtMinF( sample.low, sample.high );
    const double v2 = qwtMinF( sample.open, sample.close );
    const double v3 = qwtMaxF( sample.low, sample.high );
    const double v4 = qwtMaxF( sample.open, sample.close );

    if ( orientation == Qt::Vertical )
    {
        QwtPainter::drawLine( painter, t, v1, t, v2 );
        QwtPainter::drawLine( painter, t, v3, t, v4 );

        QRectF rect( t - 0.5 * width, sample.open,
            width, sample.close - sample.open );

        QwtPainter::drawRect( painter, rect );
    }
    else
    {
        QwtPainter::drawLine( painter, v1, t, v2, t );
        QwtPainter::drawLine( painter, v3, t, v4, t );

        const QRectF rect( sample.open, t - 0.5 * width,
            sample.close - sample.open, width );

        QwtPainter::drawRect( painter, rect );
    }
}

/*!
   \return A rectangle filled with the color of the symbol pen

   \param index Index of the legend entry
                ( usually there is only one )
   \param size Icon size

   \sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPlotTradingCurve::legendIcon( int index,
    const QSizeF& size ) const
{
    Q_UNUSED( index );
    return defaultIcon( m_data->symbolPen.color(), size );
}

/*!
   Calculate the symbol width in paint coordinates

   The width is calculated by scaling the symbol extent into
   paint device coordinates bounded by the minimum/maximum
   symbol width.

   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas

   \return Symbol width in paint coordinates

   \sa symbolExtent(), minSymbolWidth(), maxSymbolWidth()
 */
double QwtPlotTradingCurve::scaledSymbolWidth(
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    Q_UNUSED( canvasRect );

    if ( m_data->maxSymbolWidth > 0.0 &&
        m_data->minSymbolWidth >= m_data->maxSymbolWidth )
    {
        return m_data->minSymbolWidth;
    }

    const QwtScaleMap* map =
        ( orientation() == Qt::Vertical ) ? &xMap : &yMap;

    const double pos = map->transform( map->s1() + m_data->symbolExtent );

    double width = qAbs( pos - map->p1() );

    width = qwtMaxF( width,  m_data->minSymbolWidth );
    if ( m_data->maxSymbolWidth > 0.0 )
        width = qwtMinF( width, m_data->maxSymbolWidth );

    return width;
}
