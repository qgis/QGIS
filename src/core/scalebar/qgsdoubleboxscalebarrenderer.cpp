/***************************************************************************
                            qgsdoubleboxscalebarrenderer.cpp
                            --------------------------------
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

#include "qgsdoubleboxscalebarrenderer.h"

#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbol.h"
#include "qgsscalebarsettings.h"
#include "qgssymbol.h"
#include "qgstextrenderer.h"

#include <QList>
#include <QPainter>

QString QgsDoubleBoxScaleBarRenderer::id() const
{
  return u"Double Box"_s;
}

QString QgsDoubleBoxScaleBarRenderer::visibleName() const
{
  return QObject::tr( "Double Box" );
}

QgsScaleBarRenderer::Flags QgsDoubleBoxScaleBarRenderer::flags() const
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

int QgsDoubleBoxScaleBarRenderer::sortKey() const
{
  return 2;
}

QgsDoubleBoxScaleBarRenderer *QgsDoubleBoxScaleBarRenderer::clone() const
{
  return new QgsDoubleBoxScaleBarRenderer( *this );
}

void QgsDoubleBoxScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }
  QPainter *painter = context.painter();

  const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), Qgis::RenderUnit::Millimeters );
  const double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), Qgis::RenderUnit::Millimeters );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, settings.textFormat() );
  const double barTopPosition = scaledBoxContentSpace + ( settings.labelVerticalPlacement() == Qgis::ScaleBarDistanceLabelVerticalPlacement::AboveSegment ? fontMetrics.ascent() + scaledLabelBarSpace : 0 );
  const double segmentHeight = context.convertToPainterUnits( settings.height() / 2, Qgis::RenderUnit::Millimeters );

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

  double minX = 0;
  double maxX = 0;
  QgsFillSymbol *currentSymbol = nullptr;
  for ( int i = 0; i < positions.size(); ++i )
  {
    //draw top half
    if ( useColor )
    {
      currentSymbol = fillSymbol1.get();
    }
    else //secondary symbol
    {
      currentSymbol = fillSymbol2.get();
    }

    const double thisX = context.convertToPainterUnits( positions.at( i ), Qgis::RenderUnit::Millimeters ) + xOffset;
    const double thisWidth = context.convertToPainterUnits( widths.at( i ), Qgis::RenderUnit::Millimeters );

    if ( i == 0 )
      minX = thisX;
    if ( i == positions.size() - 1 )
      maxX = thisX + thisWidth;

    const QRectF segmentRectTop( thisX, barTopPosition, thisWidth, segmentHeight );
    currentSymbol->renderPolygon( QPolygonF()
                                  << segmentRectTop.topLeft()
                                  << segmentRectTop.topRight()
                                  << segmentRectTop.bottomRight()
                                  << segmentRectTop.bottomLeft()
                                  << segmentRectTop.topLeft(),
                                  nullptr, nullptr, context );
    painter->drawRect( segmentRectTop );

    //draw bottom half
    if ( useColor )
    {
      //secondary symbol
      currentSymbol = fillSymbol2.get();
    }
    else //primary symbol
    {
      currentSymbol = fillSymbol1.get(); ;
    }

    const QRectF segmentRectBottom( thisX, barTopPosition + segmentHeight, thisWidth, segmentHeight );

    currentSymbol->renderPolygon( QPolygonF()
                                  << segmentRectBottom.topLeft()
                                  << segmentRectBottom.topRight()
                                  << segmentRectBottom.bottomRight()
                                  << segmentRectBottom.bottomLeft()
                                  << segmentRectBottom.topLeft(),
                                  nullptr, nullptr, context );
    useColor = !useColor;
  }


  // and then the lines
  // note that we do this layer-by-layer, to avoid ugliness where the lines touch the outer rect
  for ( int layer = 0; layer < lineSymbol->symbolLayerCount(); ++layer )
  {
    // vertical lines
    for ( int i = 1; i < positions.size(); ++i )
    {
      const double lineX = context.convertToPainterUnits( positions.at( i ), Qgis::RenderUnit::Millimeters ) + xOffset;
      lineSymbol->renderPolyline( QPolygonF()
                                  << QPointF( lineX, barTopPosition )
                                  << QPointF( lineX, barTopPosition + segmentHeight * 2 ),
                                  nullptr, context, layer );
    }

    // middle horizontal line
    lineSymbol->renderPolyline( QPolygonF()
                                << QPointF( minX, barTopPosition + segmentHeight )
                                << QPointF( maxX, barTopPosition + segmentHeight ),
                                nullptr, context, layer );


    // outside line
    lineSymbol->renderPolyline( QPolygonF()
                                << QPointF( minX, barTopPosition )
                                << QPointF( maxX, barTopPosition )
                                << QPointF( maxX, barTopPosition + segmentHeight * 2 )
                                << QPointF( minX, barTopPosition + segmentHeight * 2 )
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

bool QgsDoubleBoxScaleBarRenderer::applyDefaultSettings( QgsScaleBarSettings &settings ) const
{
  QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( settings.fillSymbol()->symbolLayers().at( 0 ) );

  // restore the fill symbols by default
  if ( fill && fill->brushStyle() == Qt::NoBrush )
  {
    auto fillSymbol = std::make_unique< QgsFillSymbol >();
    auto fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
    fillSymbolLayer->setColor( QColor( 0, 0, 0 ) );
    fillSymbolLayer->setBrushStyle( Qt::SolidPattern );
    fillSymbolLayer->setStrokeStyle( Qt::SolidLine );
    fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
    settings.setFillSymbol( fillSymbol.release() );

    fillSymbol = std::make_unique< QgsFillSymbol >();
    fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
    fillSymbolLayer->setColor( QColor( 255, 255, 255 ) );
    fillSymbolLayer->setStrokeStyle( Qt::NoPen );
    fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
    settings.setAlternateFillSymbol( fillSymbol.release() );
  }

  return true;
}
