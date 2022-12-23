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
#include <functional>

QgsPlot::~QgsPlot() = default;

bool QgsPlot::writeXml( QDomElement &, QDomDocument &, QgsReadWriteContext & ) const
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

bool QgsPlotAxis::writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context ) const
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

bool Qgs2DPlot::writeXml( QDomElement &element, QDomDocument &document, QgsReadWriteContext &context ) const
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

  element.setAttribute( QStringLiteral( "margins" ), mMargins.toString() );

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

  mMargins = QgsMargins::fromString( element.attribute( QStringLiteral( "margins" ) ) );

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

  const double xTolerance = mXAxis.gridIntervalMinor() / 100000;
  const double yTolerance = mYAxis.gridIntervalMinor() / 100000;

  QgsNumericFormatContext numericContext;

  // calculate text metrics
  double maxYAxisLabelWidth = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  for ( double currentY = firstYLabel; currentY <= mMaxY && !qgsDoubleNear( currentY, mMaxY, yTolerance ); currentY += mYAxis.labelInterval() )
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

  constexpr int MAX_OBJECTS = 1000;

  // grid lines

  // x
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "x" ), true ) );
  double nextMajorXGrid = firstMajorXGrid;
  int objectNumber = 0;
  for ( double currentX = firstMinorXGrid; objectNumber < MAX_OBJECTS && ( currentX <= mMaxX && !qgsDoubleNear( currentX, mMaxX, xTolerance ) ); currentX += mXAxis.gridIntervalMinor(), ++objectNumber )
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
  objectNumber = 0;
  for ( double currentY = firstMinorYGrid; objectNumber < MAX_OBJECTS && ( currentY <= mMaxY && !qgsDoubleNear( currentY, mMaxY, yTolerance ) ); currentY += mYAxis.gridIntervalMinor(), ++objectNumber )
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
  objectNumber = 0;
  for ( double currentX = firstXLabel; objectNumber < MAX_OBJECTS && ( currentX <= mMaxX || qgsDoubleNear( currentX, mMaxX, xTolerance ) ); currentX += mXAxis.labelInterval(), ++objectNumber )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentX, true ) );
    const QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
    QgsTextRenderer::drawText( QPointF( ( currentX - mMinX ) * xScale + chartAreaLeft, mSize.height() - context.convertToPainterUnits( mMargins.bottom(), QgsUnitTypes::RenderMillimeters ) ),
                               0, Qgis::TextHorizontalAlignment::Center, { text }, context, mXAxis.textFormat() );
  }

  // y
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  objectNumber = 0;
  for ( double currentY = firstYLabel; objectNumber < MAX_OBJECTS && ( currentY <= mMaxY || qgsDoubleNear( currentY, mMaxY, yTolerance ) ); currentY += mYAxis.labelInterval(), ++objectNumber )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentY, true ) );
    const QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
    const double height = QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { text } );
    QgsTextRenderer::drawText( QPointF(
                                 maxYAxisLabelWidth + context.convertToPainterUnits( mMargins.left(), QgsUnitTypes::RenderMillimeters ),
                                 chartAreaBottom - ( currentY - mMinY ) * yScale + height / 2 ),
                               0, Qgis::TextHorizontalAlignment::Right, { text }, context, mYAxis.textFormat(), false );
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

  const double xTolerance = mXAxis.gridIntervalMinor() / 100000;
  const double yTolerance = mYAxis.gridIntervalMinor() / 100000;

  constexpr int MAX_LABELS = 1000;

  // calculate text metrics
  int labelNumber = 0;
  double maxXAxisLabelHeight = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "x" ), true ) );
  for ( double currentX = firstXLabel; labelNumber < MAX_LABELS && ( currentX <= mMaxX || qgsDoubleNear( currentX, mMaxX, xTolerance ) ); currentX += mXAxis.labelInterval(), labelNumber++ )
  {
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis_value" ), currentX, true ) );
    const QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
    maxXAxisLabelHeight = std::max( maxXAxisLabelHeight, QgsTextRenderer::textHeight( context, mXAxis.textFormat(), { text } ) );
  }

  double maxYAxisLabelWidth = 0;
  labelNumber = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "plot_axis" ), QStringLiteral( "y" ), true ) );
  for ( double currentY = firstMinorYGrid; labelNumber < MAX_LABELS && ( currentY <= mMaxY || qgsDoubleNear( currentY, mMaxY, yTolerance ) ); currentY += mYAxis.gridIntervalMinor(), labelNumber ++ )
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
  // aim for about 40% coverage of label text to available space
  constexpr double IDEAL_WIDTH = 0.4;
  constexpr double TOLERANCE = 0.04;
  constexpr int MAX_LABELS = 1000;

  const double leftMargin = context.convertToPainterUnits( mMargins.left(), QgsUnitTypes::RenderMillimeters );
  const double rightMargin = context.convertToPainterUnits( mMargins.right(), QgsUnitTypes::RenderMillimeters );
  const double topMargin = context.convertToPainterUnits( mMargins.top(), QgsUnitTypes::RenderMillimeters );
  const double bottomMargin = context.convertToPainterUnits( mMargins.bottom(), QgsUnitTypes::RenderMillimeters );

  const double availableWidth = mSize.width() - leftMargin - rightMargin;
  const double availableHeight = mSize.height() - topMargin - bottomMargin;

  QgsNumericFormatContext numericContext;

  auto refineIntervalForAxis = [&]( double axisMinimum, double axisMaximum,
                                    const std::function< double( double ) > &sizeForLabel,
                                    double availableSize, double idealSizePercent, double sizeTolerancePercent,
                                    double & labelInterval, double & majorInterval, double & minorInterval )
  {
    auto roundBase10 = []( double value )->double
    {
      return std::pow( 10, std::floor( std::log10( value ) ) );
    };

    // if the current interval is good enough, don't change it!
    double totalSize = 0;
    int initialLabelCount = 0;
    {
      const double firstLabelPos = std::ceil( axisMinimum / labelInterval ) * labelInterval;

      for ( double currentPos = firstLabelPos; initialLabelCount <= MAX_LABELS && currentPos <= axisMaximum; currentPos += labelInterval, ++initialLabelCount )
      {
        totalSize += sizeForLabel( currentPos );
      }
    }

    // we consider the current interval as "good enough" if it results in somewhere between 20-60% label text coverage over the size
    if ( initialLabelCount >= MAX_LABELS || ( totalSize  / availableSize < ( idealSizePercent - sizeTolerancePercent ) ) || ( totalSize  / availableSize > ( idealSizePercent + sizeTolerancePercent ) ) )
    {
      // we start with trying to fit 30 labels in and then raise the interval till we're happy
      int numberLabelsInitial = std::floor( availableSize / 30 );

      double labelIntervalTest = ( axisMaximum - axisMinimum ) / numberLabelsInitial;
      double baseValue = roundBase10( labelIntervalTest );
      double candidate = baseValue;
      int currentMultiplier = 1;

      int numberLabels = 0;
      while ( true )
      {
        const double firstLabelPosition = std::ceil( axisMinimum / candidate ) * candidate;
        double totalSize = 0;
        numberLabels = 0;
        for ( double currentPos = firstLabelPosition; currentPos <= axisMaximum; currentPos += candidate )
        {
          totalSize += sizeForLabel( currentPos );
          numberLabels += 1;

          if ( numberLabels > MAX_LABELS ) // avoid hangs if candidate size is very small
            break;
        }

        if ( numberLabels <= MAX_LABELS && totalSize <= availableSize * idealSizePercent )
          break;

        if ( currentMultiplier == 1 )
          currentMultiplier = 2;
        else if ( currentMultiplier == 2 )
          currentMultiplier = 5;
        else if ( currentMultiplier == 5 )
        {
          baseValue *= 10;
          currentMultiplier = 1;
        }

        candidate = baseValue * currentMultiplier;
      }
      labelInterval = candidate;
      if ( numberLabels < 10 )
      {
        minorInterval = labelInterval / 2;
        majorInterval = minorInterval * 4;
      }
      else
      {
        minorInterval = labelInterval;
        majorInterval = minorInterval * 5;
      }
    }
  };

  {
    double labelIntervalX = mXAxis.labelInterval();
    double majorIntervalX = mXAxis.gridIntervalMajor();
    double minorIntervalX = mXAxis.gridIntervalMinor();
    refineIntervalForAxis( mMinX, mMaxX, [ = ]( double position ) -> double
    {
      const QString text = mXAxis.numericFormat()->formatDouble( position, numericContext );
      return QgsTextRenderer::textWidth( context,  mXAxis.textFormat(), { text } );

    }, availableWidth,
    IDEAL_WIDTH, TOLERANCE, labelIntervalX, majorIntervalX, minorIntervalX );
    mXAxis.setLabelInterval( labelIntervalX );
    mXAxis.setGridIntervalMajor( majorIntervalX );
    mXAxis.setGridIntervalMinor( minorIntervalX );
  }

  {
    double labelIntervalY = mYAxis.labelInterval();
    double majorIntervalY = mYAxis.gridIntervalMajor();
    double minorIntervalY = mYAxis.gridIntervalMinor();
    refineIntervalForAxis( mMinY, mMaxY, [ = ]( double position ) -> double
    {
      const QString text = mYAxis.numericFormat()->formatDouble( position, numericContext );
      return QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { text } );
    }, availableHeight,
    IDEAL_WIDTH, TOLERANCE, labelIntervalY, majorIntervalY, minorIntervalY );
    mYAxis.setLabelInterval( labelIntervalY );
    mYAxis.setGridIntervalMajor( majorIntervalY );
    mYAxis.setGridIntervalMinor( minorIntervalY );
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
