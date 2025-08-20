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
          double x = 0;
          if ( xAxis().type() == Qgis::PlotAxisType::ValueType )
          {
            x = ( pair.first - xMinimum() ) * xScale;
          }
          else if ( xAxis().type() == Qgis::PlotAxisType::CategoryType )
          {
            x = ( categoriesWidth * pair.first ) + ( categoriesWidth / 2 );
          }
          double y = ( pair.second - yMinimum() ) * yScale;

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

  QDomElement markerSymbolsElement = document.createElement( QStringLiteral( "markerSymbols" ) );
  for ( int i = 0; i < static_cast<int>( mMarkerSymbols.size() ); i++ )
  {
    QDomElement markerSymbolElement = document.createElement( QStringLiteral( "markerSymbol" ) );
    markerSymbolElement.setAttribute( QStringLiteral( "index" ), QString::number( i ) );
    if ( mMarkerSymbols[i] )
    {
      markerSymbolElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mMarkerSymbols[i].get(), document, context ) );
    }
    markerSymbolsElement.appendChild( markerSymbolElement );
  }
  element.appendChild( markerSymbolsElement );

  QDomElement lineSymbolsElement = document.createElement( QStringLiteral( "lineSymbols" ) );
  for ( int i = 0; i < static_cast<int>( mLineSymbols.size() ); i++ )
  {
    QDomElement lineSymbolElement = document.createElement( QStringLiteral( "lineSymbol" ) );
    lineSymbolElement.setAttribute( QStringLiteral( "index" ), QString::number( i ) );
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

  const QDomNodeList markerSymbolsList = element.firstChildElement( QStringLiteral( "markerSymbols" ) ).childNodes();
  for ( int i = 0; i < markerSymbolsList.count(); i++ )
  {
    const QDomElement markerSymbolElement = markerSymbolsList.at( i ).toElement();
    const int index = markerSymbolElement.attribute( QStringLiteral( "index" ), QStringLiteral( "-1" ) ).toInt();
    if ( index >= 0 )
    {
      if ( markerSymbolElement.hasChildNodes() )
      {
        const QDomElement symbolElement = markerSymbolElement.firstChildElement( QStringLiteral( "symbol" ) );
        setMarkerSymbol( index, QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setMarkerSymbol( index, nullptr );
      }
    }
  }

  const QDomNodeList lineSymbolsList = element.firstChildElement( QStringLiteral( "lineSymbols" ) ).childNodes();
  for ( int i = 0; i < lineSymbolsList.count(); i++ )
  {
    const QDomElement lineSymbolElement = lineSymbolsList.at( i ).toElement();
    const int index = lineSymbolElement.attribute( QStringLiteral( "index" ), QStringLiteral( "-1" ) ).toInt();
    if ( index >= 0 )
    {
      if ( lineSymbolElement.hasChildNodes() )
      {
        const QDomElement symbolElement = lineSymbolElement.firstChildElement( QStringLiteral( "symbol" ) );
        setLineSymbol( index, QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setLineSymbol( index, nullptr );
      }
    }
  }

  return true;
}

QgsLineChartPlot *QgsLineChartPlot::create()
{
  return new QgsLineChartPlot();
}
