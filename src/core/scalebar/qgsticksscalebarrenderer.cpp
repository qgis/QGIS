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
         Flag::FlagRespectsUnits |
         Flag::FlagRespectsMapUnitsPerScaleBarUnit |
         Flag::FlagUsesUnitLabel |
         Flag::FlagUsesSegments |
         Flag::FlagUsesLabelBarSpace |
         Flag::FlagUsesLabelVerticalPlacement |
         Flag::FlagUsesLabelHorizontalPlacement |
         Flag::FlagUsesSubdivisions;
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
  const double middlePosition = barTopPosition + context.convertToPainterUnits( settings.height() / 2.0, QgsUnitTypes::RenderMillimeters );
  const double bottomPosition = barTopPosition + context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters );

  const double xOffset = firstLabelXOffset( settings, context, scaleContext );

  painter->save();
  if ( context.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );

  std::unique_ptr< QgsLineSymbol > symbol( settings.lineSymbol()->clone() );
  symbol->startRender( context );

  const QList<double> positions = segmentPositions( context, scaleContext, settings );

  // we render the bar symbol-layer-by-symbol-layer, to avoid ugliness where the lines overlap in multi-layer symbols
  for ( int layer = 0; layer < symbol->symbolLayerCount(); ++ layer )
  {
    // first draw the vertical lines for segments
    for ( int i = 0; i < positions.size(); ++i )
    {
      const double thisX = context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset;
      symbol->renderPolyline( QPolygonF() << QPointF( thisX, barTopPosition )
                              << QPointF( thisX, bottomPosition ), nullptr, context, layer );
    }
    // vertical positions
    double verticalPos = 0.0;
    QList<double> subTickPositionsY;
    switch ( mTickPosition )
    {
      case TicksDown:
        verticalPos = barTopPosition;
        subTickPositionsY << barTopPosition;
        subTickPositionsY << middlePosition;
        break;
      case TicksMiddle:
        verticalPos = middlePosition;
        subTickPositionsY << ( barTopPosition + middlePosition ) / 2;
        subTickPositionsY << ( bottomPosition + middlePosition ) / 2;
        break;
      case TicksUp:
        verticalPos = bottomPosition;
        subTickPositionsY << bottomPosition;
        subTickPositionsY << middlePosition;
        break;
    }
    // draw the vertical lines for right subdivisions
    for ( int i = settings.numberOfSegmentsLeft(); i < positions.size(); ++i )
    {
      for ( int j = 1; j < settings.numberOfSubdivisions(); ++j )
      {
        const double thisSubX = context.convertToPainterUnits( positions.at( i ) + j * scaleContext.segmentWidth / settings.numberOfSubdivisions(), QgsUnitTypes::RenderMillimeters ) + xOffset;
        symbol->renderPolyline( QPolygonF() << QPointF( thisSubX, subTickPositionsY.at( 0 ) )
                                << QPointF( thisSubX, subTickPositionsY.at( 1 ) ), nullptr, context, layer );
      }
    }

    //draw last tick and horizontal line
    if ( !positions.isEmpty() )
    {
      double lastTickPositionX = context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, QgsUnitTypes::RenderMillimeters ) + xOffset;

      //last vertical line
      symbol->renderPolyline( QPolygonF() << QPointF( lastTickPositionX, barTopPosition )
                              << QPointF( lastTickPositionX, bottomPosition ),
                              nullptr, context, layer );

      //horizontal line
      symbol->renderPolyline( QPolygonF() << QPointF( xOffset + context.convertToPainterUnits( positions.at( 0 ), QgsUnitTypes::RenderMillimeters ), verticalPos )
                              << QPointF( lastTickPositionX, verticalPos ), nullptr, context, layer );
    }
  }

  symbol->stopRender( context );

  painter->restore();

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}


