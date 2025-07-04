/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_seriesitem.h"
#include "qwt_scale_div.h"
#include "qwt_text.h"

class QwtPlotSeriesItem::PrivateData
{
  public:
    PrivateData()
        : orientation( Qt::Vertical )
    {
    }

    Qt::Orientation orientation;
};

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotSeriesItem::QwtPlotSeriesItem( const QwtText& title )
    : QwtPlotItem( title )
{
    m_data = new PrivateData();
    setItemInterest( QwtPlotItem::ScaleInterest, true );
}

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotSeriesItem::QwtPlotSeriesItem( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    m_data = new PrivateData();
    setItemInterest( QwtPlotItem::ScaleInterest, true );
}

//! Destructor
QwtPlotSeriesItem::~QwtPlotSeriesItem()
{
    delete m_data;
}

/*!
   Set the orientation of the item.

   The orientation() might be used in specific way by a plot item.
   F.e. a QwtPlotCurve uses it to identify how to display the curve
   int QwtPlotCurve::Steps or QwtPlotCurve::Sticks style.

   \sa orientation()
 */
void QwtPlotSeriesItem::setOrientation( Qt::Orientation orientation )
{
    if ( m_data->orientation != orientation )
    {
        m_data->orientation = orientation;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Orientation of the plot item
   \sa setOrientation()
 */
Qt::Orientation QwtPlotSeriesItem::orientation() const
{
    return m_data->orientation;
}

/*!
   \brief Draw the complete series

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
 */
void QwtPlotSeriesItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    drawSeries( painter, xMap, yMap, canvasRect, 0, -1 );
}

QRectF QwtPlotSeriesItem::boundingRect() const
{
    return dataRect();
}

void QwtPlotSeriesItem::updateScaleDiv(
    const QwtScaleDiv& xScaleDiv, const QwtScaleDiv& yScaleDiv )
{
    const QRectF rect = QRectF(
        xScaleDiv.lowerBound(), yScaleDiv.lowerBound(),
        xScaleDiv.range(), yScaleDiv.range() );

    setRectOfInterest( rect );
}

void QwtPlotSeriesItem::dataChanged()
{
    itemChanged();
}
