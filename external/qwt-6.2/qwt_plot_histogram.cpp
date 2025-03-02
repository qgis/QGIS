/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_histogram.h"
#include "qwt_painter.h"
#include "qwt_column_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_graphic.h"

#include <qstring.h>
#include <qpainter.h>

static inline bool qwtIsCombinable( const QwtInterval& d1,
    const QwtInterval& d2 )
{
    if ( d1.isValid() && d2.isValid() )
    {
        if ( d1.maxValue() == d2.minValue() )
        {
            if ( !( d1.borderFlags() & QwtInterval::ExcludeMaximum
                && d2.borderFlags() & QwtInterval::ExcludeMinimum ) )
            {
                return true;
            }
        }
    }

    return false;
}

class QwtPlotHistogram::PrivateData
{
  public:
    PrivateData()
        : baseline( 0.0 )
        , style( Columns )
        , symbol( NULL )
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    double baseline;

    QPen pen;
    QBrush brush;
    QwtPlotHistogram::HistogramStyle style;
    const QwtColumnSymbol* symbol;
};

/*!
   Constructor
   \param title Title of the histogram.
 */
QwtPlotHistogram::QwtPlotHistogram( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the histogram.
 */
QwtPlotHistogram::QwtPlotHistogram( const QString& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

//! Destructor
QwtPlotHistogram::~QwtPlotHistogram()
{
    delete m_data;
}

//! Initialize data members
void QwtPlotHistogram::init()
{
    m_data = new PrivateData();
    setData( new QwtIntervalSeriesData() );

    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Legend, true );

    setZ( 20.0 );
}

/*!
   Set the histogram's drawing style

   \param style Histogram style
   \sa HistogramStyle, style()
 */
void QwtPlotHistogram::setStyle( HistogramStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/*!
    \return Style of the histogram
    \sa HistogramStyle, setStyle()
 */
QwtPlotHistogram::HistogramStyle QwtPlotHistogram::style() const
{
    return m_data->style;
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
void QwtPlotHistogram::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   Assign a pen, that is used in a style() depending way.

   \param pen New pen
   \sa pen(), brush()
 */
void QwtPlotHistogram::setPen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Pen used in a style() depending way.
   \sa setPen(), brush()
 */
const QPen& QwtPlotHistogram::pen() const
{
    return m_data->pen;
}

/*!
   Assign a brush, that is used in a style() depending way.

   \param brush New brush
   \sa pen(), brush()
 */
void QwtPlotHistogram::setBrush( const QBrush& brush )
{
    if ( brush != m_data->brush )
    {
        m_data->brush = brush;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Brush used in a style() depending way.
   \sa setPen(), brush()
 */
const QBrush& QwtPlotHistogram::brush() const
{
    return m_data->brush;
}

/*!
   \brief Assign a symbol

   In Column style an optional symbol can be assigned, that is responsible
   for displaying the rectangle that is defined by the interval and
   the distance between baseline() and value. When no symbol has been
   defined the area is displayed as plain rectangle using pen() and brush().

   \sa style(), symbol(), drawColumn(), pen(), brush()

   \note In applications, where different intervals need to be displayed
        in a different way ( f.e different colors or even using different symbols)
        it is recommended to overload drawColumn().
 */
void QwtPlotHistogram::setSymbol( const QwtColumnSymbol* symbol )
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
const QwtColumnSymbol* QwtPlotHistogram::symbol() const
{
    return m_data->symbol;
}

/*!
   \brief Set the value of the baseline

   Each column representing an QwtIntervalSample is defined by its
   interval and the interval between baseline and the value of the sample.

   The default value of the baseline is 0.0.

   \param value Value of the baseline
   \sa baseline()
 */
void QwtPlotHistogram::setBaseline( double value )
{
    if ( m_data->baseline != value )
    {
        m_data->baseline = value;
        itemChanged();
    }
}

/*!
   \return Value of the baseline
   \sa setBaseline()
 */
double QwtPlotHistogram::baseline() const
{
    return m_data->baseline;
}

/*!
   \return Bounding rectangle of all samples.
   For an empty series the rectangle is invalid.
 */
QRectF QwtPlotHistogram::boundingRect() const
{
    QRectF rect = data()->boundingRect();
    if ( !rect.isValid() )
        return rect;

    if ( orientation() == Qt::Horizontal )
    {
        rect = QRectF( rect.y(), rect.x(),
            rect.height(), rect.width() );

        if ( rect.left() > m_data->baseline )
            rect.setLeft( m_data->baseline );
        else if ( rect.right() < m_data->baseline )
            rect.setRight( m_data->baseline );
    }
    else
    {
        if ( rect.bottom() < m_data->baseline )
            rect.setBottom( m_data->baseline );
        else if ( rect.top() > m_data->baseline )
            rect.setTop( m_data->baseline );
    }

    return rect;
}

//! \return QwtPlotItem::Rtti_PlotHistogram
int QwtPlotHistogram::rtti() const
{
    return QwtPlotItem::Rtti_PlotHistogram;
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of points
 */
void QwtPlotHistogram::setSamples(
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
void QwtPlotHistogram::setSamples(
    QwtSeriesData< QwtIntervalSample >* data )
{
    setData( data );
}

/*!
   Draw a subset of the histogram samples

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

   \sa drawOutline(), drawLines(), drawColumns
 */
void QwtPlotHistogram::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    Q_UNUSED( canvasRect )

    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    switch ( m_data->style )
    {
        case Outline:
            drawOutline( painter, xMap, yMap, from, to );
            break;
        case Lines:
            drawLines( painter, xMap, yMap, from, to );
            break;
        case Columns:
            drawColumns( painter, xMap, yMap, from, to );
            break;
        default:
            break;
    }
}

/*!
   Draw a histogram in Outline style()

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         histogram will be painted to its last point.

   \sa setStyle(), style()
   \warning The outline style requires, that the intervals are in increasing
           order and not overlapping.
 */
void QwtPlotHistogram::drawOutline( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double v0 = ( orientation() == Qt::Horizontal ) ?
        xMap.transform( baseline() ) : yMap.transform( baseline() );
    if ( doAlign )
        v0 = qRound( v0 );

    QwtIntervalSample previous;

    QPolygonF polygon;
    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = this->sample( i );

        if ( !sample.interval.isValid() )
        {
            flushPolygon( painter, v0, polygon );
            previous = sample;
            continue;
        }

        if ( previous.interval.isValid() )
        {
            if ( !qwtIsCombinable( previous.interval, sample.interval ) )
                flushPolygon( painter, v0, polygon );
        }

        if ( orientation() == Qt::Vertical )
        {
            double x1 = xMap.transform( sample.interval.minValue() );
            double x2 = xMap.transform( sample.interval.maxValue() );
            double y = yMap.transform( sample.value );
            if ( doAlign )
            {
                x1 = qRound( x1 );
                x2 = qRound( x2 );
                y = qRound( y );
            }

            if ( polygon.size() == 0 )
                polygon += QPointF( x1, v0 );

            polygon += QPointF( x1, y );
            polygon += QPointF( x2, y );
        }
        else
        {
            double y1 = yMap.transform( sample.interval.minValue() );
            double y2 = yMap.transform( sample.interval.maxValue() );
            double x = xMap.transform( sample.value );
            if ( doAlign )
            {
                y1 = qRound( y1 );
                y2 = qRound( y2 );
                x = qRound( x );
            }

            if ( polygon.size() == 0 )
                polygon += QPointF( v0, y1 );

            polygon += QPointF( x, y1 );
            polygon += QPointF( x, y2 );
        }
        previous = sample;
    }

    flushPolygon( painter, v0, polygon );
}

/*!
   Draw a histogram in Columns style()

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         histogram will be painted to its last point.

   \sa setStyle(), style(), setSymbol(), drawColumn()
 */
void QwtPlotHistogram::drawColumns( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    int from, int to ) const
{
    painter->setPen( m_data->pen );
    painter->setBrush( m_data->brush );

    const QwtSeriesData< QwtIntervalSample >* series = data();

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = series->sample( i );
        if ( !sample.interval.isNull() )
        {
            const QwtColumnRect rect = columnRect( sample, xMap, yMap );
            drawColumn( painter, rect, sample );
        }
    }
}

/*!
   Draw a histogram in Lines style()

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param from Index of the first sample to be painted
   \param to Index of the last sample to be painted. If to < 0 the
         histogram will be painted to its last point.

   \sa setStyle(), style(), setPen()
 */
void QwtPlotHistogram::drawLines( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    painter->setPen( m_data->pen );
    painter->setBrush( Qt::NoBrush );

    const QwtSeriesData< QwtIntervalSample >* series = data();

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample sample = series->sample( i );
        if ( !sample.interval.isNull() )
        {
            const QwtColumnRect rect = columnRect( sample, xMap, yMap );

            QRectF r = rect.toRect();
            if ( doAlign )
            {
                r.setLeft( qRound( r.left() ) );
                r.setRight( qRound( r.right() ) );
                r.setTop( qRound( r.top() ) );
                r.setBottom( qRound( r.bottom() ) );
            }

            switch ( rect.direction )
            {
                case QwtColumnRect::LeftToRight:
                {
                    QwtPainter::drawLine( painter,
                        r.topRight(), r.bottomRight() );
                    break;
                }
                case QwtColumnRect::RightToLeft:
                {
                    QwtPainter::drawLine( painter,
                        r.topLeft(), r.bottomLeft() );
                    break;
                }
                case QwtColumnRect::TopToBottom:
                {
                    QwtPainter::drawLine( painter,
                        r.bottomRight(), r.bottomLeft() );
                    break;
                }
                case QwtColumnRect::BottomToTop:
                {
                    QwtPainter::drawLine( painter,
                        r.topRight(), r.topLeft() );
                    break;
                }
            }
        }
    }
}

//! Internal, used by the Outline style.
void QwtPlotHistogram::flushPolygon( QPainter* painter,
    double baseLine, QPolygonF& polygon ) const
{
    if ( polygon.size() == 0 )
        return;

    if ( orientation() == Qt::Horizontal )
        polygon += QPointF( baseLine, polygon.last().y() );
    else
        polygon += QPointF( polygon.last().x(), baseLine );

    if ( m_data->brush.style() != Qt::NoBrush )
    {
        painter->setPen( Qt::NoPen );
        painter->setBrush( m_data->brush );

        if ( orientation() == Qt::Horizontal )
        {
            polygon += QPointF( polygon.last().x(), baseLine );
            polygon += QPointF( polygon.first().x(), baseLine );
        }
        else
        {
            polygon += QPointF( baseLine, polygon.last().y() );
            polygon += QPointF( baseLine, polygon.first().y() );
        }

        QwtPainter::drawPolygon( painter, polygon );

        polygon.pop_back();
        polygon.pop_back();
    }
    if ( m_data->pen.style() != Qt::NoPen )
    {
        painter->setBrush( Qt::NoBrush );
        painter->setPen( m_data->pen );
        QwtPainter::drawPolyline( painter, polygon );
    }
    polygon.clear();
}

/*!
   Calculate the area that is covered by a sample

   \param sample Sample
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.

   \return Rectangle, that is covered by a sample
 */
QwtColumnRect QwtPlotHistogram::columnRect( const QwtIntervalSample& sample,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap ) const
{
    QwtColumnRect rect;

    const QwtInterval& iv = sample.interval;
    if ( !iv.isValid() )
        return rect;

    if ( orientation() == Qt::Horizontal )
    {
        const double x0 = xMap.transform( baseline() );
        const double x = xMap.transform( sample.value );
        const double y1 = yMap.transform( iv.minValue() );
        const double y2 = yMap.transform( iv.maxValue() );

        rect.hInterval.setInterval( x0, x );
        rect.vInterval.setInterval( y1, y2, iv.borderFlags() );
        rect.direction = ( x < x0 ) ? QwtColumnRect::RightToLeft :
            QwtColumnRect::LeftToRight;
    }
    else
    {
        const double x1 = xMap.transform( iv.minValue() );
        const double x2 = xMap.transform( iv.maxValue() );
        const double y0 = yMap.transform( baseline() );
        const double y = yMap.transform( sample.value );

        rect.hInterval.setInterval( x1, x2, iv.borderFlags() );
        rect.vInterval.setInterval( y0, y );
        rect.direction = ( y < y0 ) ? QwtColumnRect::BottomToTop :
            QwtColumnRect::TopToBottom;
    }

    return rect;
}

/*!
   Draw a column for a sample in Columns style().

   When a symbol() has been set the symbol is used otherwise the
   column is displayed as plain rectangle using pen() and brush().

   \param painter Painter
   \param rect Rectangle where to paint the column in paint device coordinates
   \param sample Sample to be displayed

   \note In applications, where different intervals need to be displayed
        in a different way ( f.e different colors or even using different symbols)
        it is recommended to overload drawColumn().
 */
void QwtPlotHistogram::drawColumn( QPainter* painter,
    const QwtColumnRect& rect, const QwtIntervalSample& sample ) const
{
    Q_UNUSED( sample );

    if ( m_data->symbol &&
        ( m_data->symbol->style() != QwtColumnSymbol::NoStyle ) )
    {
        m_data->symbol->draw( painter, rect );
    }
    else
    {
        QRectF r = rect.toRect();
        if ( QwtPainter::roundingAlignment( painter ) )
        {
            r.setLeft( qRound( r.left() ) );
            r.setRight( qRound( r.right() ) );
            r.setTop( qRound( r.top() ) );
            r.setBottom( qRound( r.bottom() ) );
        }

        QwtPainter::drawRect( painter, r );
    }
}

/*!
   A plain rectangle without pen using the brush()

   \param index Index of the legend entry
                ( ignored as there is only one )
   \param size Icon size
   \return A graphic displaying the icon

   \sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 */
QwtGraphic QwtPlotHistogram::legendIcon( int index, const QSizeF& size ) const
{
    Q_UNUSED( index );
    return defaultIcon( m_data->brush, size );
}
