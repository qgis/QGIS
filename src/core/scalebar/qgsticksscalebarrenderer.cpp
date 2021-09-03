/***************************************************************************
                            qgsticksscalebarrenderer.cpp
                            ----------------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsticksscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgslayoututils.h"
#include "qgssymbol.h"
#include "qgslinesymbol.h"
#include "qgstextrenderer.h"
#include <QPainter>

QgsTicksScaleBarRenderer::QgsTicksScaleBarRenderer( QgsTicksScaleBarRenderer::TickPosition position )
  : mTickPosition( position )
{

}

QString QgsTicksScaleBarRenderer::id() const
{
  switch ( mTickPosition )
  {
    case TicksUp:
      return QStringLiteral( "Line Ticks Up" );
    case TicksDown:
      return QStringLiteral( "Line Ticks Down" );
    case TicksMiddle:
      return QStringLiteral( "Line Ticks Middle" );
  }
  return QString();  // to make gcc happy
}

QString QgsTicksScaleBarRenderer::visibleName() const
{
  switch ( mTickPosition )
  {
    case TicksUp:
      return QObject::tr( "Line Ticks Up" );
    case TicksDown:
      return QObject::tr( "Line Ticks Down" );
    case TicksMiddle:
      return QObject::tr( "Line Ticks Middle" );
  }
  return QString();  // to make gcc happy

}

int QgsTicksScaleBarRenderer::sortKey() const
{
  switch ( mTickPosition )
  {
    case TicksUp:
      return 5;
    case TicksDown:
      return 4;
    case TicksMiddle:
      return 3;
  }
  return 6;
}

QgsScaleBarRenderer::Flags QgsTicksScaleBarRenderer::flags() const
{
  return Flag::FlagUsesLineSymbol |
         Flag::FlagUsesDivisionSymbol |
         Flag::FlagUsesSubdivisionSymbol |
         Flag::FlagRespectsUnits |
         Flag::FlagRespectsMapUnitsPerScaleBarUnit |
         Flag::FlagUsesUnitLabel |
         Flag::FlagUsesSegments |
         Flag::FlagUsesLabelBarSpace |
         Flag::FlagUsesLabelVerticalPlacement |
         Flag::FlagUsesLabelHorizontalPlacement |
         Flag::FlagUsesSubdivisions |
         Flag::FlagUsesSubdivisionsHeight;
}

QgsTicksScaleBarRenderer *QgsTicksScaleBarRenderer::clone() const
{
  return new QgsTicksScaleBarRenderer( * this );
}

void QgsTicksScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
    return;

  QPainter *painter = context.painter();

  const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), QgsUnitTypes::RenderMillimeters );
  const double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, settings.textFormat() );
  const double barTopPosition = scaledBoxContentSpace + ( settings.labelVerticalPlacement() == QgsScaleBarSettings::LabelAboveSegment ? fontMetrics.ascent() + scaledLabelBarSpace : 0 );
  const double scaledHeight = context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters );
  const double scaledSubdivisionsHeight = context.convertToPainterUnits( settings.subdivisionsHeight(), QgsUnitTypes::RenderMillimeters );
  const double scaledMaxHeight = ( ( settings.numberOfSubdivisions() > 1 ) && ( scaledSubdivisionsHeight > scaledHeight ) ) ? scaledSubdivisionsHeight : scaledHeight;
  const double middlePosition = barTopPosition + scaledMaxHeight / 2.0;
  const double bottomPosition = barTopPosition + scaledMaxHeight;

  const double xOffset = firstLabelXOffset( settings, context, scaleContext );

  painter->save();
  context.setPainterFlagsUsingContext( painter );

  std::unique_ptr< QgsLineSymbol > symbol( settings.lineSymbol()->clone() );
  symbol->startRender( context );

  std::unique_ptr< QgsLineSymbol > divisionSymbol( settings.divisionLineSymbol()->clone() );
  divisionSymbol->startRender( context );

  std::unique_ptr< QgsLineSymbol > subdivisionSymbol( settings.subdivisionLineSymbol()->clone() );
  subdivisionSymbol->startRender( context );

  const QList<double> positions = segmentPositions( context, scaleContext, settings );

  // vertical positions
  double verticalPos = 0.0;
  QList<double> subTickPositionsY;
  QList<double> tickPositionsY;
  switch ( mTickPosition )
  {
    case TicksDown:
      verticalPos = barTopPosition;
      subTickPositionsY << verticalPos;
      subTickPositionsY << verticalPos + scaledSubdivisionsHeight;
      tickPositionsY << verticalPos;
      tickPositionsY << verticalPos + scaledHeight;
      break;
    case TicksMiddle:
      verticalPos = middlePosition;
      subTickPositionsY << verticalPos + scaledSubdivisionsHeight / 2.0;
      subTickPositionsY << verticalPos - scaledSubdivisionsHeight / 2.0;
      tickPositionsY << verticalPos + scaledHeight / 2.0;
      tickPositionsY << verticalPos - scaledHeight / 2.0;
      break;
    case TicksUp:
      verticalPos = bottomPosition;
      subTickPositionsY << verticalPos;
      subTickPositionsY << verticalPos - scaledSubdivisionsHeight;
      tickPositionsY << verticalPos;
      tickPositionsY << verticalPos - scaledHeight;
      break;
  }

  int symbolLayerCount = symbol->symbolLayerCount();
  symbolLayerCount = std::max( symbolLayerCount, divisionSymbol->symbolLayerCount() );
  symbolLayerCount = std::max( symbolLayerCount, subdivisionSymbol->symbolLayerCount() );

  // we render the bar symbol-layer-by-symbol-layer, to avoid ugliness where the lines overlap in multi-layer symbols
  for ( int layer = 0; layer < symbolLayerCount; ++ layer )
  {
    const bool drawDivisionsForThisSymbolLayer = layer < divisionSymbol->symbolLayerCount();
    const bool drawSubdivisionsForThisSymbolLayer = layer < subdivisionSymbol->symbolLayerCount();
    const bool drawLineForThisSymbolLayer = layer < symbol->symbolLayerCount();

    if ( drawDivisionsForThisSymbolLayer )
    {
      // first draw the vertical lines for segments
      for ( int i = 0; i < positions.size(); ++i )
      {
        const double thisX = context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset;
        divisionSymbol->renderPolyline( QPolygonF() << QPointF( thisX, tickPositionsY.at( 0 ) )
                                        << QPointF( thisX, tickPositionsY.at( 1 ) ), nullptr, context, layer );
      }
    }

    // draw the vertical lines for right subdivisions
    if ( drawSubdivisionsForThisSymbolLayer )
    {
      for ( int i = settings.numberOfSegmentsLeft(); i < positions.size(); ++i )
      {
        for ( int j = 1; j < settings.numberOfSubdivisions(); ++j )
        {
          const double thisSubX = context.convertToPainterUnits( positions.at( i ) + j * scaleContext.segmentWidth / settings.numberOfSubdivisions(), QgsUnitTypes::RenderMillimeters ) + xOffset;
          subdivisionSymbol->renderPolyline( QPolygonF() << QPointF( thisSubX, subTickPositionsY.at( 0 ) )
                                             << QPointF( thisSubX, subTickPositionsY.at( 1 ) ), nullptr, context, layer );
        }
      }
    }

    //draw last tick and horizontal line
    if ( !positions.isEmpty() )
    {
      const double lastTickPositionX = context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, QgsUnitTypes::RenderMillimeters ) + xOffset;

      //last vertical line
      if ( drawDivisionsForThisSymbolLayer )
      {
        divisionSymbol->renderPolyline( QPolygonF() << QPointF( lastTickPositionX, tickPositionsY.at( 0 ) )
                                        << QPointF( lastTickPositionX, tickPositionsY.at( 1 ) ),
                                        nullptr, context, layer );
      }

      //horizontal line
      if ( drawLineForThisSymbolLayer )
      {
        symbol->renderPolyline( QPolygonF() << QPointF( xOffset + context.convertToPainterUnits( positions.at( 0 ), QgsUnitTypes::RenderMillimeters ), verticalPos )
                                << QPointF( lastTickPositionX, verticalPos ), nullptr, context, layer );
      }
    }
  }

  symbol->stopRender( context );
  divisionSymbol->stopRender( context );
  subdivisionSymbol->stopRender( context );

  painter->restore();

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}


