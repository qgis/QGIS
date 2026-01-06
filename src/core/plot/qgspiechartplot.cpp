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

#include "qgsapplication.h"
#include "qgscolorrampimpl.h"
#include "qgsexpressioncontextutils.h"
#include "qgsnumericformatregistry.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer.h"
#include "qgsvectorlayerplotdatagatherer.h"

QgsPieChartPlot::QgsPieChartPlot()
{
  setFillSymbolAt( 0, QgsPlotDefaultSettings::pieChartFillSymbol() );
  setColorRampAt( 0, QgsPlotDefaultSettings::pieChartColorRamp() );
  mNumericFormat.reset( QgsPlotDefaultSettings::pieChartNumericFormat() );
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
  double maxLabelHeight = 0;
  switch ( mLabelType )
  {
    case Qgis::PieChartLabelType::Categories:
    {
      for ( const QString &category : categories )
      {
        maxLabelHeight = std::max( maxLabelHeight, QgsTextRenderer::textHeight( context, mLabelTextFormat, { category } ) );
      }
      break;
    }

    case Qgis::PieChartLabelType::Values:
    {
      QgsNumericFormatContext numericContext;
      QString text;

      for ( const QgsAbstractPlotSeries *series : seriesList )
      {
        if ( const QgsXyPlotSeries *xySeries = dynamic_cast<const QgsXyPlotSeries *>( series ) )
        {
          const QList<std::pair<double, double>> data = xySeries->data();
          for ( const std::pair<double, double> &pair : data )
          {
            if ( mNumericFormat )
            {
              text = mNumericFormat->formatDouble( pair.second, numericContext );
            }
            else
            {
              text = QString::number( pair.second );
            }
            maxLabelHeight = std::max( maxLabelHeight, QgsTextRenderer::textHeight( context, mLabelTextFormat, { text } ) );
          }
        }
      }
      break;
    }

    case Qgis::PieChartLabelType::NoLabels:
      break;
  }

  QgsExpressionContextScope *chartScope = new QgsExpressionContextScope( u"chart"_s );
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

  QgsNumericFormatContext numericContext;
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
    const double pieWidth = pieArea - QgsSymbolLayerUtils::estimateMaxSymbolBleed( symbol, context ) - maxLabelHeight * 3;

    QgsColorRamp *ramp = colorRampAt( seriesIndex % mColorRamps.size() );
    if ( QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp ) )
    {
      //ramp is a random colors ramp, so inform it of the total number of required colors
      //this allows the ramp to pregenerate a set of visually distinctive colors
      randomRamp->setTotalColorCount( categories.size() );
    }

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
          center = QPointF( plotArea.x() + ( ( plotArea.width() - pieArea * pieStackCount ) / 2 + pieArea * seriesIndex + pieArea / 2 ), plotArea.y() + plotArea.height() / 2 );
        }
        else
        {
          center = QPointF( plotArea.x() + plotArea.width() / 2, plotArea.y() + ( ( plotArea.height() - pieArea * pieStackCount ) / 2 + pieArea * seriesIndex + pieArea / 2 ) );
        }
        QRectF boundingBox( center.x() - pieWidth / 2, center.y() - pieWidth / 2, pieWidth, pieWidth );

        const double degreesStart = ( ySum / yTotal * 360 ) - 90; // adjust angle so we start on top
        const double degreesForward = pair.second / yTotal * 360;

        QPainterPath path;
        path.moveTo( center );
        path.arcTo( boundingBox, -degreesStart, -degreesForward );

        chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_category"_s, categories[pair.first], true ) );
        chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_value"_s, pair.second, true ) );
        symbol->setColor( categoriesColor[categories[pair.first]] );
        symbol->renderPolygon( path.toFillPolygon(), nullptr, nullptr, context );

        ySum += pair.second;
      }

      if ( mLabelType != Qgis::PieChartLabelType::NoLabels )
      {
        QString text;
        ySum = 0;
        for ( const std::pair<double, double> &pair : data )
        {
          QPointF center;
          if ( pieStackHorizontal )
          {
            center = QPointF( plotArea.x() + ( ( plotArea.width() - pieArea * pieStackCount ) / 2 + pieArea * seriesIndex + pieArea / 2 ), plotArea.y() + plotArea.height() / 2 );
          }
          else
          {
            center = QPointF( plotArea.x() + plotArea.width() / 2, plotArea.y() + ( ( plotArea.height() - pieArea * pieStackCount ) / 2 + pieArea * seriesIndex + pieArea / 2 ) );
          }

          const double degreesStart = ( ySum / yTotal * 360 ) - 90; // adjust angle so we start on top
          const double degreesForward = pair.second / yTotal * 360;
          const double degreesMid = ( degreesStart + ( degreesForward / 2 ) );

          const double labelX = ( ( pieWidth + maxLabelHeight ) / 2 ) * std::cos( degreesMid * M_PI / 180 ) + center.x();
          const double labelY = ( ( pieWidth + maxLabelHeight ) / 2 ) * std::sin( degreesMid * M_PI / 180 ) + center.y();
          const double labelYAdjustment = degreesMid > 0 && degreesMid <= 180 ? maxLabelHeight / 2 : 0;

          Qgis::TextHorizontalAlignment horizontalAlignment = Qgis::TextHorizontalAlignment::Left;
          if ( degreesMid < -85 || ( degreesMid > 85 && degreesMid <= 95 ) || degreesMid > 265 )
          {
            horizontalAlignment = Qgis::TextHorizontalAlignment::Center;
          }
          else if ( degreesMid > 95 && degreesMid <= 265 )
          {
            horizontalAlignment = Qgis::TextHorizontalAlignment::Right;
          }

          switch ( mLabelType )
          {
            case Qgis::PieChartLabelType::Categories:
              text = categories[pair.first];
              break;

            case Qgis::PieChartLabelType::Values:
              if ( mNumericFormat )
              {
                text = mNumericFormat->formatDouble( pair.second, numericContext );
              }
              else
              {
                text = QString::number( pair.second );
              }
              break;

            case Qgis::PieChartLabelType::NoLabels:
              break;
          }

          chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_category"_s, categories[pair.first], true ) );
          chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_value"_s, pair.second, true ) );
          QgsTextRenderer::drawText( QPointF( labelX, labelY + labelYAdjustment ), 0, horizontalAlignment, { text }, context, mLabelTextFormat );

          ySum += pair.second;
        }
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

  QDomElement colorRampsElement = document.createElement( u"colorRamps"_s );
  for ( int i = 0; i < static_cast<int>( mColorRamps.size() ); i++ )
  {
    QDomElement colorRampElement = document.createElement( u"colorRamp"_s );
    colorRampElement.setAttribute( u"index"_s, QString::number( i ) );
    if ( mColorRamps[i] )
    {
      colorRampElement.appendChild( QgsSymbolLayerUtils::saveColorRamp( QString(), mColorRamps[i].get(), document ) );
    }
    colorRampsElement.appendChild( colorRampElement );
  }
  element.appendChild( colorRampsElement );

  QDomElement textFormatElement = document.createElement( u"textFormat"_s );
  textFormatElement.appendChild( mLabelTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElement );

  if ( mNumericFormat )
  {
    QDomElement numericFormatElement = document.createElement( u"numericFormat"_s );
    mNumericFormat->writeXml( numericFormatElement, document, context );
    element.appendChild( numericFormatElement );
  }

  element.setAttribute( u"pieChartLabelType"_s, qgsEnumValueToKey( mLabelType ) );

  return true;
}

bool QgsPieChartPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DPlot::readXml( element, context );

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

  const QDomNodeList colorRampsList = element.firstChildElement( u"colorRamps"_s ).childNodes();
  for ( int i = 0; i < colorRampsList.count(); i++ )
  {
    const QDomElement colorRampElement = colorRampsList.at( i ).toElement();
    const int index = colorRampElement.attribute( u"index"_s, u"-1"_s ).toInt();
    if ( index >= 0 )
    {
      if ( colorRampElement.hasChildNodes() )
      {
        QDomElement rampElement = colorRampElement.firstChildElement( u"colorramp"_s );
        setColorRampAt( index, QgsSymbolLayerUtils::loadColorRamp( rampElement ).release() );
      }
      else
      {
        setColorRampAt( index, nullptr );
      }
    }
  }

  const QDomElement textFormatElement = element.firstChildElement( u"textFormat"_s );
  mLabelTextFormat.readXml( textFormatElement, context );

  const QDomElement numericFormatElement = element.firstChildElement( u"numericFormat"_s );
  if ( !numericFormatElement.isNull() )
  {
    mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElement, context ) );
  }
  else
  {
    mNumericFormat.reset();
  }

  mLabelType = qgsEnumKeyToValue( element.attribute( u"pieChartLabelType"_s ), Qgis::PieChartLabelType::NoLabels );

  return true;
}

QgsPieChartPlot *QgsPieChartPlot::create()
{
  return new QgsPieChartPlot();
}

QgsVectorLayerAbstractPlotDataGatherer *QgsPieChartPlot::createDataGatherer( QgsPlot *plot )
{
  QgsPieChartPlot *chart = dynamic_cast<QgsPieChartPlot *>( plot );
  if ( !chart )
  {
    return nullptr;
  }

  return new QgsVectorLayerXyPlotDataGatherer( Qgis::PlotAxisType::Categorical );
}

void QgsPieChartPlot::setTextFormat( const QgsTextFormat &format )
{
  mLabelTextFormat = format;
}

void QgsPieChartPlot::setNumericFormat( QgsNumericFormat *format )
{
  mNumericFormat.reset( format );
}

void QgsPieChartPlot::setLabelType( Qgis::PieChartLabelType type )
{
  mLabelType = type;
}
