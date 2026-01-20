/***************************************************************************
                         qgsbarchartplot.h
                         -----------------
    begin                : June 2025
    copyright            : (C) 2025 by Mathieu
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
#ifndef QGSBARCHARTPLOT_H
#define QGSBARCHARTPLOT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfillsymbol.h"
#include "qgsplot.h"

class QgsVectorLayerAbstractPlotDataGatherer;


/**
 * \brief A simple bar chart class.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsBarChartPlot : public Qgs2DXyPlot
{
  public:

    QgsBarChartPlot();
    ~QgsBarChartPlot() override = default;

    QString type() const override { return u"bar"_s; }

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
     * Returns the fill symbols list count.
     */
    int fillSymbolCount() const { return mFillSymbols.size(); }

    //! Returns a new bar chart.
    static QgsBarChartPlot *create() SIP_FACTORY;

    //! Returns a new data gatherer for a given bar chart \a plot.
    static QgsVectorLayerAbstractPlotDataGatherer *createDataGatherer( QgsPlot *plot ) SIP_TRANSFERBACK;

  private:

    std::vector<std::unique_ptr<QgsFillSymbol>> mFillSymbols;
};

#endif // QGSBARCHARTPLOT_H
