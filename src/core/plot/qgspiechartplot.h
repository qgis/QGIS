/***************************************************************************
                         qgspiechartplot.h
                         -----------------
    begin                : September 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPIECHARTPLOT_H
#define QGSPIECHARTPLOT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscolorramp.h"
#include "qgsfillsymbol.h"
#include "qgsnumericformat.h"
#include "qgsplot.h"

class QgsVectorLayerAbstractPlotDataGatherer;


/**
 * \brief A simple pie chart class.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsPieChartPlot : public Qgs2DPlot
{
  public:

    QgsPieChartPlot();
    ~QgsPieChartPlot() override = default;

    QString type() const override { return u"pie"_s; }

    void renderContent( QgsRenderContext &context, QgsPlotRenderContext &plotContext, const QRectF &plotArea, const QgsPlotData &plotData = QgsPlotData() ) override;

    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

    /**
     * Returns the fill symbol for the series with matching \a index.
     */
    QgsFillSymbol *fillSymbolAt( int index ) const;

    /**
     * Sets the fill \a symbol to use for the series with matching \a index.
     */
    void setFillSymbolAt( int index, QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the color ramp for the series with matching \a index.
     */
    QgsColorRamp *colorRampAt( int index ) const;

    /**
     * Sets the color \a ramp for the series with matching \a index.
     */
    void setColorRampAt( int index, QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Returns the fill symbols list count.
     */
    int fillSymbolCount() const { return mFillSymbols.size(); }

    /**
     * Returns the color ramps list count.
     */
    int colorRampCount() const { return mColorRamps.size(); }

    /**
     * Returns the text format used for the pie chart labels.
     *
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const { return mLabelTextFormat; }

    /**
     * Sets the text \a format used for the pie chart labels.
     *
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Returns the numeric format used for the pie chart labels.
     *
     * \see setNumericFormat()
     */
    QgsNumericFormat *numericFormat() const { return mNumericFormat.get(); }

    /**
     * Sets the numeric \a format used for the pie chart labels.
     *
     * Ownership of \a format is transferred to the plot.
     *
     * \see numericFormat()
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the pie chart label type.
     */
    Qgis::PieChartLabelType labelType() const { return mLabelType; }

    /**
     * Sets the pie chart label type.
     */
    void setLabelType( Qgis::PieChartLabelType type );

    //! Returns a new pie chart.
    static QgsPieChartPlot *create() SIP_FACTORY;

    //! Returns a new data gatherer for a given pie chart \a plot.
    static QgsVectorLayerAbstractPlotDataGatherer *createDataGatherer( QgsPlot *plot ) SIP_TRANSFERBACK;

  private:

    std::vector<std::unique_ptr<QgsFillSymbol>> mFillSymbols;
    std::vector<std::unique_ptr<QgsColorRamp>> mColorRamps;

    std::unique_ptr< QgsNumericFormat > mNumericFormat;

    QgsTextFormat mLabelTextFormat;
    Qgis::PieChartLabelType mLabelType = Qgis::PieChartLabelType::NoLabels;
};

#endif // QGSLINECHARTPLOT_H
