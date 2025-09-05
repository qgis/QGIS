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

#include "qgspiechartplot.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"


QgsPieChartPlot::QgsPieChartPlot()
{
  setFillSymbolAt( 0, QgsPlotDefaultSettings::pieChartFillSymbol() );
  setColorRampAt( 0, QgsPlotDefaultSettings::pieChartColorRamp() );
}

void QgsPieChartPlot::renderContent( QgsRenderContext &context, QgsPlotRenderContext &, const QRectF &plotArea, const QgsPlotData &plotData )
{
  if ( mFillSymbols.empty() )
  {
    return;
  }

  const QList<QgsAbstractPlotSeries *> seriesList = plotData.series();
  if ( seriesList.isEmpty() || plotData.categories().isEmpty() )
  {
    return;
  }

  const QStringList categories = plotData.categories();
  QgsExpressionContextScope *chartScope = new QgsExpressionContextScope( QStringLiteral( "chart" ) );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), chartScope );

  context.painter()->save();
  context.painter()->setClipRect( plotArea );

  const bool pieStackHorizontal = plotArea.width() >= plotArea.height();
  const double pieStackCount = seriesList.size();
  double pieArea = 0;
  if ( pieStackHorizontal )
  {
    pieArea = plotArea.height() * pieStackCount > plotArea.width() ? plotArea.width() / pieStackCount : plotArea.height();
  }
  else
  {
    pieArea = plotArea.width() * pieStackCount > plotArea.height() ? plotArea.height() / pieStackCount : plotArea.width();
  }

  QMap<QString, QColor> categoriesColor;
  int seriesIndex = 0;
  for ( const QgsAbstractPlotSeries *series : seriesList )
  {
    QgsFillSymbol *symbol = fillSymbolAt( seriesIndex % mFillSymbols.size() );
    if ( !symbol )
    {
      continue;
    }
    const QColor symbolColor = symbol->color();
    symbol->startRender( context );

    QgsColorRamp *ramp = colorRampAt( seriesIndex % mColorRamps.size() );

    if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
    {
      const QList<std::pair<double, double>> data = xySeries->data();
      double yTotal = 0;
      for ( const std::pair<double, double> &pair : data )
      {
        if ( !categoriesColor.contains( categories[pair.first] ) )
        {
          if ( ramp )
          {
            categoriesColor[categories[pair.first]] = ramp->color( pair.first / ( categories.size() - 1 ) );
          }
          else
          {
            categoriesColor[categories[pair.first]] = symbolColor;
          }
        }

        yTotal += pair.second;
      }

      double ySum = 0;
      for ( const std::pair<double, double> &pair : data )
      {
        QPointF center;
        if ( pieStackHorizontal )
        {
          center = QPointF( ( plotArea.width() - pieArea * pieStackCount ) / 2 + pieArea * seriesIndex + pieArea / 2, plotArea.height() / 2 );
        }
        else
        {
          center = QPointF( plotArea.width() / 2, ( plotArea.height() - pieArea * pieStackCount ) / 2 + pieArea * seriesIndex + pieArea / 2 );
        }
        const double pieWidth = pieArea - QgsSymbolLayerUtils::estimateMaxSymbolBleed( symbol, context );
        QRectF boundingBox( center.x() - pieWidth / 2, center.y() - pieWidth / 2, pieWidth, pieWidth );

        QPainterPath path;
        path.moveTo( center );
        path.arcTo( boundingBox, ( ySum / yTotal * 360 ) + 90, pair.second / yTotal * 360 );

        chartScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "chart_value" ), pair.second, true ) );
        symbol->setColor( categoriesColor[categories[pair.first]] );
        symbol->renderPolygon( path.toFillPolygon(), nullptr, nullptr, context );

        ySum += pair.second;
      }
    }

    symbol->stopRender( context );
    symbol->setColor( symbolColor );
    seriesIndex++;
  }

  context.painter()->restore();
}

QgsFillSymbol *QgsPieChartPlot::fillSymbolAt( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mFillSymbols.size() ) )
  {
    return nullptr;
  }

  return mFillSymbols[index].get();
}

void QgsPieChartPlot::setFillSymbolAt( int index, QgsFillSymbol *symbol )
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

QgsColorRamp *QgsPieChartPlot::colorRampAt( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mColorRamps.size() ) )
  {
    return nullptr;
  }

  return mColorRamps[index].get();
}

void QgsPieChartPlot::setColorRampAt( int index, QgsColorRamp *ramp )
{
  if ( index < 0 )
  {
    return;
  }

  if ( index + 1 >= static_cast<int>( mColorRamps.size() ) )
  {
    mColorRamps.resize( index + 1 );
  }

  mColorRamps[index].reset( ramp );
}

bool QgsPieChartPlot::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Qgs2DPlot::writeXml( element, document, context );

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

  QDomElement colorRampsElement = document.createElement( QStringLiteral( "colorRamps" ) );
  for ( int i = 0; i < static_cast<int>( mColorRamps.size() ); i++ )
  {
    QDomElement colorRampElement = document.createElement( QStringLiteral( "colorRamp" ) );
    colorRampElement.setAttribute( QStringLiteral( "index" ), QString::number( i ) );
    if ( mColorRamps[i] )
    {
      colorRampElement.appendChild( QgsSymbolLayerUtils::saveColorRamp( QString(), mColorRamps[i].get(), document ) );
    }
    colorRampsElement.appendChild( colorRampElement );
  }
  element.appendChild( colorRampsElement );

  return true;
}

bool QgsPieChartPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DPlot::readXml( element, context );

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
        setFillSymbolAt( index, QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElement, context ).release() );
      }
      else
      {
        setFillSymbolAt( index, nullptr );
      }
    }
  }

  const QDomNodeList colorRampsList = element.firstChildElement( QStringLiteral( "colorRamps" ) ).childNodes();
  for ( int i = 0; i < colorRampsList.count(); i++ )
  {
    const QDomElement colorRampElement = colorRampsList.at( i ).toElement();
    const int index = colorRampElement.attribute( QStringLiteral( "index" ), QStringLiteral( "-1" ) ).toInt();
    if ( index >= 0 )
    {
      if ( colorRampElement.hasChildNodes() )
      {
        QDomElement rampElement = colorRampElement.firstChildElement( QStringLiteral( "colorramp" ) );
        setColorRampAt( index, QgsSymbolLayerUtils::loadColorRamp( rampElement ).release() );
      }
      else
      {
        setColorRampAt( index, nullptr );
      }
    }
  }

  return true;
}

QgsPieChartPlot *QgsPieChartPlot::create()
{
  return new QgsPieChartPlot();
}
