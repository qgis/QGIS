/***************************************************************************
                         qgsplot.h
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPLOT_H
#define QGSPLOT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstextformat.h"
#include "qgsmargins.h"

#include <QSizeF>
#include <memory>

class QgsLineSymbol;
class QgsFillSymbol;
class QgsRenderContext;
class QgsNumericFormat;


/**
 * \brief Base class for plot/chart/graphs.
 *
 * \ingroup core
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPlot
{
  public:

    /**
     * Constructor for QgsPlot.
     */
    QgsPlot() = default;

    virtual ~QgsPlot();

  private:


};

/**
 * \brief Base class for 2-dimensional plot/chart/graphs.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT Qgs2DPlot : public QgsPlot
{
  public:

    /**
     * Constructor for Qgs2DPlot.
     */
    Qgs2DPlot();

    ~Qgs2DPlot() override;

    //! Qgs2DPlot cannot be copied
    Qgs2DPlot( const Qgs2DPlot &other ) = delete;
    //! Qgs2DPlot cannot be copied
    Qgs2DPlot &operator=( const Qgs2DPlot &other ) = delete;

    /**
     * Renders the plot.
     */
    void render( QgsRenderContext &context );

    /**
     * Returns the overall size of the plot (in millimeters) (including titles and over components which sit outside the plot area).
     *
     * \see setSize()
     */
    QSizeF size() const;

    /**
     * Sets the overall \a size of the plot (including titles and over components which sit outside the plot area).
     *
     * \see size()
     */
    void setSize( QSizeF size );

    /**
     * Returns the minimum value of the x axis.
     *
     * \see setXMinimum()
     */
    double xMinimum() const { return mMinX; }

    /**
     * Sets the \a minimum value of the x axis.
     *
     * \see xMinimum()
     */
    void setXMinimum( double minimum ) { mMinX = minimum; }

    /**
     * Returns the minimum value of the y axis.
     *
     * \see setYMinimum()
     */
    double yMinimum() const { return mMinY; }

    /**
     * Sets the \a minimum value of the y axis.
     *
     * \see yMinimum()
     */
    void setYMinimum( double minimum ) { mMinY = minimum; }

    /**
     * Returns the maximum value of the x axis.
     *
     * \see setXMaximum()
     */
    double xMaximum() const { return mMaxX; }

    /**
     * Sets the \a maximum value of the x axis.
     *
     * \see xMaximum()
     */
    void setXMaximum( double maximum ) { mMaxX = maximum; }

    /**
     * Returns the maximum value of the y axis.
     *
     * \see setYMaximum()
     */
    double yMaximum() const { return mMaxY; }

    /**
     * Sets the \a maximum value of the y axis.
     *
     * \see yMaximum()
     */
    void setYMaximum( double maximum ) { mMaxY = maximum; }

    /**
     * Returns the interval of minor grid lines for the x axis.
     *
     * \see setGridIntervalMinorX()
     */
    double gridIntervalMinorX() const { return mGridIntervalMinorX; }

    /**
     * Sets the \a interval of minor grid lines for the x axis.
     *
     * \see gridIntervalMinorX()
     */
    void setGridIntervalMinorX( double interval ) { mGridIntervalMinorX = interval; }

    /**
     * Returns the interval of major grid lines for the x axis.
     *
     * \see setGridIntervalMajorX()
     */
    double gridIntervalMajorX() const { return mGridIntervalMajorX; }

    /**
     * Sets the \a interval of major grid lines for the x axis.
     *
     * \see gridIntervalMajorX()
     */
    void setGridIntervalMajorX( double interval ) { mGridIntervalMajorX = interval; }

    /**
     * Returns the interval of minor grid lines for the y axis.
     *
     * \see setGridIntervalMinorY()
     */
    double gridIntervalMinorY() const { return mGridIntervalMinorY; }

    /**
     * Sets the \a interval of minor grid lines for the y axis.
     *
     * \see gridIntervalMinorY()
     */
    void setGridIntervalMinorY( double interval ) { mGridIntervalMinorY = interval; }

    /**
     * Returns the interval of major grid lines for the y axis.
     *
     * \see setGridIntervalMajorY()
     */
    double gridIntervalMajorY() const { return mGridIntervalMajorY; }

    /**
     * Sets the \a interval of major grid lines for the y axis.
     *
     * \see gridIntervalMajorY()
     */
    void setGridIntervalMajorY( double interval ) { mGridIntervalMajorY = interval; }

    /**
     * Returns the interval of labels for the x axis.
     *
     * \see setLabelIntervalX()
     */
    double labelIntervalX() const { return mLabelIntervalX; }

    /**
     * Sets the \a interval of labels for the x axis.
     *
     * \see labelIntervalX()
     */
    void setLabelIntervalX( double interval ) { mLabelIntervalX = interval; }

    /**
     * Returns the interval of labels for the y axis.
     *
     * \see setLabelIntervalY()
     */
    double labelIntervalY() const { return mLabelIntervalY; }

    /**
     * Sets the \a interval of labels for the y axis.
     *
     * \see labelIntervalY()
     */
    void setLabelIntervalY( double interval ) { mLabelIntervalY = interval; }

    /**
     * Returns the fill symbol used to render the background of the chart.
     *
     * \see setChartBackgroundSymbol()
     */
    QgsFillSymbol *chartBackgroundSymbol();

    /**
     * Sets the fill \a symbol used to render the background of the chart.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see chartBackgroundSymbol()
     */
    void setChartBackgroundSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbol used to render the border of the chart.
     *
     * \see setChartBorderSymbol()
     */
    QgsFillSymbol *chartBorderSymbol();

    /**
     * Sets the \a symbol used to render the border of the chart.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see chartBorderSymbol()
     */
    void setChartBorderSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the major lines in the x axis grid.
     *
     * \see setXGridMajorSymbol()
     */
    QgsLineSymbol *xGridMajorSymbol();

    /**
     * Sets the \a symbol used to render the major lines in the x axis grid.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see xGridMajorSymbol()
     */
    void setXGridMajorSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the minor lines in the x axis grid.
     *
     * \see setXGridMinorSymbol()
     */
    QgsLineSymbol *xGridMinorSymbol();

    /**
     * Sets the \a symbol used to render the minor lines in the x axis grid.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see xGridMinorSymbol()
     */
    void setXGridMinorSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the major lines in the y axis grid.
     *
     * \see setYGridMajorSymbol()
     */
    QgsLineSymbol *yGridMajorSymbol();

    /**
     * Sets the \a symbol used to render the major lines in the y axis grid.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see yGridMajorSymbol()
     */
    void setYGridMajorSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the minor lines in the y axis grid.
     *
     * \see setYGridMinorSymbol()
     */
    QgsLineSymbol *yGridMinorSymbol();

    /**
     * Sets the \a symbol used to render the minor lines in the y axis grid.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see YGridMinorSymbol()
     */
    void setYGridMinorSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the text format used for the x axis labels.
     *
     * \see setXAxisTextFormat()
     */
    QgsTextFormat xAxisTextFormat() const;

    /**
     * Sets the text \a format used for the x axis labels.
     *
     * \see xAxisTextFormat()
     */
    void setXAxisTextFormat( const QgsTextFormat &format );

    /**
     * Returns the text format used for the y axis labels.
     *
     * \see setYAxisTextFormat()
     */
    QgsTextFormat yAxisTextFormat() const;

    /**
     * Sets the text \a format used for the y axis labels.
     *
     * \see yAxisTextFormat()
     */
    void setYAxisTextFormat( const QgsTextFormat &format );

    /**
     * Returns the margins of the plot area (in millimeters)
     *
     * \see setMargins()
     */
    const QgsMargins &margins() const;

    /**
     * Sets the \a margins of the plot area (in millimeters)
     *
     * \see setMargins()
     */
    void setMargins( const QgsMargins &margins );

    /**
     * Returns the numeric format used for the x axis labels.
     *
     * \see setXAxisNumericFormat()
     */
    QgsNumericFormat *xAxisNumericFormat();

    /**
     * Sets the numeric \a format used for the x axis labels.
     *
     * Ownership of \a format is transferred to the plot.
     *
     * \see xAxisNumericFormat()
     */
    void setXAxisNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the numeric format used for the y axis labels.
     *
     * \see setYAxisNumericFormat()
     */
    QgsNumericFormat *yAxisNumericFormat();

    /**
     * Sets the numeric \a format used for the y axis labels.
     *
     * Ownership of \a format is transferred to the plot.
     *
     * \see yAxisNumericFormat()
     */
    void setYAxisNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

  private:

#ifdef SIP_RUN
    Qgs2DPlot( const Qgs2DPlot &other );
#endif

    QSizeF mSize;

    double mMinX = 0;
    double mMinY = 0;
    double mMaxX = 10;
    double mMaxY = 10;

    double mGridIntervalMinorX = 1;
    double mGridIntervalMajorX = 5;
    double mGridIntervalMinorY = 1;
    double mGridIntervalMajorY = 5;

    double mLabelIntervalX = 1;
    double mLabelIntervalY = 1;

    std::unique_ptr< QgsNumericFormat > mXAxisNumericFormat;
    std::unique_ptr< QgsNumericFormat > mYAxisNumericFormat;

    std::unique_ptr< QgsFillSymbol > mChartBackgroundSymbol;
    std::unique_ptr< QgsFillSymbol > mChartBorderSymbol;

    std::unique_ptr< QgsLineSymbol > mXGridMajorSymbol;
    std::unique_ptr< QgsLineSymbol > mXGridMinorSymbol;
    std::unique_ptr< QgsLineSymbol > mYGridMajorSymbol;
    std::unique_ptr< QgsLineSymbol > mYGridMinorSymbol;

    QgsTextFormat mXAxisLabelTextFormat;
    QgsTextFormat mYAxisLabelTextFormat;

    QgsMargins mMargins;
};

#endif // QGSPLOT_H
