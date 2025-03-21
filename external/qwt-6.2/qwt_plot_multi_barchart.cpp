/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_multi_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_column_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_legend_data.h"
#include "qwt_math.h"

#include <qmap.h>

inline static bool qwtIsIncreasing(
    const QwtScaleMap& map, const QVector< double >& values )
{
    bool isInverting = map.isInverting();

    for ( int i = 0; i < values.size(); i++ )
    {
        const double y = values[ i ];
        if ( y != 0.0 )
            return ( map.isInverting() != ( y > 0.0 ) );
    }

    return !isInverting;
}

class QwtPlotMultiBarChart::PrivateData
{
  public:
    PrivateData()
        : style( QwtPlotMultiBarChart::Grouped )
    {
    }

    QwtPlotMultiBarChart::ChartStyle style;
    QList< QwtText > barTitles;
    QMap< int, QwtColumnSymbol* > symbolMap;
};

/*!
   Constructor
   \param title Title of the chart
 */
QwtPlotMultiBarChart::QwtPlotMultiBarChart( const QwtText& title )
    : QwtPlotAbstractBarChart( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the chart
 */
QwtPlotMultiBarChart::QwtPlotMultiBarChart( const QString& title )
    : QwtPlotAbstractBarChart( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotMultiBarChart::~QwtPlotMultiBarChart()
{
    resetSymbolMap();
    delete m_data;
}

void QwtPlotMultiBarChart::init()
{
    m_data = new PrivateData;
    setData( new QwtSetSeriesData() );
}

//! \return QwtPlotItem::Rtti_PlotBarChart
int QwtPlotMultiBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotMultiBarChart;
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of points
 */
void QwtPlotMultiBarChart::setSamples(
    const QVector< QwtSetSample >& samples )
{
    setData( new QwtSetSeriesData( samples ) );
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of points
 */
void QwtPlotMultiBarChart::setSamples(
    const QVector< QVector< double > >& samples )
{
    QVector< QwtSetSample > s;
    s.reserve( samples.size() );

    for ( int i = 0; i < samples.size(); i++ )
        s += QwtSetSample( i, samples[ i ] );

    setData( new QwtSetSeriesData( s ) );
}

/*!
   Assign a series of samples

   setSamples() is just a wrapper for setData() without any additional
   value - beside that it is easier to find for the developer.

   \param data Data
   \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
 */
void QwtPlotMultiBarChart::setSamples(
    QwtSeriesData< QwtSetSample >* data )
{
    setData( data );
}

/*!
   \brief Set the titles for the bars

   The titles are used for the legend.

   \param titles Bar titles

   \sa barTitles(), legendData()
 */
void QwtPlotMultiBarChart::setBarTitles( const QList< QwtText >& titles )
{
    m_data->barTitles = titles;
    itemChanged();
}

/*!
   \return Bar titles
   \sa setBarTitles(), legendData()
 */
QList< QwtText > QwtPlotMultiBarChart::barTitles() const
{
    return m_data->barTitles;
}

/*!
   \brief Add a symbol to the symbol map

   Assign a default symbol for drawing the bar representing all values
   with the same index in a set.

   \param valueIndex Index of a value in a set
   \param symbol Symbol used for drawing a bar

   \sa symbol(), resetSymbolMap(), specialSymbol()
 */
void QwtPlotMultiBarChart::setSymbol( int valueIndex, QwtColumnSymbol* symbol )
{
    if ( valueIndex < 0 )
        return;

    QMap< int, QwtColumnSymbol* >::iterator it =
        m_data->symbolMap.find(valueIndex);
    if ( it == m_data->symbolMap.end() )
    {
        if ( symbol != NULL )
        {
            m_data->symbolMap.insert( valueIndex, symbol );

            legendChanged();
            itemChanged();
        }
    }
    else
    {
        if ( symbol != it.value() )
        {
            delete it.value();

            if ( symbol == NULL )
            {
                m_data->symbolMap.remove( valueIndex );
            }
            else
            {
                it.value() = symbol;
            }

            legendChanged();
            itemChanged();
        }
    }
}

/*!
   Find a symbol in the symbol map

   \param valueIndex Index of a value in a set
   \return The symbol, that had been set by setSymbol() or NULL.

   \sa setSymbol(), specialSymbol(), drawBar()
 */
const QwtColumnSymbol* QwtPlotMultiBarChart::symbol( int valueIndex ) const
{
    QMap< int, QwtColumnSymbol* >::const_iterator it =
        m_data->symbolMap.constFind( valueIndex );

    return ( it == m_data->symbolMap.constEnd() ) ? NULL : it.value();
}

/*!
   Find a symbol in the symbol map

   \param valueIndex Index of a value in a set
   \return The symbol, that had been set by setSymbol() or NULL.

   \sa setSymbol(), specialSymbol(), drawBar()
 */
QwtColumnSymbol* QwtPlotMultiBarChart::symbol( int valueIndex )
{
    QMap< int, QwtColumnSymbol* >::const_iterator it =
        m_data->symbolMap.constFind( valueIndex );

    return ( it == m_data->symbolMap.constEnd() ) ? NULL : it.value();
}

/*!
   Remove all symbols from the symbol map
 */
void QwtPlotMultiBarChart::resetSymbolMap()
{
    qDeleteAll( m_data->symbolMap );
    m_data->symbolMap.clear();
}

/*!
   \brief Create a symbol for special values

   Usually the symbols for displaying a bar are set by setSymbols() and
   common for all sets. By overloading specialSymbol() it is possible to
   create a temporary symbol() for displaying a special value.

   The symbol has to be created by new each time specialSymbol() is
   called. As soon as the symbol is painted this symbol gets deleted.

   When no symbol ( NULL ) is returned, the value will be displayed
   with the standard symbol that is used for all symbols with the same
   valueIndex.

   \param sampleIndex Index of the sample
   \param valueIndex Index of the value in the set

   \return NULL, meaning that the value is not special

 */
QwtColumnSymbol* QwtPlotMultiBarChart::specialSymbol(
    int sampleIndex, int valueIndex ) const
{
    Q_UNUSED( sampleIndex );
    Q_UNUSED( valueIndex );

    return NULL;
}

/*!
   Set the style of the chart

   \param style Chart style
   \sa style()
 */
void QwtPlotMultiBarChart::setStyle( ChartStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Style of the chart
   \sa setStyle()
 */
QwtPlotMultiBarChart::ChartStyle QwtPlotMultiBarChart::style() const
{
    return m_data->style;
}

/*!
   \return Bounding rectangle of all samples.
   For an empty series the rectangle is invalid.
 */
QRectF QwtPlotMultiBarChart::boundingRect() const
{
    const size_t numSamples = dataSize();

    if ( numSamples == 0 )
        return QwtPlotSeriesItem::boundingRect();

    const double baseLine = baseline();

    QRectF rect;

    if ( m_data->style != QwtPlotMultiBarChart::Stacked )
    {
        rect = QwtPlotSeriesItem::boundingRect();

        if ( rect.height() >= 0 )
        {
            if ( rect.bottom() < baseLine )
                rect.setBottom( baseLine );
            if ( rect.top() > baseLine )
                rect.setTop( baseLine );
        }
    }
    else
    {
        double xMin, xMax, yMin, yMax;

        xMin = xMax = 0.0;
        yMin = yMax = baseLine;

        const QwtSeriesData< QwtSetSample >* series = data();

        for ( size_t i = 0; i < numSamples; i++ )
        {
            const QwtSetSample sample = series->sample( i );
            if ( i == 0 )
            {
                xMin = xMax = sample.value;
            }
            else
            {
                xMin = qwtMinF( xMin, sample.value );
                xMax = qwtMaxF( xMax, sample.value );
            }

            const double y = baseLine + sample.added();

            yMin = qwtMinF( yMin, y );
            yMax = qwtMaxF( yMax, y );
        }
        rect.setRect( xMin, yMin, xMax - xMin, yMax - yMin );
    }

    if ( orientation() == Qt::Horizontal )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/*!
   Draw an interval of the bar chart

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

   \sa drawSymbols()
 */
void QwtPlotMultiBarChart::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;


    const QRectF br = data()->boundingRect();
    const QwtInterval interval( br.left(), br.right() );

    painter->save();

    for ( int i = from; i <= to; i++ )
    {
        drawSample( painter, xMap, yMap,
            canvasRect, interval, i, sample( i ) );
    }

    painter->restore();
}

/*!
   Draw a sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param boundingInterval Bounding interval of sample values
   \param index Index of the sample to be painted
   \param sample Sample value

   \sa drawSeries()
 */
void QwtPlotMultiBarChart::drawSample( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, const QwtInterval& boundingInterval,
    int index, const QwtSetSample& sample ) const
{
    if ( sample.set.size() <= 0 )
        return;

    double sampleW;

    if ( orientation() == Qt::Horizontal )
    {
        sampleW = sampleWidth( yMap, canvasRect.height(),
            boundingInterval.width(), sample.value );
    }
    else
    {
        sampleW = sampleWidth( xMap, canvasRect.width(),
            boundingInterval.width(), sample.value );
    }

    if ( m_data->style == Stacked )
    {
        drawStackedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
    else
    {
        drawGroupedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
}

/*!
   Draw a grouped sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param index Index of the sample to be painted
   \param sampleWidth Bounding width for all bars of the sample
   \param sample Sample

   \sa drawSeries(), sampleWidth()
 */
void QwtPlotMultiBarChart::drawGroupedBars( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int index, double sampleWidth,
    const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect );

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    if ( orientation() == Qt::Vertical )
    {
        const double barWidth = sampleWidth / numBars;

        const double y1 = yMap.transform( baseline() );
        const double x0 = xMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            const double x1 = x0 + i * barWidth;
            const double x2 = x1 + barWidth;

            const double y2 = yMap.transform( sample.set[i] );

            QwtColumnRect barRect;
            barRect.direction = ( y1 < y2 ) ?
                QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

            barRect.hInterval = QwtInterval( x1, x2 ).normalized();
            if ( i != 0 )
                barRect.hInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            barRect.vInterval = QwtInterval( y1, y2 ).normalized();

            drawBar( painter, index, i, barRect );
        }
    }
    else
    {
        const double barHeight = sampleWidth / numBars;

        const double x1 = xMap.transform( baseline() );
        const double y0 = yMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            double y1 = y0 + i * barHeight;
            double y2 = y1 + barHeight;

            double x2 = xMap.transform( sample.set[i] );

            QwtColumnRect barRect;
            barRect.direction = x1 < x2 ?
                QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

            barRect.hInterval = QwtInterval( x1, x2 ).normalized();

            barRect.vInterval = QwtInterval( y1, y2 );
            if ( i != 0 )
                barRect.vInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            drawBar( painter, index, i, barRect );
        }
    }
}

/*!
   Draw a stacked sample

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param index Index of the sample to be painted
   \param sampleWidth Width of the bars
   \param sample Sample

   \sa drawSeries(), sampleWidth()
 */
void QwtPlotMultiBarChart::drawStackedBars( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int index,
    double sampleWidth, const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect ); // clipping the bars ?

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    QwtInterval::BorderFlag borderFlags = QwtInterval::IncludeBorders;

    if ( orientation() == Qt::Vertical )
    {
        const double x1 = xMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double x2 = x1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( yMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

        bar.hInterval = QwtInterval( x1, x2 ).normalized();

        double sum = baseline();

        for ( int i = 0; i < numBars; i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double y1 = yMap.transform( sum );
            const double y2 = yMap.transform( sum + si );

            if ( ( y2 > y1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.vInterval = QwtInterval( y1, y2 ).normalized();
            bar.vInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
    else
    {
        const double y1 = yMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double y2 = y1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( xMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;
        bar.vInterval = QwtInterval( y1, y2 ).normalized();

        double sum = baseline();

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double x1 = xMap.transform( sum );
            const double x2 = xMap.transform( sum + si );

            if ( ( x2 > x1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            bar.hInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
}

/*!
   Draw a bar

   \param painter Painter
   \param sampleIndex Index of the sample - might be -1 when the
                     bar is painted for the legend
   \param valueIndex Index of a value in a set
   \param rect Directed target rectangle for the bar

   \sa drawSeries()
 */
void QwtPlotMultiBarChart::drawBar( QPainter* painter,
    int sampleIndex, int valueIndex, const QwtColumnRect& rect ) const
{
    const QwtColumnSymbol* specialSym = NULL;
    if ( sampleIndex >= 0 )
        specialSym = specialSymbol( sampleIndex, valueIndex );

    const QwtColumnSymbol* sym = specialSym;
    if ( sym == NULL )
        sym = symbol( valueIndex );

    if ( sym )
    {
        sym->draw( painter, rect );
    }
    else
    {
        // we build a temporary default symbol
        QwtColumnSymbol columnSymbol( QwtColumnSymbol::Box );
        columnSymbol.setLineWidth( 1 );
        columnSymbol.setFrameStyle( QwtColumnSymbol::Plain );
        columnSymbol.draw( painter, rect );
    }

    delete specialSym;
}

/*!
   \return Information to be displayed on the legend

   The chart is represented by a list of entries - one for each bar title.
   Each element contains a bar title and an icon showing its corresponding bar.

   \sa barTitles(), legendIcon(), legendIconSize()
 */
QList< QwtLegendData > QwtPlotMultiBarChart::legendData() const
{
    QList< QwtLegendData > list;
    list.reserve( m_data->barTitles.size() );

    for ( int i = 0; i < m_data->barTitles.size(); i++ )
    {
        QwtLegendData data;

        data.setValue( QwtLegendData::TitleRole,
            QVariant::fromValue( m_data->barTitles[i] ) );

        if ( !legendIconSize().isEmpty() )
        {
            data.setValue( QwtLegendData::IconRole,
                QVariant::fromValue( legendIcon( i, legendIconSize() ) ) );
        }

        list += data;
    }

    return list;
}

/*!
   \return Icon for representing a bar on the legend

   \param index Index of the bar
   \param size Icon size

   \return An icon showing a bar
   \sa drawBar(), legendData()
 */
QwtGraphic QwtPlotMultiBarChart::legendIcon( int index,
    const QSizeF& size ) const
{
    QwtColumnRect column;
    column.hInterval = QwtInterval( 0.0, size.width() - 1.0 );
    column.vInterval = QwtInterval( 0.0, size.height() - 1.0 );

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    drawBar( &painter, -1, index, column );

    return icon;
}

