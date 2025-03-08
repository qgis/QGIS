/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_spectrocurve.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"

#include <qpainter.h>

class QwtPlotSpectroCurve::PrivateData
{
  public:
    PrivateData()
        : colorRange( 0.0, 1000.0 )
        , penWidth( 0.0 )
        , paintAttributes( QwtPlotSpectroCurve::ClipPoints )
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    QwtColorMap* colorMap;
    QwtInterval colorRange;
    QVector< QRgb > colorTable;
    double penWidth;
    QwtPlotSpectroCurve::PaintAttributes paintAttributes;
};

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotSpectroCurve::QwtPlotSpectroCurve( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotSpectroCurve::QwtPlotSpectroCurve( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotSpectroCurve::~QwtPlotSpectroCurve()
{
    delete m_data;
}

/*!
   \brief Initialize data members
 */
void QwtPlotSpectroCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    m_data = new PrivateData;
    setData( new QwtPoint3DSeriesData() );

    setZ( 20.0 );
}

//! \return QwtPlotItem::Rtti_PlotSpectroCurve
int QwtPlotSpectroCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectroCurve;
}

/*!
   Specify an attribute how to draw the curve

   \param attribute Paint attribute
   \param on On/Off
   /sa PaintAttribute, testPaintAttribute()
 */
void QwtPlotSpectroCurve::setPaintAttribute( PaintAttribute attribute, bool on )
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
bool QwtPlotSpectroCurve::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/*!
   Initialize data with an array of samples.
   \param samples Vector of points
 */
void QwtPlotSpectroCurve::setSamples( const QVector< QwtPoint3D >& samples )
{
    setData( new QwtPoint3DSeriesData( samples ) );
}

/*!
   Assign a series of samples

   setSamples() is just a wrapper for setData() without any additional
   value - beside that it is easier to find for the developer.

   \param data Data
   \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
 */
void QwtPlotSpectroCurve::setSamples(
    QwtSeriesData< QwtPoint3D >* data )
{
    setData( data );
}

/*!
   Change the color map

   Often it is useful to display the mapping between intensities and
   colors as an additional plot axis, showing a color bar.

   \param colorMap Color Map

   \sa colorMap(), setColorRange(), QwtColorMap::color(),
      QwtScaleWidget::setColorBarEnabled(), QwtScaleWidget::setColorMap()
 */
void QwtPlotSpectroCurve::setColorMap( QwtColorMap* colorMap )
{
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
   \sa setColorMap(), setColorRange(), QwtColorMap::color()
 */
const QwtColorMap* QwtPlotSpectroCurve::colorMap() const
{
    return m_data->colorMap;
}

/*!
   Set the value interval, that corresponds to the color map

   \param interval interval.minValue() corresponds to 0.0,
                   interval.maxValue() to 1.0 on the color map.

   \sa colorRange(), setColorMap(), QwtColorMap::color()
 */
void QwtPlotSpectroCurve::setColorRange( const QwtInterval& interval )
{
    if ( interval != m_data->colorRange )
    {
        m_data->colorRange = interval;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Value interval, that corresponds to the color map
   \sa setColorRange(), setColorMap(), QwtColorMap::color()
 */
QwtInterval& QwtPlotSpectroCurve::colorRange() const
{
    return m_data->colorRange;
}

/*!
   Assign a pen width

   \param penWidth New pen width
   \sa penWidth()
 */
void QwtPlotSpectroCurve::setPenWidth(double penWidth)
{
    if ( penWidth < 0.0 )
        penWidth = 0.0;

    if ( m_data->penWidth != penWidth )
    {
        m_data->penWidth = penWidth;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Pen width used to draw a dot
   \sa setPenWidth()
 */
double QwtPlotSpectroCurve::penWidth() const
{
    return m_data->penWidth;
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

   \sa drawDots()
 */
void QwtPlotSpectroCurve::drawSeries( QPainter* painter,
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

    drawDots( painter, xMap, yMap, canvasRect, from, to );
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

   \sa drawSeries()
 */
void QwtPlotSpectroCurve::drawDots( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( !m_data->colorRange.isValid() )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    const QwtColorMap::Format format = m_data->colorMap->format();
    if ( format == QwtColorMap::Indexed )
        m_data->colorTable = m_data->colorMap->colorTable256();

    const QwtSeriesData< QwtPoint3D >* series = data();

    for ( int i = from; i <= to; i++ )
    {
        const QwtPoint3D sample = series->sample( i );

        double xi = xMap.transform( sample.x() );
        double yi = yMap.transform( sample.y() );
        if ( doAlign )
        {
            xi = qRound( xi );
            yi = qRound( yi );
        }

        if ( m_data->paintAttributes & QwtPlotSpectroCurve::ClipPoints )
        {
            if ( !canvasRect.contains( xi, yi ) )
                continue;
        }

        if ( format == QwtColorMap::RGB )
        {
            const QRgb rgb = m_data->colorMap->rgb(
                m_data->colorRange, sample.z() );

            painter->setPen( QPen( QColor::fromRgba( rgb ), m_data->penWidth ) );
        }
        else
        {
            const unsigned char index = m_data->colorMap->colorIndex(
                256, m_data->colorRange, sample.z() );

            painter->setPen( QPen( QColor::fromRgba( m_data->colorTable[index] ),
                m_data->penWidth ) );
        }

        QwtPainter::drawPoint( painter, QPointF( xi, yi ) );
    }

    m_data->colorTable.clear();
}
