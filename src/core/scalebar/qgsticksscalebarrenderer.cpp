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
#include <QPainter>

QString QgsTicksScaleBarRenderer::name() const
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

void QgsTicksScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
    return;

  QPainter *painter = context.painter();

  double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), QgsUnitTypes::RenderMillimeters );
  double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, settings.textFormat() );
  double barTopPosition = scaledBoxContentSpace + ( settings.labelVerticalPlacement() == QgsScaleBarSettings::LabelAboveSegment ? fontMetrics.ascent() + scaledLabelBarSpace : 0 );
  double middlePosition = barTopPosition + context.convertToPainterUnits( settings.height() / 2.0, QgsUnitTypes::RenderMillimeters );
  double bottomPosition = barTopPosition + context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters );

  double xOffset = firstLabelXOffset( settings, context, scaleContext );

  painter->save();
  if ( context.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );

  QPen pen = settings.pen();
  pen.setWidthF( context.convertToPainterUnits( pen.widthF(), QgsUnitTypes::RenderMillimeters ) );
  painter->setPen( pen );

  QList<double> positions = segmentPositions( scaleContext, settings );

  for ( int i = 0; i < positions.size(); ++i )
  {
    painter->drawLine( QLineF( context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset, barTopPosition,
                               context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset,
                               barTopPosition + context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters ) ) );
  }

  //draw last tick and horizontal line
  if ( !positions.isEmpty() )
  {
    double lastTickPositionX = context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, QgsUnitTypes::RenderMillimeters ) + xOffset;
    double verticalPos = 0.0;
    switch ( mTickPosition )
    {
      case TicksDown:
        verticalPos = barTopPosition;
        break;
      case TicksMiddle:
        verticalPos = middlePosition;
        break;
      case TicksUp:
        verticalPos = bottomPosition;
        break;
    }
    //horizontal line
    painter->drawLine( QLineF( xOffset + context.convertToPainterUnits( positions.at( 0 ), QgsUnitTypes::RenderMillimeters ),
                               verticalPos, lastTickPositionX, verticalPos ) );
    //last vertical line
    painter->drawLine( QLineF( lastTickPositionX, barTopPosition, lastTickPositionX, barTopPosition + context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters ) ) );
  }

  painter->restore();

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}


