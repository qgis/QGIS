/***************************************************************************
                         qgschartplot.cpp
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

#include "qgschartplot.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

void QgsBarChartPlot::renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData )
{
  if ( mFillSymbols.empty() )
  {
    return;
  }

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
    QgsFillSymbol *symbol = fillSymbol( seriesIndex % mFillSymbols.size() );
    if ( !symbol )
    {
      continue;
    }
    symbol->startRender( context );

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
        symbol->renderPolygon( QPolygonF( QRectF( topLeft, bottomRight ) ), nullptr, nullptr, context );
      }
    }

    symbol->stopRender( context );
    seriesIndex++;
  }

  context.painter()->restore();
}

QgsFillSymbol *QgsBarChartPlot::fillSymbol( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mFillSymbols.size() ) )
  {
    return nullptr;
  }

  return mFillSymbols[index].get();
}

void QgsBarChartPlot::setFillSymbol( int index, QgsFillSymbol *symbol )
{
  if ( index < 0 )
  {
    return;
  }

  if ( index + 1 >= static_cast<int>( mFillSymbols.size() ) )
  {
    mFillSymbols.resize( index + 1 );
  }

  mFillSymbols[index].reset( symbol );
}

bool QgsBarChartPlot::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Qgs2DXyPlot::writeXml( element, document, context );
  return true;
}

bool QgsBarChartPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DXyPlot::readXml( element, context );
  return true;
}

QgsBarChartPlot *QgsBarChartPlot::create()
{
  return new QgsBarChartPlot();
}

//
// QgsLineChartPlot
//

void QgsLineChartPlot::renderContent( QgsRenderContext &context, const QRectF &plotArea, const QgsPlotData &plotData )
{
  if ( mLineSymbols.empty() && mMarkerSymbols.empty() )
  {
    return;
  }

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
    QgsLineSymbol *lSymbol = !mLineSymbols.empty() ? lineSymbol( seriesIndex % mLineSymbols.size() ) : nullptr;
    QgsMarkerSymbol *mSymbol = !mMarkerSymbols.empty() ? markerSymbol( seriesIndex % mMarkerSymbols.size() ) : nullptr;
    if ( !lSymbol && !mSymbol )
    {
      continue;
    }

    if ( lSymbol )
    {
      lSymbol->startRender( context );
    }

    if ( mSymbol )
    {
      mSymbol->startRender( context );
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

      if ( lSymbol )
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
              lSymbol->renderPolyline( QPolygonF( line ), nullptr, context );
              line.clear();
            }
          }
        }
        if ( !line.isEmpty() )
        {
          lSymbol->renderPolyline( QPolygonF( line ), nullptr, context );
        }
      }
      if ( mSymbol )
      {
        int pointIndex = 0;
        for ( const QPointF &point : points )
        {
          if ( !point.isNull() )
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
            mSymbol->renderPoint( point, nullptr, context );
          }
          pointIndex++;
        }
      }
    }

    if ( lSymbol )
    {
      lSymbol->stopRender( context );
    }
    if ( mSymbol )
    {
      mSymbol->stopRender( context );
    }

    seriesIndex++;
  }

  context.painter()->restore();
}

QgsMarkerSymbol *QgsLineChartPlot::markerSymbol( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mMarkerSymbols.size() ) )
  {
    return nullptr;
  }

  return mMarkerSymbols[index].get();
}

void QgsLineChartPlot::setMarkerSymbol( int index, QgsMarkerSymbol *symbol )
{
  if ( index < 0 )
  {
    return;
  }

  if ( index + 1 >= static_cast<int>( mMarkerSymbols.size() ) )
  {
    mMarkerSymbols.resize( index + 1 );
  }

  mMarkerSymbols[index].reset( symbol );
}

QgsLineSymbol *QgsLineChartPlot::lineSymbol( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mLineSymbols.size() ) )
  {
    return nullptr;
  }

  return mLineSymbols[index].get();
}

void QgsLineChartPlot::setLineSymbol( int index, QgsLineSymbol *symbol )
{
  if ( index < 0 )
  {
    return;
  }

  if ( index + 1 >= static_cast<int>( mLineSymbols.size() ) )
  {
    mLineSymbols.resize( index + 1 );
  }

  mLineSymbols[index].reset( symbol );
}

bool QgsLineChartPlot::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Qgs2DXyPlot::writeXml( element, document, context );
  return true;
}

bool QgsLineChartPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DXyPlot::readXml( element, context );
  return true;
}

QgsLineChartPlot *QgsLineChartPlot::create()
{
  return new QgsLineChartPlot();
}
