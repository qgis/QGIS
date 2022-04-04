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

    /**
     * Writes the plot's properties into an XML \a element.
     */
    virtual bool writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context ) const;

    /**
     * Reads the plot's properties from an XML \a element.
     */
    virtual bool readXml( const QDomElement &element, QgsReadWriteContext &context );

  private:


};

/**
 * \brief Encapsulates the properties of a plot axis.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPlotAxis
{
  public:

    QgsPlotAxis();
    ~QgsPlotAxis();

    //! QgsPlotAxis cannot be copied
    QgsPlotAxis( const QgsPlotAxis &other ) = delete;
    //! QgsPlotAxis cannot be copied
    QgsPlotAxis &operator=( const QgsPlotAxis &other ) = delete;

    /**
     * Writes the axis' properties into an XML \a element.
     */
    bool writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context ) const;

    /**
     * Reads the axis' properties from an XML \a element.
     */
    bool readXml( const QDomElement &element, QgsReadWriteContext &context );

    /**
     * Returns the interval of minor grid lines for the axis.
     *
     * \see setGridIntervalMinor()
     */
    double gridIntervalMinor() const { return mGridIntervalMinor; }

    /**
     * Sets the \a interval of minor grid lines for the axis.
     *
     * \see gridIntervalMinor()
     */
    void setGridIntervalMinor( double interval ) { mGridIntervalMinor = interval; }

    /**
     * Returns the interval of major grid lines for the axis.
     *
     * \see setGridIntervalMajor()
     */
    double gridIntervalMajor() const { return mGridIntervalMajor; }

    /**
     * Sets the \a interval of major grid lines for the axis.
     *
     * \see gridIntervalMajor()
     */
    void setGridIntervalMajor( double interval ) { mGridIntervalMajor = interval; }

    /**
     * Returns the interval of labels for the axis.
     *
     * \see setLabelInterval()
     */
    double labelInterval() const { return mLabelInterval; }

    /**
     * Sets the \a interval of labels for the axis.
     *
     * \see labelInterval()
     */
    void setLabelInterval( double interval ) { mLabelInterval = interval; }

    /**
     * Returns the line symbol used to render the major lines in the axis grid.
     *
     * \see setGridMajorSymbol()
     */
    QgsLineSymbol *gridMajorSymbol();

    /**
     * Sets the \a symbol used to render the major lines in the axis grid.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see gridMajorSymbol()
     */
    void setGridMajorSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the minor lines in the axis grid.
     *
     * \see setGridMinorSymbol()
     */
    QgsLineSymbol *gridMinorSymbol();

    /**
     * Sets the \a symbol used to render the minor lines in the axis grid.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see gridMinorSymbol()
     */
    void setGridMinorSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the text format used for the axis labels.
     *
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the text \a format used for the axis labels.
     *
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Returns the numeric format used for the axis labels.
     *
     * \see setNumericFormat()
     */
    QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used for the axis labels.
     *
     * Ownership of \a format is transferred to the plot.
     *
     * \see numericFormat()
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

  private:

#ifdef SIP_RUN
    QgsPlotAxis( const QgsPlotAxis &other );
#endif

    double mGridIntervalMinor = 1;
    double mGridIntervalMajor = 5;

    double mLabelInterval = 1;

    std::unique_ptr< QgsNumericFormat > mNumericFormat;

    std::unique_ptr< QgsLineSymbol > mGridMajorSymbol;
    std::unique_ptr< QgsLineSymbol > mGridMinorSymbol;

    QgsTextFormat mLabelTextFormat;

};

/**
 * \brief Base class for 2-dimensional plot/chart/graphs.
 *
 * The base class is responsible for rendering the axis, grid lines and chart area. Subclasses
 * can implement the renderContent() method to render their actual plot content.
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

    bool writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &element, QgsReadWriteContext &context ) override;

    /**
     * Renders the plot.
     */
    void render( QgsRenderContext &context );

    /**
     * Renders the plot content.
     *
     * Subclasses can implement this method to render the actual plot content (e.g. bar charts, scatter plots).
     * This method will be called after the chart background and grid are rendered, but before the chart border is rendered.
     *
     * The default implementation does nothing.
     *
     * The \a plotArea argument specifies that area of the plot which corresponds to the actual plot content. Implementations
     * should take care to scale values accordingly to render points correctly inside this plot area.
     */
    virtual void renderContent( QgsRenderContext &context, const QRectF &plotArea );

    /**
     * Returns the overall size of the plot (in millimeters) (including titles and other components which sit outside the plot area).
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
     * Returns the area of the plot which corresponds to the actual plot content (excluding all titles and other components which sit
     * outside the plot area).
     */
    QRectF interiorPlotArea( QgsRenderContext &context ) const;

    /**
     * Automatically sets the grid and label intervals to optimal values
     * for display in the given render \a context.
     *
     * Intervals will be calculated in order to avoid overlapping axis labels and to ensure
     * round values are shown.
     */
    void calculateOptimisedIntervals( QgsRenderContext &context );

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
     * Returns a reference to the plot's x axis.
     *
     * \see yAxis()
     */
    QgsPlotAxis &xAxis() { return mXAxis; }

    /**
     * Returns a reference to the plot's x axis.
     *
     * \see yAxis()
     */
    const QgsPlotAxis &xAxis() const SIP_SKIP { return mXAxis; }

    /**
     * Returns a reference to the plot's y axis.
     *
     * \see xAxis()
     */
    QgsPlotAxis &yAxis() { return mYAxis; }


    /**
     * Returns a reference to the plot's y axis.
     *
     * \see xAxis()
     */
    const QgsPlotAxis &yAxis() const SIP_SKIP { return mYAxis; }

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

  private:

#ifdef SIP_RUN
    Qgs2DPlot( const Qgs2DPlot &other );
#endif

    QSizeF mSize;

    double mMinX = 0;
    double mMinY = 0;
    double mMaxX = 10;
    double mMaxY = 10;

    std::unique_ptr< QgsFillSymbol > mChartBackgroundSymbol;
    std::unique_ptr< QgsFillSymbol > mChartBorderSymbol;

    QgsMargins mMargins;

    QgsPlotAxis mXAxis;
    QgsPlotAxis mYAxis;
};

#endif // QGSPLOT_H
