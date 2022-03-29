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
#include "qgsplot.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgstextrenderer.h"
#include "qgsbasicnumericformat.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgsnumericformatregistry.h"

QgsPlot::~QgsPlot() = default;

bool QgsPlot::writeXml( QDomElement &, QDomDocument &, QgsReadWriteContext & )
{
  return true;
}

bool QgsPlot::readXml( const QDomElement &, QgsReadWriteContext & )
{
  return true;
}

//
// Qgs2DPlot
//

Qgs2DPlot::Qgs2DPlot()
  : mMargins( 2, 2, 2, 2 )
{
  // setup default style

  mXAxisNumericFormat = std::make_unique< QgsBasicNumericFormat >();
  mYAxisNumericFormat = std::make_unique< QgsBasicNumericFormat >();

  std::unique_ptr< QgsSimpleFillSymbolLayer > chartFill = std::make_unique< QgsSimpleFillSymbolLayer >( QColor( 255, 255, 255 ) );
  mChartBackgroundSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { chartFill.release() } ) );

  std::unique_ptr< QgsSimpleLineSymbolLayer > chartBorder = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20 ), 0.1 );
  mChartBorderSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { chartBorder.release() } ) );

  std::unique_ptr< QgsSimpleLineSymbolLayer > gridMinor = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20, 50 ), 0.1 );
  mXGridMinorSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { gridMinor->clone() } ) );
  mYGridMinorSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { gridMinor->clone() } ) );

  std::unique_ptr< QgsSimpleLineSymbolLayer > gridMajor = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20, 150 ), 0.1 );
  mXGridMajorSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { gridMajor->clone() } ) );
  mYGridMajorSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { gridMajor->clone() } ) );
}

bool Qgs2DPlot::writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context )
{
  element.setAttribute( QStringLiteral( "minX" ), qgsDoubleToString( mMinX ) );
  element.setAttribute( QStringLiteral( "maxX" ), qgsDoubleToString( mMaxX ) );
  element.setAttribute( QStringLiteral( "minY" ), qgsDoubleToString( mMinY ) );
  element.setAttribute( QStringLiteral( "maxY" ), qgsDoubleToString( mMaxY ) );

  element.setAttribute( QStringLiteral( "gridIntervalMinorX" ), qgsDoubleToString( mGridIntervalMinorX ) );
  element.setAttribute( QStringLiteral( "gridIntervalMajorX" ), qgsDoubleToString( mGridIntervalMajorX ) );
  element.setAttribute( QStringLiteral( "gridIntervalMinorY" ), qgsDoubleToString( mGridIntervalMinorY ) );
  element.setAttribute( QStringLiteral( "gridIntervalMajorY" ), qgsDoubleToString( mGridIntervalMajorY ) );

  element.setAttribute( QStringLiteral( "labelIntervalX" ), qgsDoubleToString( mLabelIntervalX ) );
  element.setAttribute( QStringLiteral( "labelIntervalY" ), qgsDoubleToString( mLabelIntervalY ) );

  QDomElement xAxisFormatElement = document.createElement( QStringLiteral( "xAxisNumericFormat" ) );
  mXAxisNumericFormat->writeXml( xAxisFormatElement, document, context );
  element.appendChild( xAxisFormatElement );
  QDomElement yAxisFormatElement = document.createElement( QStringLiteral( "yAxisNumericFormat" ) );
  mYAxisNumericFormat->writeXml( yAxisFormatElement, document, context );
  element.appendChild( yAxisFormatElement );

  QDomElement backgroundElement = document.createElement( QStringLiteral( "backgroundSymbol" ) );
  backgroundElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mChartBackgroundSymbol.get(), document, context ) );
  element.appendChild( backgroundElement );
  QDomElement borderElement = document.createElement( QStringLiteral( "borderSymbol" ) );
  borderElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mChartBorderSymbol.get(), document, context ) );
  element.appendChild( borderElement );
  QDomElement xGridMajorElement = document.createElement( QStringLiteral( "xGridMajorSymbol" ) );
  xGridMajorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mXGridMajorSymbol.get(), document, context ) );
  element.appendChild( xGridMajorElement );
  QDomElement xGridMinorElement = document.createElement( QStringLiteral( "xGridMinorSymbol" ) );
  xGridMinorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mXGridMinorSymbol.get(), document, context ) );
  element.appendChild( xGridMinorElement );
  QDomElement yGridMajorElement = document.createElement( QStringLiteral( "yGridMajorSymbol" ) );
  yGridMajorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mYGridMajorSymbol.get(), document, context ) );
  element.appendChild( yGridMajorElement );
  QDomElement yGridMinorElement = document.createElement( QStringLiteral( "yGridMinorSymbol" ) );
  yGridMinorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mYGridMinorSymbol.get(), document, context ) );
  element.appendChild( yGridMinorElement );

  QDomElement xAxisTextFormatElement = document.createElement( QStringLiteral( "xAxisTextFormat" ) );
  xAxisTextFormatElement.appendChild( mXAxisLabelTextFormat.writeXml( document, context ) );
  element.appendChild( xAxisTextFormatElement );
  QDomElement yAxisTextFormatElement = document.createElement( QStringLiteral( "yAxisTextFormat" ) );
  yAxisTextFormatElement.appendChild( mYAxisLabelTextFormat.writeXml( document, context ) );
  element.appendChild( yAxisTextFormatElement );

  QDomElement marginsElement = document.createElement( QStringLiteral( "margins" ) );
  marginsElement.setAttribute( QStringLiteral( "left" ), qgsDoubleToString( mMargins.left() ) );
  marginsElement.setAttribute( QStringLiteral( "right" ), qgsDoubleToString( mMargins.right() ) );
  marginsElement.setAttribute( QStringLiteral( "top" ), qgsDoubleToString( mMargins.top() ) );
  marginsElement.setAttribute( QStringLiteral( "bottom" ), qgsDoubleToString( mMargins.bottom() ) );
  element.appendChild( marginsElement );

  return true;
}

bool Qgs2DPlot::readXml( const QDomElement &element, QgsReadWriteContext &context )
{
  mMinX = element.attribute( QStringLiteral( "minX" ) ).toDouble();
  mMaxX = element.attribute( QStringLiteral( "maxX" ) ).toDouble();
  mMinY = element.attribute( QStringLiteral( "minY" ) ).toDouble();
  mMaxY = element.attribute( QStringLiteral( "maxY" ) ).toDouble();

  mGridIntervalMinorX = element.attribute( QStringLiteral( "gridIntervalMinorX" ) ).toDouble();
  mGridIntervalMajorX = element.attribute( QStringLiteral( "gridIntervalMajorX" ) ).toDouble();
  mGridIntervalMinorY = element.attribute( QStringLiteral( "gridIntervalMinorY" ) ).toDouble();
  mGridIntervalMajorY = element.attribute( QStringLiteral( "gridIntervalMajorY" ) ).toDouble();

  mLabelIntervalX = element.attribute( QStringLiteral( "labelIntervalX" ) ).toDouble();
  mLabelIntervalY = element.attribute( QStringLiteral( "labelIntervalY" ) ).toDouble();

  const QDomElement xAxisFormatElement = element.firstChildElement( QStringLiteral( "xAxisNumericFormat" ) );
  mXAxisNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( xAxisFormatElement, context ) );
  const QDomElement yAxisFormatElement = element.firstChildElement( QStringLiteral( "yAxisNumericFormat" ) );
  mYAxisNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( yAxisFormatElement, context ) );

  const QDomElement backgroundElement = element.firstChildElement( QStringLiteral( "backgroundSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mChartBackgroundSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundElement, context ) );
  const QDomElement borderElement = element.firstChildElement( QStringLiteral( "borderSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mChartBorderSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( borderElement, context ) );
  const QDomElement xGridMajorElement = element.firstChildElement( QStringLiteral( "xGridMajorSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mXGridMajorSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( xGridMajorElement, context ) );
  const QDomElement xGridMinorElement = element.firstChildElement( QStringLiteral( "xGridMinorSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mXGridMinorSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( xGridMinorElement, context ) );
  const QDomElement yGridMajorElement = element.firstChildElement( QStringLiteral( "yGridMajorSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mYGridMajorSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( yGridMajorElement, context ) );
  const QDomElement yGridMinorElement = element.firstChildElement( QStringLiteral( "yGridMinorSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mYGridMinorSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( yGridMinorElement, context ) );

  const QDomElement xAxisTextFormatElement = element.firstChildElement( QStringLiteral( "xAxisTextFormat" ) );
  mXAxisLabelTextFormat.readXml( xAxisTextFormatElement, context );
  const QDomElement yAxisTextFormatElement = element.firstChildElement( QStringLiteral( "yAxisTextFormat" ) );
  mYAxisLabelTextFormat.readXml( yAxisTextFormatElement, context );

  const QDomElement marginsElement = element.firstChildElement( QStringLiteral( "margins" ) );
  mMargins.setLeft( marginsElement.attribute( QStringLiteral( "left" ) ).toDouble() );
  mMargins.setRight( marginsElement.attribute( QStringLiteral( "right" ) ).toDouble() );
  mMargins.setTop( marginsElement.attribute( QStringLiteral( "top" ) ).toDouble() );
  mMargins.setBottom( marginsElement.attribute( QStringLiteral( "bottom" ) ).toDouble() );

  return true;
}

void Qgs2DPlot::render( QgsRenderContext &context )
{
  mChartBackgroundSymbol->startRender( context );
  mChartBorderSymbol->startRender( context );
  mXGridMinorSymbol->startRender( context );
  mYGridMinorSymbol->startRender( context );
  mXGridMajorSymbol->startRender( context );
  mYGridMajorSymbol->startRender( context );

  const double firstMinorXGrid = std::ceil( mMinX / mGridIntervalMinorX ) * mGridIntervalMinorX;
  const double firstMajorXGrid = std::ceil( mMinX / mGridIntervalMajorX ) * mGridIntervalMajorX;
  const double firstMinorYGrid = std::ceil( mMinY / mGridIntervalMinorY ) * mGridIntervalMinorY;
  const double firstMajorYGrid = std::ceil( mMinY / mGridIntervalMajorY ) * mGridIntervalMajorY;
  const double firstXLabel = std::ceil( mMinX / mLabelIntervalX ) * mLabelIntervalX;
  const double firstYLabel = std::ceil( mMinY / mLabelIntervalY ) * mLabelIntervalY;

  const QRectF plotArea = interiorPlotArea( context );

  QgsNumericFormatContext numericContext;

  // calculate text metrics
  double maxYAxisLabelWidth = 0;
  for ( double currentY = firstYLabel; currentY <= mMaxY; currentY += mLabelIntervalY )
  {
    const QString text = mYAxisNumericFormat->formatDouble( currentY, numericContext );
    maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, QgsTextRenderer::textWidth( context, mYAxisLabelTextFormat, { text } ) );
  }

  const double chartAreaLeft = plotArea.left();
  const double chartAreaRight = plotArea.right();
  const double chartAreaTop = plotArea.top();
  const double chartAreaBottom = plotArea.bottom();

  // chart background
  mChartBackgroundSymbol->renderPolygon( QPolygonF(
  {
    QPointF( chartAreaLeft, chartAreaTop ),
    QPointF( chartAreaRight, chartAreaTop ),
    QPointF( chartAreaRight, chartAreaBottom ),
    QPointF( chartAreaLeft, chartAreaBottom ),
    QPointF( chartAreaLeft, chartAreaTop )
  } ), nullptr, nullptr, context );

  const double xScale = ( chartAreaRight - chartAreaLeft ) / ( mMaxX - mMinX );
  const double yScale = ( chartAreaBottom - chartAreaTop ) / ( mMaxY - mMinY );


  // grid lines

  // x
  double nextMajorXGrid = firstMajorXGrid;
  for ( double currentX = firstMinorXGrid; currentX <= mMaxX; currentX += mGridIntervalMinorX )
  {
    bool isMinor = true;
    if ( qgsDoubleNear( currentX, nextMajorXGrid ) )
    {
      isMinor = false;
      nextMajorXGrid += mGridIntervalMajorX;
    }

    if ( isMinor )
    {
      mXGridMinorSymbol->renderPolyline( QPolygonF(
                                           QVector<QPointF>
      {
        QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, chartAreaBottom ),
        QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, chartAreaTop )
      } ), nullptr, context );
    }
    else
    {
      mXGridMajorSymbol->renderPolyline( QPolygonF(
                                           QVector<QPointF>
      {
        QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, chartAreaBottom ),
        QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, chartAreaTop )
      } ), nullptr, context );
    }
  }

  // y
  double nextMajorYGrid = firstMajorYGrid;
  for ( double currentY = firstMinorYGrid; currentY <= mMaxY; currentY += mGridIntervalMinorY )
  {
    bool isMinor = true;
    if ( qgsDoubleNear( currentY, nextMajorYGrid ) )
    {
      isMinor = false;
      nextMajorYGrid += mGridIntervalMajorY;
    }

    if ( isMinor )
    {
      mYGridMinorSymbol->renderPolyline( QPolygonF(
                                           QVector<QPointF>
      {
        QPointF( chartAreaLeft, chartAreaBottom - ( currentY - mMinY ) * yScale ),
        QPointF( chartAreaRight, chartAreaBottom - ( currentY - mMinY ) * yScale )
      } ), nullptr, context );
    }
    else
    {
      mYGridMajorSymbol->renderPolyline( QPolygonF(
                                           QVector<QPointF>
      {
        QPointF( chartAreaLeft, chartAreaBottom - ( currentY - mMinY ) * yScale ),
        QPointF( chartAreaRight, chartAreaBottom - ( currentY - mMinY ) * yScale )
      } ), nullptr, context );
    }
  }


  // axis labels

  // x
  for ( double currentX = firstXLabel; currentX <= mMaxX; currentX += mLabelIntervalX )
  {
    const QString text = mXAxisNumericFormat->formatDouble( currentX, numericContext );
    QgsTextRenderer::drawText( QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, mSize.height() - context.convertToPainterUnits( mMargins.bottom(), QgsUnitTypes::RenderMillimeters ) ),
                               0, QgsTextRenderer::AlignCenter, { text }, context, mXAxisLabelTextFormat );
  }

  // y
  for ( double currentY = firstYLabel; currentY <= mMaxY; currentY += mLabelIntervalY )
  {
    const QString text = mYAxisNumericFormat->formatDouble( currentY, numericContext );
    const double height = QgsTextRenderer::textHeight( context, mYAxisLabelTextFormat, { text } );
    QgsTextRenderer::drawText( QPointF(
                                 maxYAxisLabelWidth + context.convertToPainterUnits( mMargins.left(), QgsUnitTypes::RenderMillimeters ),
                                 chartAreaBottom - ( currentY - mMinY ) * yScale + height / 2 ),
                               0, QgsTextRenderer::AlignRight, { text }, context, mYAxisLabelTextFormat, false );
  }

  // give subclasses a chance to draw their content
  renderContent( context, plotArea );

  // border
  mChartBorderSymbol->renderPolygon( QPolygonF(
  {
    QPointF( chartAreaLeft, chartAreaTop ),
    QPointF( chartAreaRight, chartAreaTop ),
    QPointF( chartAreaRight, chartAreaBottom ),
    QPointF( chartAreaLeft, chartAreaBottom ),
    QPointF( chartAreaLeft, chartAreaTop )
  } ), nullptr, nullptr, context );

  mChartBackgroundSymbol->stopRender( context );
  mChartBorderSymbol->stopRender( context );
  mXGridMinorSymbol->stopRender( context );
  mYGridMinorSymbol->stopRender( context );
  mXGridMajorSymbol->stopRender( context );
  mYGridMajorSymbol->stopRender( context );
}

void Qgs2DPlot::renderContent( QgsRenderContext &, const QRectF & )
{

}

Qgs2DPlot::~Qgs2DPlot() = default;

QSizeF Qgs2DPlot::size() const
{
  return mSize;
}

void Qgs2DPlot::setSize( QSizeF size )
{
  mSize = size;
}

QRectF Qgs2DPlot::interiorPlotArea( const QgsRenderContext &context ) const
{
  const double firstMinorYGrid = std::ceil( mMinY / mGridIntervalMinorY ) * mGridIntervalMinorY;
  const double firstXLabel = std::ceil( mMinX / mLabelIntervalX ) * mLabelIntervalX;

  QgsNumericFormatContext numericContext;

  // calculate text metrics
  double maxXAxisLabelHeight = 0;
  for ( double currentX = firstXLabel; currentX <= mMaxX; currentX += mLabelIntervalX )
  {
    const QString text = mXAxisNumericFormat->formatDouble( currentX, numericContext );
    maxXAxisLabelHeight = std::max( maxXAxisLabelHeight, QgsTextRenderer::textHeight( context, mXAxisLabelTextFormat, { text } ) );
  }
  double maxYAxisLabelWidth = 0;
  for ( double currentY = firstMinorYGrid; currentY <= mMaxY; currentY += mGridIntervalMinorY )
  {
    const QString text = mYAxisNumericFormat->formatDouble( currentY, numericContext );
    maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, QgsTextRenderer::textWidth( context, mYAxisLabelTextFormat, { text } ) );
  }

  const double leftTextSize = maxYAxisLabelWidth + context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  const double rightTextSize = 0;
  const double bottomTextSize = maxXAxisLabelHeight + context.convertToPainterUnits( 0.5, QgsUnitTypes::RenderMillimeters );
  const double topTextSize = 0;

  const double leftMargin = context.convertToPainterUnits( mMargins.left(), QgsUnitTypes::RenderMillimeters ) + leftTextSize;
  const double rightMargin = context.convertToPainterUnits( mMargins.right(), QgsUnitTypes::RenderMillimeters ) + rightTextSize;
  const double topMargin = context.convertToPainterUnits( mMargins.top(), QgsUnitTypes::RenderMillimeters ) + topTextSize;
  const double bottomMargin = context.convertToPainterUnits( mMargins.bottom(), QgsUnitTypes::RenderMillimeters ) + bottomTextSize;

  return QRectF( leftMargin, topMargin, mSize.width() - rightMargin - leftMargin, mSize.height() - bottomMargin - topMargin );
}

QgsFillSymbol *Qgs2DPlot::chartBackgroundSymbol()
{
  return mChartBackgroundSymbol.get();
}

void Qgs2DPlot::setChartBackgroundSymbol( QgsFillSymbol *symbol )
{
  mChartBackgroundSymbol.reset( symbol );
}

QgsFillSymbol *Qgs2DPlot::chartBorderSymbol()
{
  return mChartBorderSymbol.get();
}

void Qgs2DPlot::setChartBorderSymbol( QgsFillSymbol *symbol )
{
  mChartBorderSymbol.reset( symbol );
}

QgsLineSymbol *Qgs2DPlot::xGridMajorSymbol()
{
  return mXGridMajorSymbol.get();
}

void Qgs2DPlot::setXGridMajorSymbol( QgsLineSymbol *symbol )
{
  mXGridMajorSymbol.reset( symbol );
}

QgsLineSymbol *Qgs2DPlot::xGridMinorSymbol()
{
  return mXGridMinorSymbol.get();
}

void Qgs2DPlot::setXGridMinorSymbol( QgsLineSymbol *symbol )
{
  mXGridMinorSymbol.reset( symbol );
}

QgsLineSymbol *Qgs2DPlot::yGridMajorSymbol()
{
  return mYGridMajorSymbol.get();
}

void Qgs2DPlot::setYGridMajorSymbol( QgsLineSymbol *symbol )
{
  mYGridMajorSymbol.reset( symbol );
}

QgsLineSymbol *Qgs2DPlot::yGridMinorSymbol()
{
  return mYGridMinorSymbol.get();
}

void Qgs2DPlot::setYGridMinorSymbol( QgsLineSymbol *symbol )
{
  mYGridMinorSymbol.reset( symbol );
}

QgsTextFormat Qgs2DPlot::xAxisTextFormat() const
{
  return mXAxisLabelTextFormat;
}

void Qgs2DPlot::setXAxisTextFormat( const QgsTextFormat &format )
{
  mXAxisLabelTextFormat = format;
}

QgsTextFormat Qgs2DPlot::yAxisTextFormat() const
{
  return mYAxisLabelTextFormat;
}

void Qgs2DPlot::setYAxisTextFormat( const QgsTextFormat &format )
{
  mYAxisLabelTextFormat = format;
}

const QgsMargins &Qgs2DPlot::margins() const
{
  return mMargins;
}

void Qgs2DPlot::setMargins( const QgsMargins &margins )
{
  mMargins = margins;
}

QgsNumericFormat *Qgs2DPlot::xAxisNumericFormat()
{
  return mXAxisNumericFormat.get();
}

void Qgs2DPlot::setXAxisNumericFormat( QgsNumericFormat *format )
{
  mXAxisNumericFormat.reset( format );
}

QgsNumericFormat *Qgs2DPlot::yAxisNumericFormat()
{
  return mYAxisNumericFormat.get();
}

void Qgs2DPlot::setYAxisNumericFormat( QgsNumericFormat *format )
{
  mYAxisNumericFormat.reset( format );
}

