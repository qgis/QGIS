/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_BAR_CHART_H
#define QWT_PLOT_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_abstract_barchart.h"

class QwtColumnRect;
class QwtColumnSymbol;
template< typename T > class QwtSeriesData;

/*!
   \brief QwtPlotBarChart displays a series of a values as bars.

   Each bar might be customized individually by implementing
   a specialSymbol(). Otherwise it is rendered using a default symbol.

   Depending on its orientation() the bars are displayed horizontally
   or vertically. The bars cover the interval between the baseline()
   and the value.

   By activating the LegendBarTitles mode each sample will have
   its own entry on the legend.

   The most common use case of a bar chart is to display a
   list of y coordinates, where the x coordinate is simply the index
   in the list. But for other situations ( f.e. when values are related
   to dates ) it is also possible to set x coordinates explicitly.

   \sa QwtPlotMultiBarChart, QwtPlotHistogram, QwtPlotCurve::Sticks,
      QwtPlotSeriesItem::orientation(), QwtPlotAbstractBarChart::baseline()
 */
class QWT_EXPORT QwtPlotBarChart
    : public QwtPlotAbstractBarChart
    , public QwtSeriesStore< QPointF >
{
  public:
    /*!
       \brief Legend modes.

       The default setting is QwtPlotBarChart::LegendChartTitle.
       \sa setLegendMode(), legendMode()
     */
    enum LegendMode
    {
        /*!
           One entry on the legend showing the default symbol
           and the title() of the chart

           \sa QwtPlotItem::title()
         */
        LegendChartTitle,

        /*!
           One entry for each value showing the individual symbol
           of the corresponding bar and the bar title.

           \sa specialSymbol(), barTitle()
         */
        LegendBarTitles
    };

    explicit QwtPlotBarChart( const QString& title = QString() );
    explicit QwtPlotBarChart( const QwtText& title );

    virtual ~QwtPlotBarChart();

    virtual int rtti() const QWT_OVERRIDE;

    void setSamples( const QVector< QPointF >& );
    void setSamples( const QVector< double >& );
    void setSamples( QwtSeriesData< QPointF >* );

    void setSymbol( QwtColumnSymbol* );
    const QwtColumnSymbol* symbol() const;

    void setLegendMode( LegendMode );
    LegendMode legendMode() const;

    virtual void drawSeries( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const QWT_OVERRIDE;

    virtual QRectF boundingRect() const QWT_OVERRIDE;

    virtual QwtColumnSymbol* specialSymbol(
        int sampleIndex, const QPointF& ) const;

    virtual QwtText barTitle( int sampleIndex ) const;

  protected:
    virtual void drawSample( QPainter* painter,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, const QwtInterval& boundingInterval,
        int index, const QPointF& sample ) const;

    virtual void drawBar( QPainter*,
        int sampleIndex, const QPointF& sample,
        const QwtColumnRect& ) const;

    QwtColumnRect columnRect(
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, const QwtInterval& boundingInterval,
        const QPointF& sample ) const;

    QList< QwtLegendData > legendData() const QWT_OVERRIDE;
    QwtGraphic legendIcon( int index, const QSizeF& ) const QWT_OVERRIDE;

  private:
    void init();

    class PrivateData;
    PrivateData* m_data;
};

#endif
