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
#include "qgsvectorlayerplotdatagatherer.h"

QgsBarChartPlot::QgsBarChartPlot()
{
  setFillSymbolAt( 0, QgsPlotDefaultSettings::barChartFillSymbol() );
}

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
    case Qgis::PlotAxisType::Categorical:
      if ( categories.isEmpty() )
      {
        return;
      }
      break;

    case Qgis::PlotAxisType::Interval:
      break;
  }

  QgsExpressionContextScope *chartScope = new QgsExpressionContextScope( u"chart"_s );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), chartScope );

  context.painter()->save();
  context.painter()->setClipRect( plotArea );

  double minX = xMinimum();
  double maxX = xMaximum();
  double minY = yMinimum();
  double maxY = yMaximum();
  double majorIntervalX = xAxis().gridIntervalMajor();
  double minorIntervalX = xAxis().gridIntervalMinor();
  double labelIntervalX = xAxis().labelInterval();
  double majorIntervalY = yAxis().gridIntervalMajor();
  double minorIntervalY = yAxis().gridIntervalMinor();
  double labelIntervalY = yAxis().labelInterval();
  Qgs2DXyPlot::applyDataDefinedProperties( context, minX, maxX, minY, maxY, majorIntervalX, minorIntervalX, labelIntervalX, majorIntervalY, minorIntervalY, labelIntervalY );

  const double xScale = plotArea.width() / ( maxX - minX );
  const double yScale = plotArea.height() / ( maxY - minY );
  const double categoriesWidth = plotArea.width() / categories.size();
  const double valuesWidth = plotArea.width() * ( minorIntervalX / ( maxX - minX ) );
  const double barsWidth = xAxis().type() == Qgis::PlotAxisType::Categorical ? categoriesWidth / 2 : valuesWidth / 2;
  const double barWidth = barsWidth / seriesList.size();
  int seriesIndex = 0;
  for ( const QgsAbstractPlotSeries *series : seriesList )
  {
    QgsFillSymbol *symbol = fillSymbolAt( seriesIndex % mFillSymbols.size() );
    if ( !symbol )
    {
      continue;
    }
    symbol->startRender( context );

    const double barStartAdjustment = -( barsWidth / 2 ) + barWidth * seriesIndex;
    if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
    {
      const QList<std::pair<double, double>> data = xySeries->data();
      for ( const std::pair<double, double> &pair : data )
      {
        double x = 0;
        switch ( xAxis().type() )
        {
          case Qgis::PlotAxisType::Categorical:
            if ( pair.first < 0 || pair.first >= categories.size() )
            {
              continue;
            }
            x = ( categoriesWidth * pair.first ) + ( categoriesWidth / 2 ) + barStartAdjustment;
            chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_category"_s, categories[pair.first], true ) );
            break;

          case Qgis::PlotAxisType::Interval:
            x = ( pair.first - minX ) * xScale + barStartAdjustment;
            break;
        }

        double y = ( pair.second - minY ) * yScale;

        const double zero = ( 0.0 - minY ) * yScale;
        const QPoint topLeft( plotArea.left() + x,
                              plotArea.y() + plotArea.height() - y );
        const QPoint bottomRight( plotArea.left() + x + barWidth,
                                  plotArea.y() + plotArea.height() - zero );

        chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_value"_s, pair.second, true ) );
        symbol->renderPolygon( QPolygonF( QRectF( topLeft, bottomRight ) ), nullptr, nullptr, context );
      }
    }

    symbol->stopRender( context );
    seriesIndex++;
  }

  context.painter()->restore();
}

QgsFillSymbol *QgsBarChartPlot::fillSymbolAt( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mFillSymbols.size() ) )
  {
    return nullptr;
  }

  return mFillSymbols[index].get();
}

void QgsBarChartPlot::setFillSymbolAt( int index, QgsFillSymbol *symbol )
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

  QDomElement fillSymbolsElement = document.createElement( u"fillSymbols"_s );
  for ( int i = 0; i < static_cast<int>( mFillSymbols.size() ); i++ )
  {
    QDomElement fillSymbolElement = document.createElement( u"fillSymbol"_s );
    fillSymbolElement.setAttribute( u"index"_s, QString::number( i ) );
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

  const QDomNodeList fillSymbolsList = element.firstChildElement( u"fillSymbols"_s ).childNodes();
  for ( int i = 0; i < fillSymbolsList.count(); i++ )
  {
    const QDomElement fillSymbolElement = fillSymbolsList.at( i ).toElement();
    const int index = fillSymbolElement.attribute( u"index"_s, u"-1"_s ).toInt();
    if ( index >= 0 )
    {
      if ( fillSymbolElement.hasChildNodes() )
      {
        const QDomElement symbolElement = fillSymbolElement.firstChildElement( u"symbol"_s );
        setFillSymbolAt( index, QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setFillSymbolAt( index, nullptr );
      }
    }
  }

  return true;
}

QgsBarChartPlot *QgsBarChartPlot::create()
{
  return new QgsBarChartPlot();
}

QgsVectorLayerAbstractPlotDataGatherer *QgsBarChartPlot::createDataGatherer( QgsPlot *plot )
{
  QgsBarChartPlot *chart = dynamic_cast<QgsBarChartPlot *>( plot );
  if ( !chart )
  {
    return nullptr;
  }

  return new QgsVectorLayerXyPlotDataGatherer( chart->xAxis().type() );
}
