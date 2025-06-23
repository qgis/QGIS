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
#include "qgsmarkersymbol.h"

void QgsBarChart::renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData )
{
  const QList<QgsAbstractPlotSeries *> seriesList = plotData.series();
  if ( seriesList.isEmpty() )
  {
    return;
  }

  const QStringList categories = plotData.categories();
  if ( xAxis().type() == Qgis::PlotAxisType::CategoryType && categories.isEmpty() )
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
    if ( !series->fillSymbol() )
    {
      continue;
    }
    series->fillSymbol()->startRender( context );

    const double barStartAdjustement = -( barsWidth / 2 ) + barWidth * seriesIndex;
    if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
    {
      const QList<std::pair<double, double>> data = xySeries->data();
      for ( const std::pair<double, double> &pair : data )
      {
        if ( xAxis().type() == Qgis::PlotAxisType::CategoryType && ( pair.first < 0 || pair.first >= categories.size() ) )
        {
          continue;
        }

        double x, y;
        if ( xAxis().type() == Qgis::PlotAxisType::ValueType )
        {
          x = ( pair.first - xMinimum() ) * xScale + barStartAdjustement;
        }
        else if ( xAxis().type() == Qgis::PlotAxisType::CategoryType )
        {
          x = ( categoriesWidth * pair.first ) + ( categoriesWidth / 2 ) + barStartAdjustement;
        }
        y = ( pair.second - yMinimum() ) * yScale;

        const double zero = ( 0.0 - yMinimum() ) * yScale;
        const QPoint topLeft( plotArea.left() + x,
                              plotArea.y() + plotArea.height() - y );
        const QPoint bottomRight( plotArea.left() + x + barWidth,
                                  plotArea.y() + plotArea.height() - zero );

        chartScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "chart_value" ), pair.second, true ) );
        series->fillSymbol()->renderPolygon( QPolygonF( QRectF( topLeft, bottomRight ) ), nullptr, nullptr, context );
      }
    }

    series->fillSymbol()->stopRender( context );
    seriesIndex++;
  }

  context.painter()->restore();
}

bool QgsBarChart::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Qgs2DXyPlot::writeXml( element, document, context );
  return true;
}

bool QgsBarChart::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DXyPlot::readXml( element, context );
  return true;
}

QgsBarChart *QgsBarChart::create()
{
  return new QgsBarChart();
}

//
// QgsLineChart
//

void QgsLineChart::renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData )
{
  const QList<QgsAbstractPlotSeries *> seriesList = plotData.series();
  if ( seriesList.isEmpty() )
  {
    return;
  }

  const QStringList categories = plotData.categories();
  if ( xAxis().type() == Qgis::PlotAxisType::CategoryType && categories.isEmpty() )
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
    if ( !series->lineSymbol() && !series->markerSymbol() )
    {
      continue;
    }

    if ( series->lineSymbol() )
    {
      series->lineSymbol()->startRender( context );
    }

    if ( series->markerSymbol() )
    {
      series->markerSymbol()->startRender( context );
    }

    if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
    {
      const QList<std::pair<double, double>> data = xySeries->data();
      QVector<QPointF> points;
      points.fill( QPointF(), xAxis().type() == Qgis::PlotAxisType::ValueType ? data.size() : categories.size() );
      int dataIndex = 0;
      for ( const std::pair<double, double> &pair : data )
      {
        if ( xAxis().type() == Qgis::PlotAxisType::CategoryType && ( pair.first < 0 || pair.first >= categories.size() ) )
        {
          continue;
        }

        if ( !std::isnan( pair.second ) )
        {
          double x, y;
          if ( xAxis().type() == Qgis::PlotAxisType::ValueType )
          {
            x = ( pair.first - xMinimum() ) * xScale;
          }
          else if ( xAxis().type() == Qgis::PlotAxisType::CategoryType )
          {
            x = ( categoriesWidth * pair.first ) + ( categoriesWidth / 2 );
          }
          y = ( pair.second - yMinimum() ) * yScale;

          const QPointF point( plotArea.x() + x, plotArea.y() + plotArea.height() - y );
          points.replace( xAxis().type() == Qgis::PlotAxisType::ValueType ? dataIndex : pair.first, point );
        }
        dataIndex++;
      }

      if ( series->lineSymbol() )
      {
        chartScope->removeVariable( QStringLiteral( "chart_value" ) );
        QVector<QPointF> line;
        for ( const QPointF &point : points )
        {
          if ( !point.isNull() )
          {
            line << point;
          }
          else
          {
            if ( !line.isEmpty() )
            {
              series->lineSymbol()->renderPolyline( QPolygonF( line ), nullptr, context );
              line.clear();
            }
          }
        }
        if ( !line.isEmpty() )
        {
          series->lineSymbol()->renderPolyline( QPolygonF( line ), nullptr, context );
        }
      }
      if ( series->markerSymbol() )
      {
        int pointIndex = 0;
        for ( const QPointF &point : points )
        {
          if ( !point.isNull() )
          {
            if ( series->markerSymbol() )
            {
              double value = 0;
              if ( xAxis().type() == Qgis::PlotAxisType::ValueType )
              {
                value = data.at( pointIndex ).second;
              }
              else
              {
                for ( const std::pair<double, double> &pair : data )
                {
                  if ( pair.first == pointIndex )
                  {
                    value = pair.second;
                    break;
                  }
                }
              }

              chartScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "chart_value" ), value, true ) );
              series->markerSymbol()->renderPoint( point, nullptr, context );
            }
          }
          pointIndex++;
        }
      }
    }

    if ( series->lineSymbol() )
    {
      series->lineSymbol()->stopRender( context );
    }
    if ( series->markerSymbol() )
    {
      series->markerSymbol()->stopRender( context );
    }

    seriesIndex++;
  }

  context.painter()->restore();
}

bool QgsLineChart::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Qgs2DXyPlot::writeXml( element, document, context );
  return true;
}

bool QgsLineChart::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DXyPlot::readXml( element, context );
  return true;
}

QgsLineChart *QgsLineChart::create()
{
  return new QgsLineChart();
}
