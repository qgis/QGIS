/***************************************************************************
                         qgsplot.cpp
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

#include "qgschart.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

void QgsBarChart::renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData )
{
  const QList<QgsAbstractPlotSeries *> seriesList = plotData.series();
  if ( seriesList.isEmpty() )
  {
    return;
  }

  const QList<QVariant> categories = seriesList.at( 0 )->categories();
  if ( categories.isEmpty() )
  {
    return;
  }

  QgsExpressionContextScope *chartScope = new QgsExpressionContextScope( QStringLiteral( "chart" ) );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), chartScope );

  context.painter()->save();
  context.painter()->setClipRect( plotArea );

  const double xScale = plotArea.width() / ( xMaximum() - xMinimum() );
  const double yScale = plotArea.height() / ( yMaximum() - yMinimum() );
  const double categoriesWidth = plotArea.width() / categories.size();
  const double barsWidth = categoriesWidth / 2;
  const double barWidth = barsWidth / seriesList.size();
  int seriesIndex = 0;
  for ( const QgsAbstractPlotSeries *series : seriesList )
  {
    QgsFillSymbol *symbol = dynamic_cast<QgsFillSymbol *>( series->symbol() );
    if ( !symbol )
    {
      continue;
    }
    symbol->startRender( context );

    const double barStartAdjustement = -( barsWidth / 2 ) + barWidth * seriesIndex;
    if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
    {
      const QList<std::pair<QVariant, double>> data = xySeries->data();
      for ( const std::pair<QVariant, double> &pair : data )
      {
        double x, y;
        if ( xAxis().type() == Qgis::PlotAxisType::ValueType )
        {
          x = ( pair.first.toDouble() ) * xScale + barStartAdjustement;
        }
        else if ( xAxis().type() == Qgis::PlotAxisType::CategoryType )
        {
          x = ( categoriesWidth * categories.indexOf( pair.first ) ) + ( categoriesWidth / 2 ) + barStartAdjustement;
        }
        y = ( pair.second - yMinimum() ) * yScale;

        const double zero = ( 0.0 - yMinimum() ) * yScale;
        const QPoint topLeft( plotArea.left() + x,
                              plotArea.y() + plotArea.height() - y );
        const QPoint bottomRight( plotArea.left() + x + barWidth,
                                  plotArea.y() + plotArea.height() - zero );

        chartScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "chart_value" ), pair.second, true ) );
        symbol->renderPolygon( QPolygonF( QRectF( topLeft, bottomRight ) ), nullptr, nullptr, context );
      }
    }

    symbol->stopRender( context );
    seriesIndex++;
  }

  context.painter()->restore();
}

void QgsLineChart::renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData )
{
  const QList<QgsAbstractPlotSeries *> seriesList = plotData.series();
  if ( seriesList.isEmpty() )
  {
    return;
  }

  const QList<QVariant> categories = seriesList.at( 0 )->categories();
  if ( categories.isEmpty() )
  {
    return;
  }

  QgsExpressionContextScope *chartScope = new QgsExpressionContextScope( QStringLiteral( "chart" ) );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), chartScope );

  context.painter()->save();
  context.painter()->setClipRect( plotArea );

  const double xScale = plotArea.width() / ( xMaximum() - xMinimum() );
  const double yScale = plotArea.height() / ( yMaximum() - yMinimum() );
  const double categoriesWidth = plotArea.width() / categories.size();
  int seriesIndex = 0;
  for ( const QgsAbstractPlotSeries *series : seriesList )
  {
    QgsLineSymbol *symbol = dynamic_cast<QgsLineSymbol *>( series->symbol() );
    if ( !symbol )
    {
      continue;
    }
    symbol->startRender( context );

    if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
    {
      const QList<std::pair<QVariant, double>> data = xySeries->data();
      QVector<QPointF> points;
      QList<double> values;
      points.fill( QPointF(), xAxis().type() == Qgis::PlotAxisType::ValueType ? data.size() : categories.size() );
      int dataIndex = 0;
      for ( const std::pair<QVariant, double> &pair : data )
      {
        double x, y;
        if ( xAxis().type() == Qgis::PlotAxisType::ValueType )
        {
          x = ( pair.first.toDouble() ) * xScale;
        }
        else if ( xAxis().type() == Qgis::PlotAxisType::CategoryType )
        {
          x = ( categoriesWidth * categories.indexOf( pair.first ) ) + ( categoriesWidth / 2 );
        }
        y = ( pair.second - yMinimum() ) * yScale;

        values << pair.second;
        points.replace( xAxis().type() == Qgis::PlotAxisType::ValueType ? dataIndex : categories.indexOf( pair.first ), QPointF( plotArea.x() + x,
                        plotArea.y() + plotArea.height() - y ) );
        dataIndex++;
      }

      chartScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "chart_value" ), QVariant::fromValue( values ), true ) );
      symbol->renderPolyline( QPolygonF( points ), nullptr, context );
    }

    symbol->stopRender( context );
    seriesIndex++;
  }

  context.painter()->restore();
}
