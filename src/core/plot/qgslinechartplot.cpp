/***************************************************************************
                         qgslinechartplot.cpp
                         --------------------
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

#include "qgslinechartplot.h"

#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayerplotdatagatherer.h"

QgsLineChartPlot::QgsLineChartPlot()
{
  setMarkerSymbolAt( 0, QgsPlotDefaultSettings::lineChartMarkerSymbol() );
  setLineSymbolAt( 0, QgsPlotDefaultSettings::lineChartLineSymbol() );
}

void QgsLineChartPlot::renderContent( QgsRenderContext &context, QgsPlotRenderContext &, const QRectF &plotArea, const QgsPlotData &plotData )
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
  int seriesIndex = 0;
  for ( const QgsAbstractPlotSeries *series : seriesList )
  {
    QgsLineSymbol *lSymbol = !mLineSymbols.empty() ? lineSymbolAt( seriesIndex % mLineSymbols.size() ) : nullptr;
    QgsMarkerSymbol *mSymbol = !mMarkerSymbols.empty() ? markerSymbolAt( seriesIndex % mMarkerSymbols.size() ) : nullptr;
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
      points.fill( QPointF(), xAxis().type() == Qgis::PlotAxisType::Interval ? data.size() : categories.size() );
      int dataIndex = 0;
      for ( const std::pair<double, double> &pair : data )
      {
        if ( !std::isnan( pair.second ) )
        {
          double x = 0;
          switch ( xAxis().type() )
          {
            case Qgis::PlotAxisType::Categorical:
              if ( pair.first < 0 || pair.first >= categories.size() )
              {
                continue;
              }
              x = ( categoriesWidth * pair.first ) + ( categoriesWidth / 2 );
              break;
            case Qgis::PlotAxisType::Interval:
              x = ( pair.first - minX ) * xScale;
              break;
          }
          double y = ( pair.second - minY ) * yScale;

          const QPointF point( plotArea.x() + x, plotArea.y() + plotArea.height() - y );
          points.replace( xAxis().type() == Qgis::PlotAxisType::Interval ? dataIndex : pair.first, point );
        }
        dataIndex++;
      }

      if ( lSymbol )
      {
        chartScope->removeVariable( u"chart_value"_s );
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
            switch ( xAxis().type() )
            {
              case Qgis::PlotAxisType::Interval:
                value = data.at( pointIndex ).second;
                break;

              case Qgis::PlotAxisType::Categorical:
                bool found = false;
                for ( const std::pair<double, double> &pair : data )
                {
                  if ( pair.first == pointIndex )
                  {
                    found = true;
                    value = pair.second;
                    chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_category"_s, categories[pair.first], true ) );
                    break;
                  }
                }
                if ( !found )
                {
                  continue;
                }
                break;
            }

            chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_value"_s, value, true ) );
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

QgsMarkerSymbol *QgsLineChartPlot::markerSymbolAt( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mMarkerSymbols.size() ) )
  {
    return nullptr;
  }

  return mMarkerSymbols[index].get();
}

void QgsLineChartPlot::setMarkerSymbolAt( int index, QgsMarkerSymbol *symbol )
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

QgsLineSymbol *QgsLineChartPlot::lineSymbolAt( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mLineSymbols.size() ) )
  {
    return nullptr;
  }

  return mLineSymbols[index].get();
}

void QgsLineChartPlot::setLineSymbolAt( int index, QgsLineSymbol *symbol )
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

  QDomElement markerSymbolsElement = document.createElement( u"markerSymbols"_s );
  for ( int i = 0; i < static_cast<int>( mMarkerSymbols.size() ); i++ )
  {
    QDomElement markerSymbolElement = document.createElement( u"markerSymbol"_s );
    markerSymbolElement.setAttribute( u"index"_s, QString::number( i ) );
    if ( mMarkerSymbols[i] )
    {
      markerSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mMarkerSymbols[i].get(), document, context ) );
    }
    markerSymbolsElement.appendChild( markerSymbolElement );
  }
  element.appendChild( markerSymbolsElement );

  QDomElement lineSymbolsElement = document.createElement( u"lineSymbols"_s );
  for ( int i = 0; i < static_cast<int>( mLineSymbols.size() ); i++ )
  {
    QDomElement lineSymbolElement = document.createElement( u"lineSymbol"_s );
    lineSymbolElement.setAttribute( u"index"_s, QString::number( i ) );
    if ( mLineSymbols[i] )
    {
      lineSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mLineSymbols[i].get(), document, context ) );
    }
    lineSymbolsElement.appendChild( lineSymbolElement );
  }
  element.appendChild( lineSymbolsElement );

  return true;
}

bool QgsLineChartPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DXyPlot::readXml( element, context );

  const QDomNodeList markerSymbolsList = element.firstChildElement( u"markerSymbols"_s ).childNodes();
  for ( int i = 0; i < markerSymbolsList.count(); i++ )
  {
    const QDomElement markerSymbolElement = markerSymbolsList.at( i ).toElement();
    const int index = markerSymbolElement.attribute( u"index"_s, u"-1"_s ).toInt();
    if ( index >= 0 )
    {
      if ( markerSymbolElement.hasChildNodes() )
      {
        const QDomElement symbolElement = markerSymbolElement.firstChildElement( u"symbol"_s );
        setMarkerSymbolAt( index, QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setMarkerSymbolAt( index, nullptr );
      }
    }
  }

  const QDomNodeList lineSymbolsList = element.firstChildElement( u"lineSymbols"_s ).childNodes();
  for ( int i = 0; i < lineSymbolsList.count(); i++ )
  {
    const QDomElement lineSymbolElement = lineSymbolsList.at( i ).toElement();
    const int index = lineSymbolElement.attribute( u"index"_s, u"-1"_s ).toInt();
    if ( index >= 0 )
    {
      if ( lineSymbolElement.hasChildNodes() )
      {
        const QDomElement symbolElement = lineSymbolElement.firstChildElement( u"symbol"_s );
        setLineSymbolAt( index, QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setLineSymbolAt( index, nullptr );
      }
    }
  }

  return true;
}

QgsLineChartPlot *QgsLineChartPlot::create()
{
  return new QgsLineChartPlot();
}

QgsVectorLayerAbstractPlotDataGatherer *QgsLineChartPlot::createDataGatherer( QgsPlot *plot )
{
  QgsLineChartPlot *chart = dynamic_cast<QgsLineChartPlot *>( plot );
  if ( !chart )
  {
    return nullptr;
  }

  return new QgsVectorLayerXyPlotDataGatherer( chart->xAxis().type() );
}
