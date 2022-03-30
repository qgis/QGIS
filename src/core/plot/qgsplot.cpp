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
#include "qgsexpressioncontextutils.h"

QgsPlot::~QgsPlot() = default;

bool QgsPlot::writeXml( QDomElement &, QDomDocument &, QgsReadWriteContext & )
{
  return true;
}

bool QgsPlot::readXml( const QDomElement &, QgsReadWriteContext & )
{
  return true;
}


// QgsPlotAxis

QgsPlotAxis::QgsPlotAxis()
{
  // setup default style

  mNumericFormat = std::make_unique< QgsBasicNumericFormat >();

  std::unique_ptr< QgsSimpleLineSymbolLayer > gridMinor = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20, 50 ), 0.1 );
  gridMinor->setPenCapStyle( Qt::FlatCap );
  mGridMinorSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { gridMinor.release() } ) );

  std::unique_ptr< QgsSimpleLineSymbolLayer > gridMajor = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20, 150 ), 0.1 );
  gridMajor->setPenCapStyle( Qt::FlatCap );
  mGridMajorSymbol = std::make_unique< QgsLineSymbol>( QgsSymbolLayerList( { gridMajor.release() } ) );
}

QgsPlotAxis::~QgsPlotAxis() = default;

bool QgsPlotAxis::writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context )
{
  element.setAttribute( QStringLiteral( "gridIntervalMinor" ), qgsDoubleToString( mGridIntervalMinor ) );
  element.setAttribute( QStringLiteral( "gridIntervalMajor" ), qgsDoubleToString( mGridIntervalMajor ) );
  element.setAttribute( QStringLiteral( "labelInterval" ), qgsDoubleToString( mLabelInterval ) );

  QDomElement numericFormatElement = document.createElement( QStringLiteral( "numericFormat" ) );
  mNumericFormat->writeXml( numericFormatElement, document, context );
  element.appendChild( numericFormatElement );

  QDomElement gridMajorElement = document.createElement( QStringLiteral( "gridMajorSymbol" ) );
  gridMajorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mGridMajorSymbol.get(), document, context ) );
  element.appendChild( gridMajorElement );
  QDomElement gridMinorElement = document.createElement( QStringLiteral( "gridMinorSymbol" ) );
  gridMinorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mGridMinorSymbol.get(), document, context ) );
  element.appendChild( gridMinorElement );

  QDomElement textFormatElement = document.createElement( QStringLiteral( "textFormat" ) );
  textFormatElement.appendChild( mLabelTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElement );

  return true;
}

bool QgsPlotAxis::readXml( const QDomElement &element, QgsReadWriteContext &context )
{
  mGridIntervalMinor = element.attribute( QStringLiteral( "gridIntervalMinor" ) ).toDouble();
  mGridIntervalMajor = element.attribute( QStringLiteral( "gridIntervalMajor" ) ).toDouble();
  mLabelInterval = element.attribute( QStringLiteral( "labelInterval" ) ).toDouble();

  const QDomElement numericFormatElement = element.firstChildElement( QStringLiteral( "numericFormat" ) );
  mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElement, context ) );

  const QDomElement gridMajorElement = element.firstChildElement( QStringLiteral( "gridMajorSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mGridMajorSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( gridMajorElement, context ) );
  const QDomElement gridMinorElement = element.firstChildElement( QStringLiteral( "gridMinorSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mGridMinorSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( gridMinorElement, context ) );

  const QDomElement textFormatElement = element.firstChildElement( QStringLiteral( "textFormat" ) );
  mLabelTextFormat.readXml( textFormatElement, context );

  return true;
}

QgsNumericFormat *QgsPlotAxis::numericFormat() const
{
  return mNumericFormat.get();
}

void QgsPlotAxis::setNumericFormat( QgsNumericFormat *format )
{
  mNumericFormat.reset( format );
}

QgsLineSymbol *QgsPlotAxis::gridMajorSymbol()
{
  return mGridMajorSymbol.get();
}

void QgsPlotAxis::setGridMajorSymbol( QgsLineSymbol *symbol )
{
  mGridMajorSymbol.reset( symbol );
}

QgsLineSymbol *QgsPlotAxis::gridMinorSymbol()
{
  return mGridMinorSymbol.get();
}

void QgsPlotAxis::setGridMinorSymbol( QgsLineSymbol *symbol )
{
  mGridMinorSymbol.reset( symbol );
}

QgsTextFormat QgsPlotAxis::textFormat() const
{
  return mLabelTextFormat;
}

void QgsPlotAxis::setTextFormat( const QgsTextFormat &format )
{
  mLabelTextFormat = format;
}


//
// Qgs2DPlot
//

Qgs2DPlot::Qgs2DPlot()
  : mMargins( 2, 2, 2, 2 )
{
  // setup default style
  std::unique_ptr< QgsSimpleFillSymbolLayer > chartFill = std::make_unique< QgsSimpleFillSymbolLayer >( QColor( 255, 255, 255 ) );
  mChartBackgroundSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { chartFill.release() } ) );

  std::unique_ptr< QgsSimpleLineSymbolLayer > chartBorder = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20 ), 0.1 );
  mChartBorderSymbol = std::make_unique< QgsFillSymbol>( QgsSymbolLayerList( { chartBorder.release() } ) );
}

bool Qgs2DPlot::writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context )
{
  QgsPlot::writeXml( element, document, context );

  element.setAttribute( QStringLiteral( "minX" ), qgsDoubleToString( mMinX ) );
  element.setAttribute( QStringLiteral( "maxX" ), qgsDoubleToString( mMaxX ) );
  element.setAttribute( QStringLiteral( "minY" ), qgsDoubleToString( mMinY ) );
  element.setAttribute( QStringLiteral( "maxY" ), qgsDoubleToString( mMaxY ) );

  QDomElement xAxisElement = document.createElement( QStringLiteral( "xAxis" ) );
  mXAxis.writeXml( xAxisElement, document, context );
  element.appendChild( xAxisElement );
  QDomElement yAxisElement = document.createElement( QStringLiteral( "yAxis" ) );
  mYAxis.writeXml( yAxisElement, document, context );
  element.appendChild( yAxisElement );

  QDomElement backgroundElement = document.createElement( QStringLiteral( "backgroundSymbol" ) );
  backgroundElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mChartBackgroundSymbol.get(), document, context ) );
  element.appendChild( backgroundElement );
  QDomElement borderElement = document.createElement( QStringLiteral( "borderSymbol" ) );
  borderElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mChartBorderSymbol.get(), document, context ) );
  element.appendChild( borderElement );

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
  QgsPlot::readXml( element, context );

  mMinX = element.attribute( QStringLiteral( "minX" ) ).toDouble();
  mMaxX = element.attribute( QStringLiteral( "maxX" ) ).toDouble();
  mMinY = element.attribute( QStringLiteral( "minY" ) ).toDouble();
  mMaxY = element.attribute( QStringLiteral( "maxY" ) ).toDouble();

  const QDomElement xAxisElement = element.firstChildElement( QStringLiteral( "xAxis" ) );
  mXAxis.readXml( xAxisElement, context );
  const QDomElement yAxisElement = element.firstChildElement( QStringLiteral( "yAxis" ) );
  mYAxis.readXml( yAxisElement, context );

  const QDomElement backgroundElement = element.firstChildElement( QStringLiteral( "backgroundSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mChartBackgroundSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundElement, context ) );
  const QDomElement borderElement = element.firstChildElement( QStringLiteral( "borderSymbol" ) ).firstChildElement( QStringLiteral( "symbol" ) );
  mChartBorderSymbol.reset( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( borderElement, context ) );

  const QDomElement marginsElement = element.firstChildElement( QStringLiteral( "margins" ) );
  mMargins.setLeft( marginsElement.attribute( QStringLiteral( "left" ) ).toDouble() );
  mMargins.setRight( marginsElement.attribute( QStringLiteral( "right" ) ).toDouble() );
  mMargins.setTop( marginsElement.attribute( QStringLiteral( "top" ) ).toDouble() );
  mMargins.setBottom( marginsElement.attribute( QStringLiteral( "bottom" ) ).toDouble() );

  return true;
}

void Qgs2DPlot::render( QgsRenderContext &context )
{
  QgsExpressionContextScope *plotScope = new QgsExpressionContextScope( QStringLiteral( "plot" ) );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), plotScope );

  mChartBackgroundSymbol->startRender( context );
  mChartBorderSymbol->startRender( context );
  mXAxis.gridMinorSymbol()->startRender( context );
  mYAxis.gridMinorSymbol()->startRender( context );
  mXAxis.gridMajorSymbol()->startRender( context );
  mYAxis.gridMajorSymbol()->startRender( context );

  const double firstMinorXGrid = std::ceil( mMinX / mXAxis.gridIntervalMinor() ) * mXAxis.gridIntervalMinor();
  const double firstMajorXGrid = std::ceil( mMinX / mXAxis.gridIntervalMajor() ) * mXAxis.gridIntervalMajor();
  const double firstMinorYGrid = std::ceil( mMinY / mYAxis.gridIntervalMinor() ) * mYAxis.gridIntervalMinor();
  const double firstMajorYGrid = std::ceil( mMinY / mYAxis.gridIntervalMajor() ) * mYAxis.gridIntervalMajor();
  const double firstXLabel = std::ceil( mMinX / mXAxis.labelInterval() ) * mXAxis.labelInterval();
  const double firstYLabel = std::ceil( mMinY / mYAxis.labelInterval() ) * mYAxis.labelInterval();

  const QRectF plotArea = interiorPlotArea( context );

  QgsNumericFormatContext numericContext;

  // calculate text metrics
  double maxYAxisLabelWidth = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  for ( double currentY = firstYLabel; currentY <= mMaxY; currentY += mYAxis.labelInterval() )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentY, true ) );
    const QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
    maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, QgsTextRenderer::textWidth( context, mYAxis.textFormat(), { text } ) );
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

  const double xTolerance = mXAxis.gridIntervalMinor() / 100000;
  const double yTolerance = mYAxis.gridIntervalMinor() / 100000;

  // grid lines

  // x
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "x" ), true ) );
  double nextMajorXGrid = firstMajorXGrid;
  for ( double currentX = firstMinorXGrid; currentX <= mMaxX; currentX += mXAxis.gridIntervalMinor() )
  {
    bool isMinor = true;
    if ( qgsDoubleNear( currentX, nextMajorXGrid, xTolerance ) )
    {
      isMinor = false;
      nextMajorXGrid += mXAxis.gridIntervalMajor();
    }

    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentX, true ) );

    QgsLineSymbol *currentGridSymbol = isMinor ? mXAxis.gridMinorSymbol() : mXAxis.gridMajorSymbol();
    currentGridSymbol->renderPolyline( QPolygonF(
                                         QVector<QPointF>
    {
      QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, chartAreaBottom ),
      QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, chartAreaTop )
    } ), nullptr, context );
  }

  // y
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  double nextMajorYGrid = firstMajorYGrid;
  for ( double currentY = firstMinorYGrid; currentY <= mMaxY; currentY += mYAxis.gridIntervalMinor() )
  {
    bool isMinor = true;
    if ( qgsDoubleNear( currentY, nextMajorYGrid, yTolerance ) )
    {
      isMinor = false;
      nextMajorYGrid += mYAxis.gridIntervalMajor();
    }

    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentY, true ) );

    QgsLineSymbol *currentGridSymbol = isMinor ? mYAxis.gridMinorSymbol() : mYAxis.gridMajorSymbol();
    currentGridSymbol->renderPolyline( QPolygonF(
                                         QVector<QPointF>
    {
      QPointF( chartAreaLeft, chartAreaBottom - ( currentY - mMinY ) * yScale ),
      QPointF( chartAreaRight, chartAreaBottom - ( currentY - mMinY ) * yScale )
    } ), nullptr, context );
  }

  // axis labels

  // x
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "x" ), true ) );
  for ( double currentX = firstXLabel; currentX <= mMaxX || qgsDoubleNear( currentX, mMaxX, xTolerance ); currentX += mXAxis.labelInterval() )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentX, true ) );
    const QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
    QgsTextRenderer::drawText( QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, mSize.height() - context.convertToPainterUnits( mMargins.bottom(), QgsUnitTypes::RenderMillimeters ) ),
                               0, QgsTextRenderer::AlignCenter, { text }, context, mXAxis.textFormat() );
  }

  // y
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  for ( double currentY = firstYLabel; currentY <= mMaxY || qgsDoubleNear( currentY, mMaxY, yTolerance ); currentY += mYAxis.labelInterval() )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentY, true ) );
    const QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
    const double height = QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { text } );
    QgsTextRenderer::drawText( QPointF(
                                 maxYAxisLabelWidth + context.convertToPainterUnits( mMargins.left(), QgsUnitTypes::RenderMillimeters ),
                                 chartAreaBottom - ( currentY - mMinY ) * yScale + height / 2 ),
                               0, QgsTextRenderer::AlignRight, { text }, context, mYAxis.textFormat(), false );
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
  mXAxis.gridMinorSymbol()->stopRender( context );
  mYAxis.gridMinorSymbol()->stopRender( context );
  mXAxis.gridMajorSymbol()->stopRender( context );
  mYAxis.gridMajorSymbol()->stopRender( context );
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

QRectF Qgs2DPlot::interiorPlotArea( QgsRenderContext &context ) const
{
  QgsExpressionContextScope *plotScope = new QgsExpressionContextScope( QStringLiteral( "plot" ) );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), plotScope );

  const double firstMinorYGrid = std::ceil( mMinY / mYAxis.gridIntervalMinor() ) * mYAxis.gridIntervalMinor();
  const double firstXLabel = std::ceil( mMinX / mXAxis.labelInterval() ) * mXAxis.labelInterval();

  QgsNumericFormatContext numericContext;

  // calculate text metrics
  double maxXAxisLabelHeight = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "x" ), true ) );
  for ( double currentX = firstXLabel; currentX <= mMaxX; currentX += mXAxis.labelInterval() )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentX, true ) );
    const QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
    maxXAxisLabelHeight = std::max( maxXAxisLabelHeight, QgsTextRenderer::textHeight( context, mXAxis.textFormat(), { text } ) );
  }

  double maxYAxisLabelWidth = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  for ( double currentY = firstMinorYGrid; currentY <= mMaxY; currentY += mYAxis.gridIntervalMinor() )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentY, true ) );
    const QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
    maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, QgsTextRenderer::textWidth( context, mYAxis.textFormat(), { text } ) );
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

void Qgs2DPlot::calculateOptimisedIntervals( QgsRenderContext &context )
{
  QgsNumericFormatContext numericContext;

  // calculate text metrics
  const double minXTextWidth = QgsTextRenderer::textWidth( context, mXAxis.textFormat(), { mXAxis.numericFormat()->formatDouble( mMinX, numericContext )  } );
  const double maxXTextWidth = QgsTextRenderer::textWidth( context, mXAxis.textFormat(), { mXAxis.numericFormat()->formatDouble( mMaxX, numericContext )  } );
  const double averageXTextWidth  = ( minXTextWidth + maxXTextWidth ) * 0.5;

  const double minYTextHeight = QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { mYAxis.numericFormat()->formatDouble( mMinY, numericContext )  } );
  const double maxYTextHeight = QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { mYAxis.numericFormat()->formatDouble( mMaxY, numericContext )  } );
  const double averageYTextHeight = ( minYTextHeight + maxYTextHeight ) * 0.5;

  const double leftMargin = context.convertToPainterUnits( mMargins.left(), QgsUnitTypes::RenderMillimeters );
  const double rightMargin = context.convertToPainterUnits( mMargins.right(), QgsUnitTypes::RenderMillimeters );
  const double topMargin = context.convertToPainterUnits( mMargins.top(), QgsUnitTypes::RenderMillimeters );
  const double bottomMargin = context.convertToPainterUnits( mMargins.bottom(), QgsUnitTypes::RenderMillimeters );

  const double availableWidth = mSize.width() - leftMargin - rightMargin;
  const double availableHeight = mSize.height() - topMargin - bottomMargin;

  // aim for roughly 40% of the width/height to be taken up by labels
  // we start with this and drop labels till things fit nicely
  int numberLabelsX = std::floor( availableWidth / averageXTextWidth );
  int numberLabelsY = std::floor( availableHeight / averageYTextHeight );

  auto roundBase10 = []( double value )->double
  {
    return std::pow( 10, std::ceil( std::log10( value ) ) );
  };

  double labelIntervalX = ( mMaxX - mMinX ) / numberLabelsX;
  double candidate = roundBase10( labelIntervalX );
  int numberLabels = 0;
  while ( true )
  {
    const double firstXLabel = std::ceil( mMinX / candidate ) * candidate;
    double totalWidth = 0;
    numberLabels = 0;
    for ( double currentX = firstXLabel; currentX <= mMaxX; currentX += candidate )
    {
      const QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
      totalWidth += QgsTextRenderer::textWidth( context, mXAxis.textFormat(), { text } );
      numberLabels += 1;
    }

    if ( totalWidth <= availableWidth * 0.4 )
      break;

    candidate *= 2;
  }
  mXAxis.setLabelInterval( candidate );
  if ( numberLabels < 10 )
  {
    mXAxis.setGridIntervalMinor( mXAxis.labelInterval() / 2 );
    mXAxis.setGridIntervalMajor( mXAxis.gridIntervalMinor() * 4 );
  }
  else
  {
    mXAxis.setGridIntervalMinor( mXAxis.labelInterval() );
    mXAxis.setGridIntervalMajor( mXAxis.gridIntervalMinor() * 5 );
  }

  double labelIntervalY = ( mMaxY - mMinY ) / numberLabelsY;
  candidate = roundBase10( labelIntervalY );
  while ( true )
  {
    const double firstYLabel = std::ceil( mMinY / candidate ) * candidate;
    double totalHeight = 0;
    numberLabels = 0;
    for ( double currentY = firstYLabel; currentY <= mMaxY; currentY += candidate )
    {
      const QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
      totalHeight += QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { text } );
    }

    if ( totalHeight <= availableHeight * 0.4 )
      break;

    candidate *= 2;
  }
  mYAxis.setLabelInterval( candidate );
  if ( numberLabels < 10 )
  {
    mYAxis.setGridIntervalMinor( mYAxis.labelInterval() / 2 );
    mYAxis.setGridIntervalMajor( mYAxis.gridIntervalMinor() * 4 );
  }
  else
  {
    mYAxis.setGridIntervalMinor( mYAxis.labelInterval() );
    mYAxis.setGridIntervalMajor( mYAxis.gridIntervalMinor() * 5 );
  }
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

const QgsMargins &Qgs2DPlot::margins() const
{
  return mMargins;
}

void Qgs2DPlot::setMargins( const QgsMargins &margins )
{
  mMargins = margins;
}
