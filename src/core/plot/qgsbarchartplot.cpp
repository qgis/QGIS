/***************************************************************************
                         qgsbarchartplot.cpp
                         -------------------
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

#include "qgsbarchartplot.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"


void QgsBarChartPlot::renderContent( QgsRenderContext &context, QgsPlotRenderContext &, const QRectF &plotArea, const QgsPlotData &plotData )
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
  switch ( xAxis().type() )
  {
    case Qgis::PlotAxisType::CategoryType:
      if ( categories.isEmpty() )
      {
        return;
      }
      break;

    case Qgis::PlotAxisType::ValueType:
      break;
  }

  QgsExpressionContextScope *chartScope = new QgsExpressionContextScope( QStringLiteral( "chart" ) );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), chartScope );

  context.painter()->save();
  context.painter()->setClipRect( plotArea );

  const double xScale = plotArea.width() / ( xMaximum() - xMinimum() );
  const double yScale = plotArea.height() / ( yMaximum() - yMinimum() );
  const double categoriesWidth = plotArea.width() / categories.size();
  const double valuesWidth = plotArea.width() * ( xAxis().gridIntervalMinor() / ( xMaximum() - xMinimum() ) );
  const double barsWidth = xAxis().type() == Qgis::PlotAxisType::CategoryType ? categoriesWidth / 2 : valuesWidth / 2;
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
        double x = 0;
        switch ( xAxis().type() )
        {
          case Qgis::PlotAxisType::CategoryType:
            if ( pair.first < 0 || pair.first >= categories.size() )
            {
              continue;
            }
            x = ( categoriesWidth * pair.first ) + ( categoriesWidth / 2 ) + barStartAdjustement;
            break;

          case Qgis::PlotAxisType::ValueType:
            x = ( pair.first - xMinimum() ) * xScale + barStartAdjustement;
            break;
        }

        double y = ( pair.second - yMinimum() ) * yScale;

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

  QDomElement fillSymbolsElement = document.createElement( QStringLiteral( "fillSymbols" ) );
  for ( int i = 0; i < static_cast<int>( mFillSymbols.size() ); i++ )
  {
    QDomElement fillSymbolElement = document.createElement( QStringLiteral( "fillSymbol" ) );
    fillSymbolElement.setAttribute( QStringLiteral( "index" ), QString::number( i ) );
    if ( mFillSymbols[i] )
    {
      fillSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mFillSymbols[i].get(), document, context ) );
    }
    fillSymbolsElement.appendChild( fillSymbolElement );
  }
  element.appendChild( fillSymbolsElement );

  return true;
}

bool QgsBarChartPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DXyPlot::readXml( element, context );

  const QDomNodeList fillSymbolsList = element.firstChildElement( QStringLiteral( "fillSymbols" ) ).childNodes();
  for ( int i = 0; i < fillSymbolsList.count(); i++ )
  {
    const QDomElement fillSymbolElement = fillSymbolsList.at( i ).toElement();
    const int index = fillSymbolElement.attribute( QStringLiteral( "index" ), QStringLiteral( "-1" ) ).toInt();
    if ( index >= 0 )
    {
      if ( fillSymbolElement.hasChildNodes() )
      {
        const QDomElement symbolElement = fillSymbolElement.firstChildElement( QStringLiteral( "symbol" ) );
        setFillSymbol( index, QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setFillSymbol( index, nullptr );
      }
    }
  }

  return true;
}

QgsBarChartPlot *QgsBarChartPlot::create()
{
  return new QgsBarChartPlot();
}
