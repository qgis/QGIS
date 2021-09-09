/***************************************************************************
                            qgssteppedlinescalebarrenderer.cpp
                            --------------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgssteppedlinescalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgslayoututils.h"
#include "qgssymbol.h"
#include "qgstextrenderer.h"
#include "qgslinesymbol.h"
#include <QList>
#include <QPainter>

QString QgsSteppedLineScaleBarRenderer::id() const
{
  return QStringLiteral( "stepped" );
}

QString QgsSteppedLineScaleBarRenderer::visibleName() const
{
  return QObject::tr( "Stepped Line" );
}

int QgsSteppedLineScaleBarRenderer::sortKey() const
{
  return 7;
}

QgsScaleBarRenderer::Flags QgsSteppedLineScaleBarRenderer::flags() const
{
  return Flag::FlagUsesLineSymbol |
         Flag::FlagRespectsUnits |
         Flag::FlagRespectsMapUnitsPerScaleBarUnit |
         Flag::FlagUsesUnitLabel |
         Flag::FlagUsesSegments |
         Flag::FlagUsesLabelBarSpace |
         Flag::FlagUsesLabelVerticalPlacement |
         Flag::FlagUsesLabelHorizontalPlacement;
}

QgsSteppedLineScaleBarRenderer *QgsSteppedLineScaleBarRenderer::clone() const
{
  return new QgsSteppedLineScaleBarRenderer( *this );
}

void QgsSteppedLineScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }
  QPainter *painter = context.painter();

  std::unique_ptr< QgsLineSymbol > sym( settings.lineSymbol()->clone() );
  sym->startRender( context ) ;

  const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), QgsUnitTypes::RenderMillimeters );
  const double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, settings.textFormat() );
  const double barTopPosition = scaledBoxContentSpace + ( settings.labelVerticalPlacement() == QgsScaleBarSettings::LabelAboveSegment ? fontMetrics.ascent() + scaledLabelBarSpace : 0 );
  const double barBottomPosition = barTopPosition + context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters );

  painter->save();
  context.setPainterFlagsUsingContext( painter );

  painter->setPen( Qt::NoPen );

  const double xOffset = firstLabelXOffset( settings, context, scaleContext );

  const QList<double> positions = segmentPositions( context, scaleContext, settings );
  const QList<double> widths = segmentWidths( scaleContext, settings );

  QPolygonF points;

  for ( int i = 0; i < positions.size() + 1; ++i )
  {
    // we render one extra place, corresponding to the final position + width (i.e. the "end" of the bar)
    const double x = i < positions.size() ? context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset
                     : context.convertToPainterUnits( positions.at( i - 1 ), QgsUnitTypes::RenderMillimeters ) + xOffset + context.convertToPainterUnits( widths.at( i - 1 ), QgsUnitTypes::RenderMillimeters );
    if ( i % 2 == 0 )
    {
      points << QPointF( x, barBottomPosition ) << QPointF( x, barTopPosition );
    }
    else
    {
      points << QPointF( x, barTopPosition ) << QPointF( x, barBottomPosition ) ;
    }
  }

  sym->renderPolyline( points, nullptr, context );

  painter->restore();

  sym->stopRender( context );

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}



