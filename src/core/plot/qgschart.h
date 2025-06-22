/***************************************************************************
                         qgschart.h
                         ---------------
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
#ifndef QGSCHART_H
#define QGSCHART_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsplot.h"


/**
 * \brief A simple bar chart class.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsBarChart : public Qgs2DXyPlot
{
  public:

    QgsBarChart() = default;
    ~QgsBarChart() = default;

    void renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData = QgsPlotData() ) override;

  private:


};


/**
 * \brief A simple line chart class.
 *
 * \warning This class is not considered stable API, and may change in future!
 *
 * \ingroup core
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsLineChart : public Qgs2DXyPlot
{
  public:

    QgsLineChart() = default;
    ~QgsLineChart() = default;

    void renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData = QgsPlotData() ) override;

  private:


};

#endif
