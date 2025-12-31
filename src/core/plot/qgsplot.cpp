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

#include <functional>

#include "qgsapplication.h"
#include "qgsbasicnumericformat.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsnumericformatregistry.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer.h"

QgsPropertiesDefinition QgsPlot::sPropertyDefinitions;

QgsPlot::~QgsPlot() = default;

bool QgsPlot::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  element.setAttribute( u"plotType"_s, type() );

  QDomElement dataDefinedPropertiesElement = document.createElement( u"dataDefinedProperties"_s );
  mDataDefinedProperties.writeXml( dataDefinedPropertiesElement, QgsPlot::propertyDefinitions() );
  element.appendChild( dataDefinedPropertiesElement );

  return true;
}

bool QgsPlot::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  QDomElement dataDefinedPropertiesElement = element.firstChildElement( u"dataDefinedProperties"_s );
  mDataDefinedProperties.readXml( dataDefinedPropertiesElement, QgsPlot::propertyDefinitions() );

  return true;
}

void QgsPlot::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsPlot::DataDefinedProperty::MarginLeft ), QgsPropertyDefinition( "dataDefinedPlotMarginLeft", QObject::tr( "Left margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::MarginTop ), QgsPropertyDefinition( "dataDefinedPlotMarginTop", QObject::tr( "Top margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::MarginRight ), QgsPropertyDefinition( "dataDefinedPlotMarginRight", QObject::tr( "Right margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::MarginBottom ), QgsPropertyDefinition( "dataDefinedPlotMarginBottom", QObject::tr( "Bottom margin" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::XAxisMajorInterval ), QgsPropertyDefinition( "dataDefinedPlotXAxisMajorInterval", QObject::tr( "Major grid line interval for X-axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::XAxisMinorInterval ), QgsPropertyDefinition( "dataDefinedPlotXAxisMinorInterval", QObject::tr( "Minor grid line interval for X-axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::XAxisLabelInterval ), QgsPropertyDefinition( "dataDefinedPlotXAxisLabelInterval", QObject::tr( "Label interval for X-axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::YAxisMajorInterval ), QgsPropertyDefinition( "dataDefinedPlotYAxisMajorInterval", QObject::tr( "Major grid line interval for Y-axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::YAxisMinorInterval ), QgsPropertyDefinition( "dataDefinedPlotYAxisMinorInterval", QObject::tr( "Minor grid line interval for Y-axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::YAxisLabelInterval ), QgsPropertyDefinition( "dataDefinedPlotYAxisLabelInterval", QObject::tr( "Label interval for Y-axis" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::XAxisMinimum ), QgsPropertyDefinition( "dataDefinedPlotXAxisMinimum", QObject::tr( "X-axis minimum value" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::XAxisMaximum ), QgsPropertyDefinition( "dataDefinedPlotXAxisMaximum", QObject::tr( "X-axis maximum value" ), QgsPropertyDefinition::DoublePositive ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::YAxisMinimum ), QgsPropertyDefinition( "dataDefinedPlotYAxisMinimum", QObject::tr( "Y-axis minimum value" ), QgsPropertyDefinition::Double ) },
    { static_cast< int >( QgsPlot::DataDefinedProperty::YAxisMaximum ), QgsPropertyDefinition( "dataDefinedPlotYAxisMaximum", QObject::tr( "Y-axis maximum value" ), QgsPropertyDefinition::Double ) },
  };
}

const QgsPropertiesDefinition &QgsPlot::propertyDefinitions()
{
  QgsPlot::initPropertyDefinitions();
  return sPropertyDefinitions;
}

// QgsPlotAxis

QgsPlotAxis::QgsPlotAxis()
{
  // setup default style
  mNumericFormat.reset( QgsPlotDefaultSettings::axisLabelNumericFormat() );
  mGridMinorSymbol.reset( QgsPlotDefaultSettings::axisGridMinorSymbol() );
  mGridMajorSymbol.reset( QgsPlotDefaultSettings::axisGridMajorSymbol() );
}

QgsPlotAxis::~QgsPlotAxis() = default;


Qgis::PlotAxisType QgsPlotAxis::type() const
{
  return mType;
}

void QgsPlotAxis::setType( Qgis::PlotAxisType type )
{
  mType = type;
}

bool QgsPlotAxis::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( u"type"_s, qgsEnumValueToKey( mType ) );
  element.setAttribute( u"gridIntervalMinor"_s, qgsDoubleToString( mGridIntervalMinor ) );
  element.setAttribute( u"gridIntervalMajor"_s, qgsDoubleToString( mGridIntervalMajor ) );
  element.setAttribute( u"labelInterval"_s, qgsDoubleToString( mLabelInterval ) );
  element.setAttribute( u"suffix"_s, mLabelSuffix );
  element.setAttribute( u"suffixPlacement"_s, qgsEnumValueToKey( mSuffixPlacement ) );

  QDomElement numericFormatElement = document.createElement( u"numericFormat"_s );
  mNumericFormat->writeXml( numericFormatElement, document, context );
  element.appendChild( numericFormatElement );

  QDomElement gridMajorElement = document.createElement( u"gridMajorSymbol"_s );
  gridMajorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mGridMajorSymbol.get(), document, context ) );
  element.appendChild( gridMajorElement );
  QDomElement gridMinorElement = document.createElement( u"gridMinorSymbol"_s );
  gridMinorElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mGridMinorSymbol.get(), document, context ) );
  element.appendChild( gridMinorElement );

  QDomElement textFormatElement = document.createElement( u"textFormat"_s );
  textFormatElement.appendChild( mLabelTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElement );

  return true;
}

bool QgsPlotAxis::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mType = qgsEnumKeyToValue( element.attribute( u"type"_s ), Qgis::PlotAxisType::Interval );
  mGridIntervalMinor = element.attribute( u"gridIntervalMinor"_s ).toDouble();
  mGridIntervalMajor = element.attribute( u"gridIntervalMajor"_s ).toDouble();
  mLabelInterval = element.attribute( u"labelInterval"_s ).toDouble();

  mLabelSuffix = element.attribute( u"suffix"_s );
  mSuffixPlacement = qgsEnumKeyToValue( element.attribute( u"suffixPlacement"_s ), Qgis::PlotAxisSuffixPlacement::NoLabels );

  const QDomElement numericFormatElement = element.firstChildElement( u"numericFormat"_s );
  mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElement, context ) );

  const QDomElement gridMajorElement = element.firstChildElement( u"gridMajorSymbol"_s ).firstChildElement( u"symbol"_s );
  mGridMajorSymbol = QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( gridMajorElement, context );
  const QDomElement gridMinorElement = element.firstChildElement( u"gridMinorSymbol"_s ).firstChildElement( u"symbol"_s );
  mGridMinorSymbol = QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( gridMinorElement, context );

  const QDomElement textFormatElement = element.firstChildElement( u"textFormat"_s );
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

QString QgsPlotAxis::labelSuffix() const
{
  return mLabelSuffix;
}

void QgsPlotAxis::setLabelSuffix( const QString &suffix )
{
  mLabelSuffix = suffix;
}

Qgis::PlotAxisSuffixPlacement QgsPlotAxis::labelSuffixPlacement() const
{
  return mSuffixPlacement;
}

void QgsPlotAxis::setLabelSuffixPlacement( Qgis::PlotAxisSuffixPlacement placement )
{
  mSuffixPlacement = placement;
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
}

bool Qgs2DPlot::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QgsPlot::writeXml( element, document, context );

  element.setAttribute( u"margins"_s, mMargins.toString() );

  return true;
}

bool Qgs2DPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  QgsPlot::readXml( element, context );

  mMargins = QgsMargins::fromString( element.attribute( u"margins"_s ) );

  return true;
}

void Qgs2DPlot::render( QgsRenderContext &context, QgsPlotRenderContext &plotContext, const QgsPlotData &plotData )
{
  QgsExpressionContextScope *plotScope = new QgsExpressionContextScope( u"plot"_s );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), plotScope );

  const QRectF plotArea = interiorPlotArea( context, plotContext );

  // give subclasses a chance to draw their content
  renderContent( context, plotContext, plotArea, plotData );
}

void Qgs2DPlot::renderContent( QgsRenderContext &, QgsPlotRenderContext &, const QRectF &, const QgsPlotData & )
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

QRectF Qgs2DPlot::interiorPlotArea( QgsRenderContext &context, QgsPlotRenderContext & ) const
{
  QgsMargins usedMargins = mMargins;
  applyDataDefinedProperties( context, usedMargins );

  const double leftMargin = context.convertToPainterUnits( usedMargins.left(), Qgis::RenderUnit::Millimeters );
  const double rightMargin = context.convertToPainterUnits( usedMargins.right(), Qgis::RenderUnit::Millimeters );
  const double topMargin = context.convertToPainterUnits( usedMargins.top(), Qgis::RenderUnit::Millimeters );
  const double bottomMargin = context.convertToPainterUnits( usedMargins.bottom(), Qgis::RenderUnit::Millimeters );

  return QRectF( leftMargin, topMargin, mSize.width() - rightMargin - leftMargin, mSize.height() - bottomMargin - topMargin );
}

const QgsMargins &Qgs2DPlot::margins() const
{
  return mMargins;
}

void Qgs2DPlot::setMargins( const QgsMargins &margins )
{
  mMargins = margins;
}

void Qgs2DPlot::applyDataDefinedProperties( QgsRenderContext &context, QgsMargins &margins ) const
{
  if ( !dataDefinedProperties().hasActiveProperties() )
  {
    return;
  }

  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::MarginLeft ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::MarginLeft, context.expressionContext(), margins.left(), &ok );

    if ( ok )
    {
      margins.setLeft( value );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::MarginRight ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::MarginRight, context.expressionContext(), margins.right(), &ok );

    if ( ok )
    {
      margins.setRight( value );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::MarginTop ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::MarginTop, context.expressionContext(), margins.top(), &ok );

    if ( ok )
    {
      margins.setTop( value );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::MarginBottom ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::MarginBottom, context.expressionContext(), margins.bottom(), &ok );

    if ( ok )
    {
      margins.setBottom( value );
    }
  }
}


//
// Qgs2DPlot
//

Qgs2DXyPlot::Qgs2DXyPlot()
  : Qgs2DPlot()
{
  // setup default style
  mChartBackgroundSymbol.reset( QgsPlotDefaultSettings::chartBackgroundSymbol() );
  mChartBorderSymbol.reset( QgsPlotDefaultSettings::chartBorderSymbol() );
}

bool Qgs2DXyPlot::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Qgs2DPlot::writeXml( element, document, context );

  element.setAttribute( u"minX"_s, qgsDoubleToString( mMinX ) );
  element.setAttribute( u"maxX"_s, qgsDoubleToString( mMaxX ) );
  element.setAttribute( u"minY"_s, qgsDoubleToString( mMinY ) );
  element.setAttribute( u"maxY"_s, qgsDoubleToString( mMaxY ) );

  QDomElement xAxisElement = document.createElement( u"xAxis"_s );
  mXAxis.writeXml( xAxisElement, document, context );
  element.appendChild( xAxisElement );
  QDomElement yAxisElement = document.createElement( u"yAxis"_s );
  mYAxis.writeXml( yAxisElement, document, context );
  element.appendChild( yAxisElement );

  QDomElement backgroundElement = document.createElement( u"backgroundSymbol"_s );
  backgroundElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mChartBackgroundSymbol.get(), document, context ) );
  element.appendChild( backgroundElement );
  QDomElement borderElement = document.createElement( u"borderSymbol"_s );
  borderElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QString(), mChartBorderSymbol.get(), document, context ) );
  element.appendChild( borderElement );

  return true;
}

bool Qgs2DXyPlot::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Qgs2DPlot::readXml( element, context );

  mMinX = element.attribute( u"minX"_s ).toDouble();
  mMaxX = element.attribute( u"maxX"_s ).toDouble();
  mMinY = element.attribute( u"minY"_s ).toDouble();
  mMaxY = element.attribute( u"maxY"_s ).toDouble();

  const QDomElement xAxisElement = element.firstChildElement( u"xAxis"_s );
  mXAxis.readXml( xAxisElement, context );
  const QDomElement yAxisElement = element.firstChildElement( u"yAxis"_s );
  mYAxis.readXml( yAxisElement, context );

  const QDomElement backgroundElement = element.firstChildElement( u"backgroundSymbol"_s ).firstChildElement( u"symbol"_s );
  mChartBackgroundSymbol = QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundElement, context );
  const QDomElement borderElement = element.firstChildElement( u"borderSymbol"_s ).firstChildElement( u"symbol"_s );
  mChartBorderSymbol = QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( borderElement, context );

  return true;
}

void Qgs2DXyPlot::render( QgsRenderContext &context, QgsPlotRenderContext &plotContext, const QgsPlotData &plotData )
{
  QgsExpressionContextScope *plotScope = new QgsExpressionContextScope( u"plot"_s );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), plotScope );

  mChartBackgroundSymbol->startRender( context );
  mChartBorderSymbol->startRender( context );
  mXAxis.gridMinorSymbol()->startRender( context );
  mYAxis.gridMinorSymbol()->startRender( context );
  mXAxis.gridMajorSymbol()->startRender( context );
  mYAxis.gridMajorSymbol()->startRender( context );

  QgsMargins margins = mMargins;
  Qgs2DPlot::applyDataDefinedProperties( context, margins );
  double minX = mMinX;
  double maxX = mMaxX;
  double minY = mMinY;
  double maxY = mMaxY;
  double majorIntervalX = mXAxis.gridIntervalMajor();
  double minorIntervalX = mXAxis.gridIntervalMinor();
  double labelIntervalX = mXAxis.labelInterval();
  double majorIntervalY = mYAxis.gridIntervalMajor();
  double minorIntervalY = mYAxis.gridIntervalMinor();
  double labelIntervalY = mYAxis.labelInterval();
  applyDataDefinedProperties( context, minX, maxX, minY, maxY, majorIntervalX, minorIntervalX, labelIntervalX, majorIntervalY, minorIntervalY, labelIntervalY );

  const double firstMinorXGrid = std::ceil( minX / minorIntervalX ) * minorIntervalX;
  const double firstMajorXGrid = std::ceil( minX / majorIntervalX ) * majorIntervalX;
  const double firstMinorYGrid = std::ceil( minY / minorIntervalY ) * minorIntervalY;
  const double firstMajorYGrid = std::ceil( minY / majorIntervalY ) * majorIntervalY;
  const double firstXLabel = labelIntervalX > 0 ? std::ceil( minX / labelIntervalX ) * labelIntervalX : 0;
  const double firstYLabel = labelIntervalY > 0 ? std::ceil( minY / labelIntervalY ) * labelIntervalY : 0;

  const QString xAxisSuffix = mXAxis.labelSuffix();
  const QString yAxisSuffix = mYAxis.labelSuffix();

  const QRectF plotArea = interiorPlotArea( context, plotContext );

  const double xTolerance = minorIntervalX / 100000;
  const double yTolerance = minorIntervalY / 100000;

  QgsNumericFormatContext numericContext;

  // categories
  const QStringList categories = plotData.categories();

  // calculate text metrics
  double maxYAxisLabelWidth = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"y"_s, true ) );
  switch ( mYAxis.type() )
  {
    case Qgis::PlotAxisType::Interval:
      if ( labelIntervalY > 0 )
      {
        for ( double currentY = firstYLabel; ; currentY += labelIntervalY )
        {
          const bool hasMoreLabels = currentY + labelIntervalY <= maxY && !qgsDoubleNear( currentY + labelIntervalY, maxY, yTolerance );
          plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentY, true ) );
          QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
          switch ( mYAxis.labelSuffixPlacement() )
          {
            case Qgis::PlotAxisSuffixPlacement::NoLabels:
              break;

            case Qgis::PlotAxisSuffixPlacement::EveryLabel:
              text += yAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::FirstLabel:
              if ( currentY == firstYLabel )
                text += yAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::LastLabel:
              if ( !hasMoreLabels )
                text += yAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels:
              if ( currentY == firstYLabel || !hasMoreLabels )
                text += yAxisSuffix;
              break;
          }

          maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, QgsTextRenderer::textWidth( context, mYAxis.textFormat(), { text } ) );
          if ( !hasMoreLabels )
            break;
        }
      }
      break;

    case Qgis::PlotAxisType::Categorical:
      for ( int i = 0; i < categories.size(); i++ )
      {
        maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, QgsTextRenderer::textWidth( context, mYAxis.textFormat(), { categories.at( i ) } ) );
      }
      break;
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

  const double xScale = ( chartAreaRight - chartAreaLeft ) / ( maxX - minX );
  const double yScale = ( chartAreaBottom - chartAreaTop ) / ( maxY - minY );

  constexpr int MAX_OBJECTS = 1000;

  // grid lines

  // x
  switch ( mXAxis.type() )
  {
    case Qgis::PlotAxisType::Interval:
    {
      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"x"_s, true ) );
      double nextMajorXGrid = firstMajorXGrid;
      int objectNumber = 0;
      for ( double currentX = firstMinorXGrid; objectNumber < MAX_OBJECTS && ( currentX <= maxX && !qgsDoubleNear( currentX, maxX, xTolerance ) ); currentX += minorIntervalX, ++objectNumber )
      {
        bool isMinor = true;
        if ( qgsDoubleNear( currentX, nextMajorXGrid, xTolerance ) )
        {
          isMinor = false;
          nextMajorXGrid += majorIntervalX;
        }

        plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentX, true ) );

        QgsLineSymbol *currentGridSymbol = isMinor ? mXAxis.gridMinorSymbol() : mXAxis.gridMajorSymbol();
        currentGridSymbol->renderPolyline( QPolygonF(
                                             QVector<QPointF>
        {
          QPointF( ( currentX - minX ) * xScale + chartAreaLeft, chartAreaBottom ),
          QPointF( ( currentX - minX ) * xScale + chartAreaLeft, chartAreaTop )
        } ), nullptr, context );
      }
      break;
    }

    case Qgis::PlotAxisType::Categorical:
      // No grid lines here, skipping
      break;
  }

  // y
  switch ( mYAxis.type() )
  {
    case Qgis::PlotAxisType::Interval:
    {
      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"y"_s, true ) );
      double nextMajorYGrid = firstMajorYGrid;
      int objectNumber = 0;
      for ( double currentY = firstMinorYGrid; objectNumber < MAX_OBJECTS && ( currentY <= maxY && !qgsDoubleNear( currentY, maxY, yTolerance ) ); currentY += minorIntervalY, ++objectNumber )
      {
        bool isMinor = true;
        if ( qgsDoubleNear( currentY, nextMajorYGrid, yTolerance ) )
        {
          isMinor = false;
          nextMajorYGrid += majorIntervalY;
        }

        plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentY, true ) );

        QgsLineSymbol *currentGridSymbol = isMinor ? mYAxis.gridMinorSymbol() : mYAxis.gridMajorSymbol();
        currentGridSymbol->renderPolyline( QPolygonF(
                                             QVector<QPointF>
        {
          QPointF( chartAreaLeft, chartAreaBottom - ( currentY - minY ) * yScale ),
          QPointF( chartAreaRight, chartAreaBottom - ( currentY - minY ) * yScale )
        } ), nullptr, context );
      }
      break;
    }

    case Qgis::PlotAxisType::Categorical:
      // No grid lines here, skipping
      break;
  }

  // axis labels

  // x
  switch ( mXAxis.type() )
  {
    case Qgis::PlotAxisType::Interval:
    {
      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"x"_s, true ) );
      int objectNumber = 0;
      if ( labelIntervalX > 0 )
      {
        for ( double currentX = firstXLabel; ; currentX += labelIntervalX, ++objectNumber )
        {
          const bool hasMoreLabels = objectNumber + 1 < MAX_OBJECTS && ( currentX + labelIntervalX <= maxX || qgsDoubleNear( currentX + labelIntervalX, maxX, xTolerance ) );
          plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentX, true ) );
          QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
          switch ( mXAxis.labelSuffixPlacement() )
          {
            case Qgis::PlotAxisSuffixPlacement::NoLabels:
              break;

            case Qgis::PlotAxisSuffixPlacement::EveryLabel:
              text += xAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::FirstLabel:
              if ( objectNumber == 0 )
                text += xAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::LastLabel:
              if ( !hasMoreLabels )
                text += xAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels:
              if ( objectNumber == 0 || !hasMoreLabels )
                text += xAxisSuffix;
              break;
          }

          QgsTextRenderer::drawText( QPointF( ( currentX - minX ) * xScale + chartAreaLeft, mSize.height() - context.convertToPainterUnits( margins.bottom(), Qgis::RenderUnit::Millimeters ) ),
                                     0, Qgis::TextHorizontalAlignment::Center, { text }, context, mXAxis.textFormat() );
          if ( !hasMoreLabels )
            break;
        }
      }
      break;
    }

    case Qgis::PlotAxisType::Categorical:
    {
      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"x"_s, true ) );
      const double categoryWidth = plotArea.width() / categories.size();
      for ( int i = 0; i < categories.size(); i++ )
      {
        const double currentX = ( i * categoryWidth ) + categoryWidth / 2.0;
        plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, categories.at( i ), true ) );
        QgsTextRenderer::drawText( QPointF( currentX + chartAreaLeft, mSize.height() - context.convertToPainterUnits( margins.bottom(), Qgis::RenderUnit::Millimeters ) ),
                                   0, Qgis::TextHorizontalAlignment::Center, { categories.at( i ) }, context, mXAxis.textFormat() );
      }
      break;
    }
  }

  // y
  switch ( mYAxis.type() )
  {
    case Qgis::PlotAxisType::Interval:
    {
      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"y"_s, true ) );
      int objectNumber = 0;
      if ( labelIntervalY > 0 )
      {
        for ( double currentY = firstYLabel; ; currentY += labelIntervalY, ++objectNumber )
        {
          const bool hasMoreLabels = objectNumber + 1 < MAX_OBJECTS && ( currentY + labelIntervalY <= maxY || qgsDoubleNear( currentY + labelIntervalY, maxY, yTolerance ) );
          plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentY, true ) );
          QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
          switch ( mYAxis.labelSuffixPlacement() )
          {
            case Qgis::PlotAxisSuffixPlacement::NoLabels:
              break;

            case Qgis::PlotAxisSuffixPlacement::EveryLabel:
              text += yAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::FirstLabel:
              if ( objectNumber == 0 )
                text += yAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::LastLabel:
              if ( !hasMoreLabels )
                text += yAxisSuffix;
              break;

            case Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels:
              if ( objectNumber == 0 || !hasMoreLabels )
                text += yAxisSuffix;
              break;
          }

          const double height = QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { text } );
          QgsTextRenderer::drawText( QPointF(
                                       maxYAxisLabelWidth + context.convertToPainterUnits( margins.left(), Qgis::RenderUnit::Millimeters ),
                                       chartAreaBottom - ( currentY - minY ) * yScale + height / 2 ),
                                     0, Qgis::TextHorizontalAlignment::Right, { text }, context, mYAxis.textFormat(), false );
          if ( !hasMoreLabels )
            break;
        }
      }
      break;
    }

    case Qgis::PlotAxisType::Categorical:
    {
      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"y"_s, true ) );
      const double categoryHeight = plotArea.height() / categories.size();
      for ( int i = 0; i < categories.size(); i++ )
      {
        const double currentY = ( i * categoryHeight ) + categoryHeight / 2.0;
        plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, categories.at( i ), true ) );
        const double height = QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { categories.at( i ) } );
        QgsTextRenderer::drawText( QPointF(
                                     maxYAxisLabelWidth + context.convertToPainterUnits( margins.left(), Qgis::RenderUnit::Millimeters ),
                                     chartAreaBottom - currentY + height / 2 ),
                                   0, Qgis::TextHorizontalAlignment::Right, { categories.at( i ) }, context, mYAxis.textFormat(), false );
      }
      break;
    }
  }

  // give subclasses a chance to draw their content
  renderContent( context, plotContext, plotArea, plotData );

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

Qgs2DXyPlot::~Qgs2DXyPlot() = default;

QRectF Qgs2DXyPlot::interiorPlotArea( QgsRenderContext &context, QgsPlotRenderContext & ) const
{
  double minX = mMinX;
  double maxX = mMaxX;
  double minY = mMinY;
  double maxY = mMaxY;
  double majorIntervalX = mXAxis.gridIntervalMajor();
  double minorIntervalX = mXAxis.gridIntervalMinor();
  double labelIntervalX = mXAxis.labelInterval();
  double majorIntervalY = mYAxis.gridIntervalMajor();
  double minorIntervalY = mYAxis.gridIntervalMinor();
  double labelIntervalY = mYAxis.labelInterval();
  applyDataDefinedProperties( context, minX, maxX, minY, maxY, majorIntervalX, minorIntervalX, labelIntervalX, majorIntervalY, minorIntervalY, labelIntervalY );

  QgsExpressionContextScope *plotScope = new QgsExpressionContextScope( u"plot"_s );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), plotScope );

  const double firstMinorYGrid = std::ceil( minY / minorIntervalY ) * minorIntervalY;
  const double firstXLabel = labelIntervalX > 0 ? std::ceil( minX / labelIntervalX ) * labelIntervalX : 0;

  const QString xAxisSuffix = mXAxis.labelSuffix();
  const QString yAxisSuffix = mYAxis.labelSuffix();
  const double yAxisSuffixWidth = yAxisSuffix.isEmpty() ? 0 : QgsTextRenderer::textWidth( context, mYAxis.textFormat(), { yAxisSuffix } );

  QgsNumericFormatContext numericContext;

  const double xTolerance = minorIntervalX / 100000;
  const double yTolerance = minorIntervalX / 100000;

  constexpr int MAX_LABELS = 1000;

  // calculate text metrics
  int labelNumber = 0;
  double maxXAxisLabelHeight = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"x"_s, true ) );
  if ( labelIntervalX > 0 )
  {
    for ( double currentX = firstXLabel; ; currentX += labelIntervalX, labelNumber++ )
    {
      const bool hasMoreLabels = labelNumber + 1 < MAX_LABELS && ( currentX + labelIntervalX <= maxX || qgsDoubleNear( currentX + labelIntervalX, maxX, xTolerance ) );

      plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentX, true ) );
      QString text = mXAxis.numericFormat()->formatDouble( currentX, numericContext );
      switch ( mXAxis.labelSuffixPlacement() )
      {
        case Qgis::PlotAxisSuffixPlacement::NoLabels:
          break;

        case Qgis::PlotAxisSuffixPlacement::EveryLabel:
          text += xAxisSuffix;
          break;

        case Qgis::PlotAxisSuffixPlacement::FirstLabel:
          if ( labelNumber == 0 )
            text += xAxisSuffix;
          break;

        case Qgis::PlotAxisSuffixPlacement::LastLabel:
          if ( !hasMoreLabels )
            text += xAxisSuffix;
          break;

        case Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels:
          if ( labelNumber == 0 || !hasMoreLabels )
            text += xAxisSuffix;
          break;
      }
      maxXAxisLabelHeight = std::max( maxXAxisLabelHeight, QgsTextRenderer::textHeight( context, mXAxis.textFormat(), { text } ) );
      if ( !hasMoreLabels )
        break;
    }
  }

  double maxYAxisLabelWidth = 0;
  labelNumber = 0;
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, u"y"_s, true ) );
  for ( double currentY = firstMinorYGrid; ; currentY += minorIntervalY, labelNumber ++ )
  {
    const bool hasMoreLabels = labelNumber + 1 < MAX_LABELS && ( currentY + minorIntervalY <= maxY || qgsDoubleNear( currentY + minorIntervalY, maxY, yTolerance ) );
    plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, currentY, true ) );
    const QString text = mYAxis.numericFormat()->formatDouble( currentY, numericContext );
    double thisLabelWidth = QgsTextRenderer::textWidth( context, mYAxis.textFormat(), { text } );
    if ( yAxisSuffixWidth > 0 )
    {
      switch ( mYAxis.labelSuffixPlacement() )
      {
        case Qgis::PlotAxisSuffixPlacement::NoLabels:
          break;

        case Qgis::PlotAxisSuffixPlacement::EveryLabel:
          thisLabelWidth += yAxisSuffixWidth;
          break;

        case Qgis::PlotAxisSuffixPlacement::FirstLabel:
          if ( labelNumber == 0 )
            thisLabelWidth += yAxisSuffixWidth;
          break;

        case Qgis::PlotAxisSuffixPlacement::LastLabel:
          if ( !hasMoreLabels )
            thisLabelWidth += yAxisSuffixWidth;
          break;

        case Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels:
          if ( labelNumber == 0 || !hasMoreLabels )
            thisLabelWidth += yAxisSuffixWidth;
          break;
      }
    }
    maxYAxisLabelWidth = std::max( maxYAxisLabelWidth, thisLabelWidth );
    if ( !hasMoreLabels )
      break;
  }

  const double leftTextSize = maxYAxisLabelWidth + context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );
  const double rightTextSize = 0;
  const double bottomTextSize = maxXAxisLabelHeight + context.convertToPainterUnits( 0.5, Qgis::RenderUnit::Millimeters );
  const double topTextSize = 0;

  QgsMargins margins = mMargins;
  Qgs2DPlot::applyDataDefinedProperties( context, margins );
  const double leftMargin = context.convertToPainterUnits( margins.left(), Qgis::RenderUnit::Millimeters ) + leftTextSize;
  const double rightMargin = context.convertToPainterUnits( margins.right(), Qgis::RenderUnit::Millimeters ) + rightTextSize;
  const double topMargin = context.convertToPainterUnits( margins.top(), Qgis::RenderUnit::Millimeters ) + topTextSize;
  const double bottomMargin = context.convertToPainterUnits( margins.bottom(), Qgis::RenderUnit::Millimeters ) + bottomTextSize;

  return QRectF( leftMargin, topMargin, mSize.width() - rightMargin - leftMargin, mSize.height() - bottomMargin - topMargin );
}

void Qgs2DXyPlot::calculateOptimisedIntervals( QgsRenderContext &context, QgsPlotRenderContext & )
{
  if ( !mSize.isValid() )
    return;

  // aim for about 40% coverage of label text to available space
  constexpr double IDEAL_WIDTH = 0.4;
  constexpr double TOLERANCE = 0.04;
  constexpr int MAX_LABELS = 1000;

  QgsMargins margins = mMargins;
  Qgs2DPlot::applyDataDefinedProperties( context, margins );
  const double leftMargin = context.convertToPainterUnits( margins.left(), Qgis::RenderUnit::Millimeters );
  const double rightMargin = context.convertToPainterUnits( margins.right(), Qgis::RenderUnit::Millimeters );
  const double topMargin = context.convertToPainterUnits( margins.top(), Qgis::RenderUnit::Millimeters );
  const double bottomMargin = context.convertToPainterUnits( margins.bottom(), Qgis::RenderUnit::Millimeters );

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

  double minX = mMinX;
  double maxX = mMaxX;
  double minY = mMinY;
  double maxY = mMaxY;
  double majorIntervalX = mXAxis.gridIntervalMajor();
  double minorIntervalX = mXAxis.gridIntervalMinor();
  double labelIntervalX = mXAxis.labelInterval();
  double majorIntervalY = mYAxis.gridIntervalMajor();
  double minorIntervalY = mYAxis.gridIntervalMinor();
  double labelIntervalY = mYAxis.labelInterval();
  applyDataDefinedProperties( context, minX, maxX, minY, maxY, majorIntervalX, minorIntervalX, labelIntervalX, majorIntervalY, minorIntervalY, labelIntervalY );

  {
    const QString suffixX = mXAxis.labelSuffix();
    const double suffixWidth = !suffixX.isEmpty() ? QgsTextRenderer::textWidth( context,  mXAxis.textFormat(), { suffixX } ) : 0;
    refineIntervalForAxis( minX, maxX, [this, &context, suffixWidth, &numericContext]( double position ) -> double
    {
      const QString text = mXAxis.numericFormat()->formatDouble( position, numericContext );
      // this isn't accurate, as we're always considering the suffix to be present... but it's too tricky to actually consider
      // the suffix placement!
      return QgsTextRenderer::textWidth( context,  mXAxis.textFormat(), { text } ) + suffixWidth;
    }, availableWidth,
    IDEAL_WIDTH, TOLERANCE, labelIntervalX, majorIntervalX, minorIntervalX );
    mXAxis.setLabelInterval( labelIntervalX );
    mXAxis.setGridIntervalMajor( majorIntervalX );
    mXAxis.setGridIntervalMinor( minorIntervalX );
  }

  {
    const QString suffixY = mYAxis.labelSuffix();
    refineIntervalForAxis( minY, maxY, [this, &context, suffixY, &numericContext]( double position ) -> double
    {
      const QString text = mYAxis.numericFormat()->formatDouble( position, numericContext );
      // this isn't accurate, as we're always considering the suffix to be present... but it's too tricky to actually consider
      // the suffix placement!
      return QgsTextRenderer::textHeight( context, mYAxis.textFormat(), { text + suffixY } );
    }, availableHeight,
    IDEAL_WIDTH, TOLERANCE, labelIntervalY, majorIntervalY, minorIntervalY );
    mYAxis.setLabelInterval( labelIntervalY );
    mYAxis.setGridIntervalMajor( majorIntervalY );
    mYAxis.setGridIntervalMinor( minorIntervalY );
  }
}

QgsFillSymbol *Qgs2DXyPlot::chartBackgroundSymbol()
{
  return mChartBackgroundSymbol.get();
}

void Qgs2DXyPlot::setChartBackgroundSymbol( QgsFillSymbol *symbol )
{
  mChartBackgroundSymbol.reset( symbol );
}

QgsFillSymbol *Qgs2DXyPlot::chartBorderSymbol()
{
  return mChartBorderSymbol.get();
}

void Qgs2DXyPlot::setChartBorderSymbol( QgsFillSymbol *symbol )
{
  mChartBorderSymbol.reset( symbol );
}

void Qgs2DXyPlot::applyDataDefinedProperties( QgsRenderContext &context, double &minX, double &maxX, double &minY, double &maxY, double &majorIntervalX, double &minorIntervalX, double &labelIntervalX, double &majorIntervalY, double &minorIntervalY, double &labelIntervalY ) const
{
  if ( !dataDefinedProperties().hasActiveProperties() )
  {
    return;
  }

  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::XAxisMinimum ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::XAxisMinimum, context.expressionContext(), minX, &ok );

    if ( ok )
    {
      minX = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::XAxisMaximum ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::XAxisMaximum, context.expressionContext(), maxX, &ok );

    if ( ok )
    {
      maxX = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::YAxisMinimum ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::YAxisMinimum, context.expressionContext(), minY, &ok );

    if ( ok )
    {
      minY = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::YAxisMaximum ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::YAxisMaximum, context.expressionContext(), maxY, &ok );

    if ( ok )
    {
      maxY = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::XAxisMajorInterval ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::XAxisMajorInterval, context.expressionContext(), majorIntervalX, &ok );

    if ( ok )
    {
      majorIntervalX = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::XAxisMinorInterval ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::XAxisMinorInterval, context.expressionContext(), minorIntervalX, &ok );

    if ( ok )
    {
      minorIntervalX = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::XAxisLabelInterval ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::XAxisLabelInterval, context.expressionContext(), labelIntervalX, &ok );

    if ( ok )
    {
      labelIntervalX = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::YAxisMajorInterval ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::YAxisMajorInterval, context.expressionContext(), majorIntervalY, &ok );

    if ( ok )
    {
      majorIntervalY = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::YAxisMinorInterval ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::YAxisMinorInterval, context.expressionContext(), minorIntervalY, &ok );

    if ( ok )
    {
      minorIntervalY = value;
    }
  }
  if ( mDataDefinedProperties.isActive( QgsPlot::DataDefinedProperty::YAxisLabelInterval ) )
  {
    bool ok = false;
    double value = mDataDefinedProperties.valueAsDouble( QgsPlot::DataDefinedProperty::YAxisLabelInterval, context.expressionContext(), labelIntervalY, &ok );

    if ( ok )
    {
      labelIntervalY = value;
    }
  }
}

//
// QgsPlotDefaultSettings
//

QgsNumericFormat *QgsPlotDefaultSettings::axisLabelNumericFormat()
{
  return new QgsBasicNumericFormat();
}

QgsLineSymbol *QgsPlotDefaultSettings::axisGridMajorSymbol()
{
  auto gridMajor = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20, 150 ), 0.1 );
  gridMajor->setPenCapStyle( Qt::FlatCap );
  return new QgsLineSymbol( QgsSymbolLayerList( { gridMajor.release() } ) );
}

QgsLineSymbol *QgsPlotDefaultSettings::axisGridMinorSymbol()
{
  auto gridMinor = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20, 50 ), 0.1 );
  gridMinor->setPenCapStyle( Qt::FlatCap );
  return new QgsLineSymbol( QgsSymbolLayerList( { gridMinor.release() } ) );
}

QgsFillSymbol *QgsPlotDefaultSettings::chartBackgroundSymbol()
{
  auto chartFill = std::make_unique< QgsSimpleFillSymbolLayer >( QColor( 255, 255, 255 ) );
  return new QgsFillSymbol( QgsSymbolLayerList( { chartFill.release() } ) );
}

QgsFillSymbol *QgsPlotDefaultSettings::chartBorderSymbol()
{
  auto chartBorder = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 20, 20, 20 ), 0.1 );
  return new QgsFillSymbol( QgsSymbolLayerList( { chartBorder.release() } ) );
}

QgsMarkerSymbol *QgsPlotDefaultSettings::lineChartMarkerSymbol()
{
  auto chartMarker = std::make_unique< QgsSimpleMarkerSymbolLayer>( Qgis::MarkerShape::Circle, 1.8, 0.0, DEFAULT_SCALE_METHOD, QColor( 89, 150, 50 ) );
  return new QgsMarkerSymbol( QgsSymbolLayerList( { chartMarker.release() } ) );
}

QgsLineSymbol *QgsPlotDefaultSettings::lineChartLineSymbol()
{
  auto chartLine = std::make_unique< QgsSimpleLineSymbolLayer>( QColor( 89, 150, 50, 100 ), 0.6 );
  return new QgsLineSymbol( QgsSymbolLayerList( { chartLine.release() } ) );
}

QgsFillSymbol *QgsPlotDefaultSettings::barChartFillSymbol()
{
  auto chartFill = std::make_unique< QgsSimpleFillSymbolLayer>( QColor( 89, 150, 50 ) );
  return new QgsFillSymbol( QgsSymbolLayerList( { chartFill.release() } ) );
}

QgsFillSymbol *QgsPlotDefaultSettings::pieChartFillSymbol()
{
  auto chartFill = std::make_unique< QgsSimpleFillSymbolLayer>( QColor( 150, 150, 150 ) );
  return new QgsFillSymbol( QgsSymbolLayerList( { chartFill.release() } ) );
}

QgsColorRamp *QgsPlotDefaultSettings::pieChartColorRamp()
{
  return new QgsPresetSchemeColorRamp( { QColor( 89, 150, 50 ), QColor( 228, 26, 28 ), QColor( 55, 126, 184 ), QColor( 152, 78, 163 ), QColor( 255, 127, 0 ), QColor( 166, 86, 40 ), QColor( 247, 129, 191 ), QColor( 153, 153, 153 ) } );
}

QgsNumericFormat *QgsPlotDefaultSettings::pieChartNumericFormat()
{
  return new QgsBasicNumericFormat();
}

//
// QgsPlotData
//

QgsPlotData::~QgsPlotData()
{
  clearSeries();
}

QgsPlotData::QgsPlotData( const QgsPlotData &other )
  : mCategories( other.mCategories )
{
  for ( QgsAbstractPlotSeries *series : other.mSeries )
  {
    addSeries( series->clone() );
  }
}

QgsPlotData::QgsPlotData( QgsPlotData &&other )
  : mSeries( std::move( other.mSeries ) )
  , mCategories( std::move( other.mCategories ) )
{
}

QgsPlotData &QgsPlotData::operator=( const QgsPlotData &other )
{
  if ( this != &other )
  {
    clearSeries();

    mCategories = other.mCategories;
    for ( QgsAbstractPlotSeries *series : other.mSeries )
    {
      addSeries( series->clone() );
    }
  }
  return *this;
}

QgsPlotData &QgsPlotData::operator=( QgsPlotData &&other )
{
  if ( this != &other )
  {
    clearSeries();

    mCategories = std::move( other.mCategories );
    mSeries = std::move( other.mSeries );
  }
  return *this;
}

QList<QgsAbstractPlotSeries *> QgsPlotData::series() const
{
  return mSeries;
}

void QgsPlotData::addSeries( QgsAbstractPlotSeries *series )
{
  if ( !mSeries.contains( series ) )
  {
    mSeries << series;
  }
}

void QgsPlotData::clearSeries()
{
  qDeleteAll( mSeries );
  mSeries.clear();
}

QStringList QgsPlotData::categories() const
{
  return mCategories;
}

void QgsPlotData::setCategories( const QStringList &categories )
{
  mCategories = categories;
}

//
// QgsAbstractPlotSeries
//

QString QgsAbstractPlotSeries::name() const
{
  return mName;
}

void QgsAbstractPlotSeries::setName( const QString &name )
{
  mName = name;
}

//
// QgsXyPlotSeries
//

QList<std::pair<double, double>> QgsXyPlotSeries::data() const
{
  return mData;
}

void QgsXyPlotSeries::setData( const QList<std::pair<double, double>> &data )
{
  mData = data;
}

void QgsXyPlotSeries::append( double x, double y )
{
  mData << std::make_pair( x, y );
}

void QgsXyPlotSeries::clear()
{
  mData.clear();
}

QgsAbstractPlotSeries *QgsXyPlotSeries::clone() const
{
  QgsXyPlotSeries *series = new QgsXyPlotSeries();
  series->setName( name() );
  series->setData( mData );
  return series;
}
