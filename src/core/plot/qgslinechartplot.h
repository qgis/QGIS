/***************************************************************************
                         qgslinechartplot.h
                         ------------------
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
#ifndef QGSLINECHARTPLOT_H
#define QGSLINECHARTPLOT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsplot.h"

class QgsVectorLayerAbstractPlotDataGatherer;


/**
 * \brief A simple line chart class.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsLineChartPlot : public Qgs2DXyPlot
{
  public:

    QgsLineChartPlot();
    ~QgsLineChartPlot() override = default;

    QString type() const override { return u"line"_s; }

    void renderContent( QgsRenderContext &context, QgsPlotRenderContext &plotContext, const QRectF &plotArea, const QgsPlotData &plotData = QgsPlotData() ) override;

    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

    /**
     * Returns the marker symbol for the series with matching \a index.
     */
    QgsMarkerSymbol *markerSymbolAt( int index ) const;

    /**
     * Sets the fill \a symbol to use for the series with matching \a index.
     */
    void setMarkerSymbolAt( int index, QgsMarkerSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbols list count.
     */
    int markerSymbolCount() const { return mMarkerSymbols.size(); }

    /**
     * Returns the line symbol for the series with matching \a index.
     */
    QgsLineSymbol *lineSymbolAt( int index ) const;

    /**
     * Sets the line \a symbol to use for the series with matching \a index.
     */
    void setLineSymbolAt( int index, QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbols list count.
     */
    int lineSymbolCount() const { return mLineSymbols.size(); }

    //! Returns a new line chart.
    static QgsLineChartPlot *create() SIP_FACTORY;

    //! Returns a new data gatherer for a given line chart \a plot.
    static QgsVectorLayerAbstractPlotDataGatherer *createDataGatherer( QgsPlot *plot ) SIP_TRANSFERBACK;

  private:

    std::vector<std::unique_ptr<QgsMarkerSymbol>> mMarkerSymbols;
    std::vector<std::unique_ptr<QgsLineSymbol>> mLineSymbols;
};

#endif // QGSLINECHARTPLOT_H
