/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_MULTI_BAR_CHART_H
#define QWT_PLOT_MULTI_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_abstract_barchart.h"

class QwtColumnRect;
class QwtColumnSymbol;
template< typename T > class QwtSeriesData;

/*!
   \brief QwtPlotMultiBarChart displays a series of a samples that consist
         each of a set of values.

   Each value is displayed as a bar, the bars of each set can be organized
   side by side or accumulated.

   Each bar of a set is rendered by a QwtColumnSymbol, that is set by setSymbol().
   The bars of different sets use the same symbols. Exceptions are possible
   by overloading specialSymbol() or overloading drawBar().

   Depending on its orientation() the bars are displayed horizontally
   or vertically. The bars cover the interval between the baseline()
   and the value.

   In opposite to most other plot items, QwtPlotMultiBarChart returns more
   than one entry for the legend - one for each symbol.

   \sa QwtPlotBarChart, QwtPlotHistogram
      QwtPlotSeriesItem::orientation(), QwtPlotAbstractBarChart::baseline()
 */
class QWT_EXPORT QwtPlotMultiBarChart
    : public QwtPlotAbstractBarChart
    , public QwtSeriesStore< QwtSetSample >
{
  public:
    /*!
        \brief Chart styles.

        The default setting is QwtPlotMultiBarChart::Grouped.
        \sa setStyle(), style()
     */
    enum ChartStyle
    {
        //! The bars of a set are displayed side by side
        Grouped,

        /*!
            The bars are displayed on top of each other accumulating
            to a single bar. All values of a set need to have the same
            sign.
         */
        Stacked
    };

    explicit QwtPlotMultiBarChart( const QString& title = QString() );
    explicit QwtPlotMultiBarChart( const QwtText& title );

    virtual ~QwtPlotMultiBarChart();

    virtual int rtti() const QWT_OVERRIDE;

    void setBarTitles( const QList< QwtText >& );
    QList< QwtText > barTitles() const;

    void setSamples( const QVector< QwtSetSample >& );
    void setSamples( const QVector< QVector< double > >& );
    void setSamples( QwtSeriesData< QwtSetSample >* );

    void setStyle( ChartStyle style );
    ChartStyle style() const;

    void setSymbol( int valueIndex, QwtColumnSymbol* );
    const QwtColumnSymbol* symbol( int valueIndex ) const;

    void resetSymbolMap();

    virtual void drawSeries( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const QWT_OVERRIDE;

    virtual QRectF boundingRect() const QWT_OVERRIDE;

    virtual QList< QwtLegendData > legendData() const QWT_OVERRIDE;

    virtual QwtGraphic legendIcon(
        int index, const QSizeF& ) const QWT_OVERRIDE;

  protected:
    QwtColumnSymbol* symbol( int valueIndex );

    virtual QwtColumnSymbol* specialSymbol(
        int sampleIndex, int valueIndex ) const;

    virtual void drawSample( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, const QwtInterval& boundingInterval,
        int index, const QwtSetSample& ) const;

    virtual void drawBar( QPainter*, int sampleIndex,
        int valueIndex, const QwtColumnRect& ) const;

    void drawStackedBars( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int index,
        double sampleWidth, const QwtSetSample& ) const;

    void drawGroupedBars( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int index,
        double sampleWidth, const QwtSetSample& ) const;

  private:
    void init();

    class PrivateData;
    PrivateData* m_data;
};

#endif
