/***************************************************************************
                            qgshollowscalebarrenderer.cpp
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

#include "qgshollowscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgslayoututils.h"
#include "qgssymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgstextrenderer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include <QList>
#include <QPainter>

QString QgsHollowScaleBarRenderer::id() const
{
  return QStringLiteral( "hollow" );
}

QString QgsHollowScaleBarRenderer::visibleName() const
{
  return QObject::tr( "Hollow" );
}

QgsScaleBarRenderer::Flags QgsHollowScaleBarRenderer::flags() const
{
  return Flag::FlagUsesLineSymbol |
         Flag::FlagUsesFillSymbol |
         Flag::FlagUsesAlternateFillSymbol |
         Flag::FlagRespectsUnits |
         Flag::FlagRespectsMapUnitsPerScaleBarUnit |
         Flag::FlagUsesUnitLabel |
         Flag::FlagUsesSegments |
         Flag::FlagUsesLabelBarSpace |
         Flag::FlagUsesLabelVerticalPlacement |
         Flag::FlagUsesLabelHorizontalPlacement;
}

int QgsHollowScaleBarRenderer::sortKey() const
{
  return 8;
}

QgsHollowScaleBarRenderer *QgsHollowScaleBarRenderer::clone() const
{
  return new QgsHollowScaleBarRenderer( *this );
}

void QgsHollowScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }
  QPainter *painter = context.painter();

  const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), QgsUnitTypes::RenderMillimeters );
  const double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, settings.textFormat() );
  const double barTopPosition = scaledBoxContentSpace + ( settings.labelVerticalPlacement() == QgsScaleBarSettings::LabelAboveSegment ? fontMetrics.ascent() + scaledLabelBarSpace : 0 );
  const double barHeight = context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters );

  painter->save();
  context.setPainterFlagsUsingContext( painter );

  std::unique_ptr< QgsLineSymbol > lineSymbol( settings.lineSymbol()->clone() );
  lineSymbol->startRender( context );

  std::unique_ptr< QgsFillSymbol > fillSymbol1( settings.fillSymbol()->clone() );
  fillSymbol1->startRender( context );

  std::unique_ptr< QgsFillSymbol > fillSymbol2( settings.alternateFillSymbol()->clone() );
  fillSymbol2->startRender( context );

  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  bool useColor = true; //alternate brush color/white
  const double xOffset = firstLabelXOffset( settings, context, scaleContext );

  const QList<double> positions = segmentPositions( context, scaleContext, settings );
  const QList<double> widths = segmentWidths( scaleContext, settings );

  // draw the fill
  double minX = 0;
  double maxX = 0;
  QgsFillSymbol *currentSymbol = nullptr;
  for ( int i = 0; i < positions.size(); ++i )
  {
    if ( useColor ) //alternating colors
    {
      currentSymbol = fillSymbol1.get();
    }
    else //secondary fill
    {
      currentSymbol = fillSymbol2.get();
    }

    const double thisX = context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset;
    const double thisWidth = context.convertToPainterUnits( widths.at( i ), QgsUnitTypes::RenderMillimeters );

    if ( i == 0 )
      minX = thisX;
    if ( i == positions.size() - 1 )
      maxX = thisX + thisWidth;

    const QRectF segmentRect( thisX, barTopPosition, thisWidth, barHeight );
    currentSymbol->renderPolygon( QPolygonF()
                                  << segmentRect.topLeft()
                                  << segmentRect.topRight()
                                  << segmentRect.bottomRight()
                                  << segmentRect.bottomLeft()
                                  << segmentRect.topLeft(), nullptr, nullptr, context );
    useColor = !useColor;
  }

  // and then the lines
  // note that we do this layer-by-layer, to avoid ugliness where the lines touch the outer rect
  for ( int layer = 0; layer < lineSymbol->symbolLayerCount(); ++layer )
  {
    // horizontal lines
    bool drawLine = false;
    for ( int i = 0; i < positions.size(); ++i )
    {
      drawLine = !drawLine;
      if ( !drawLine )
        continue;

      const double lineX = context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset;
      const double lineLength = context.convertToPainterUnits( widths.at( i ), QgsUnitTypes::RenderMillimeters );
      lineSymbol->renderPolyline( QPolygonF()
                                  << QPointF( lineX, barTopPosition + barHeight / 2.0 )
                                  << QPointF( lineX + lineLength, barTopPosition + barHeight / 2.0 ),
                                  nullptr, context, layer );
    }

    // vertical lines
    for ( int i = 1; i < positions.size(); ++i )
    {
      const double lineX = context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset;
      lineSymbol->renderPolyline( QPolygonF()
                                  << QPointF( lineX, barTopPosition )
                                  << QPointF( lineX, barTopPosition + barHeight ),
                                  nullptr, context, layer );
    }

    // outside line
    lineSymbol->renderPolyline( QPolygonF()
                                << QPointF( minX, barTopPosition )
                                << QPointF( maxX, barTopPosition )
                                << QPointF( maxX, barTopPosition + barHeight )
                                << QPointF( minX, barTopPosition + barHeight )
                                << QPointF( minX, barTopPosition ),
                                nullptr, context, layer );
  }

  lineSymbol->stopRender( context );
  fillSymbol1->stopRender( context );
  fillSymbol2->stopRender( context );
  painter->restore();

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}

bool QgsHollowScaleBarRenderer::applyDefaultSettings( QgsScaleBarSettings &settings ) const
{
  // null the fill symbols by default
  std::unique_ptr< QgsFillSymbol > fillSymbol = std::make_unique< QgsFillSymbol >();
  std::unique_ptr< QgsSimpleFillSymbolLayer > fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
  fillSymbolLayer->setColor( QColor( 0, 0, 0 ) );
  fillSymbolLayer->setBrushStyle( Qt::NoBrush );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  fillSymbol->changeSymbolLayer( 0, fillSymbolLayer->clone() );
  settings.setFillSymbol( fillSymbol.release() );

  fillSymbol = std::make_unique< QgsFillSymbol >();
  fillSymbolLayer->setColor( QColor( 255, 255, 255 ) );
  fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
  settings.setAlternateFillSymbol( fillSymbol.release() );

  return true;
}



