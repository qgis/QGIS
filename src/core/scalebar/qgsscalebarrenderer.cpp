/***************************************************************************
                            qgsscalebarrenderer.cpp
                            -----------------------
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

#include "qgsscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgslayoututils.h"
#include "qgstextrenderer.h"
#include "qgsexpressioncontextutils.h"
#include <QFontMetricsF>
#include <QPainter>

void QgsScaleBarRenderer::drawDefaultLabels( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter *painter = context.painter();

  painter->save();

  QgsTextFormat format = settings.textFormat();

  QgsExpressionContextScope *scaleScope = new QgsExpressionContextScope( QStringLiteral( "scalebar_text" ) );
  QgsExpressionContextScopePopper scopePopper( context.expressionContext(), scaleScope );

  QString firstLabel = firstLabelString( settings );
  QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, format );
  double xOffset = fontMetrics.width( firstLabel ) / 2.0;

  double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), QgsUnitTypes::RenderMillimeters );
  double scaledHeight = context.convertToPainterUnits( settings.height(), QgsUnitTypes::RenderMillimeters );

  double currentLabelNumber = 0.0;

  int nSegmentsLeft = settings.numberOfSegmentsLeft();
  int segmentCounter = 0;

  QString currentNumericLabel;
  QList<double> positions = segmentPositions( scaleContext, settings );

  bool drawZero = true;
  switch ( settings.labelHorizontalPlacement() )
  {
    case QgsScaleBarSettings::LabelCenteredSegment:
      drawZero = false;
      break;
    case QgsScaleBarSettings::LabelCenteredEdge:
      drawZero = true;
      break;
  }

  for ( int i = 0; i < positions.size(); ++i )
  {
    if ( segmentCounter == 0 && nSegmentsLeft > 0 )
    {
      //label first left segment
      currentNumericLabel = firstLabel;
    }
    else if ( segmentCounter != 0 && segmentCounter == nSegmentsLeft ) //reset label number to 0 if there are left segments
    {
      currentLabelNumber = 0.0;
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentNumericLabel = QString::number( currentLabelNumber / settings.mapUnitsPerScaleBarUnit() );
    }

    //don't draw label for intermediate left segments or the zero label when it needs to be skipped
    if ( ( segmentCounter == 0 || segmentCounter >= nSegmentsLeft ) && ( currentNumericLabel != QStringLiteral( "0" ) || drawZero ) )
    {
      scaleScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "scale_value" ), currentNumericLabel, true, false ) );
      QPointF pos;
      if ( settings.labelHorizontalPlacement() == QgsScaleBarSettings::LabelCenteredSegment )
      {
        if ( segmentCounter == 0 )
        {
          // if the segment counter is zero with a non zero label, this is the left-of-zero label
          pos.setX( context.convertToPainterUnits( positions.at( i ) + ( scaleContext.segmentWidth / 2 ), QgsUnitTypes::RenderMillimeters ) );
        }
        else
        {
          pos.setX( context.convertToPainterUnits( positions.at( i ) - ( scaleContext.segmentWidth / 2 ), QgsUnitTypes::RenderMillimeters ) );
        }
      }
      else
      {
        pos.setX( context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset );
      }
      pos.setY( fontMetrics.ascent() + scaledBoxContentSpace + ( settings.labelVerticalPlacement() == QgsScaleBarSettings::LabelBelowSegment ? scaledHeight + scaledLabelBarSpace : 0 ) );
      QgsTextRenderer::drawText( pos, 0, QgsTextRenderer::AlignCenter, QStringList() << currentNumericLabel, context, format );
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentLabelNumber += settings.unitsPerSegment();
    }
    ++segmentCounter;
  }

  //also draw the last label
  if ( !positions.isEmpty() )
  {
    // note: this label is NOT centered over the end of the bar - rather the numeric portion
    // of it is, without considering the unit label suffix. That's drawn at the end after
    // horizontally centering just the numeric portion.
    currentNumericLabel = QString::number( currentLabelNumber / settings.mapUnitsPerScaleBarUnit() );
    scaleScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "scale_value" ), currentNumericLabel, true, false ) );
    QPointF pos;
    pos.setY( fontMetrics.ascent() + scaledBoxContentSpace + ( settings.labelVerticalPlacement() == QgsScaleBarSettings::LabelBelowSegment ? scaledHeight + scaledLabelBarSpace : 0 ) );
    if ( settings.labelHorizontalPlacement() == QgsScaleBarSettings::LabelCenteredSegment )
    {
      pos.setX( context.convertToPainterUnits( positions.at( positions.size() - 1 ) + ( scaleContext.segmentWidth / 2 ), QgsUnitTypes::RenderMillimeters ) + xOffset );
      QgsTextRenderer::drawText( pos, 0, QgsTextRenderer::AlignCenter, QStringList() << ( currentNumericLabel + ' ' + settings.unitLabel() ), context, format );
    }
    else
    {
      pos.setX( context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, QgsUnitTypes::RenderMillimeters ) + xOffset
                - fontMetrics.width( currentNumericLabel ) / 2.0 );
      QgsTextRenderer::drawText( pos, 0, QgsTextRenderer::AlignLeft, QStringList() << ( currentNumericLabel + ' ' + settings.unitLabel() ), context, format );
    }
  }

  painter->restore();
}

QSizeF QgsScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  QFont font = settings.textFormat().toQFont();

  //consider centered first label
  double firstLabelWidth = QgsLayoutUtils::textWidthMM( font, firstLabelString( settings ) );
  if ( settings.labelHorizontalPlacement() == QgsScaleBarSettings::LabelCenteredSegment )
  {
    if ( firstLabelWidth > scaleContext.segmentWidth )
    {
      firstLabelWidth = ( firstLabelWidth - scaleContext.segmentWidth ) / 2;
    }
    else
    {
      firstLabelWidth = 0.0;
    }
  }
  else
  {
    firstLabelWidth = firstLabelWidth / 2;
  }

  //consider last number and label
  double largestLabelNumber = settings.numberOfSegments() * settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit();
  QString largestNumberLabel = QString::number( largestLabelNumber );
  QString largestLabel = largestNumberLabel + ' ' + settings.unitLabel();
  double largestLabelWidth;
  if ( settings.labelHorizontalPlacement() == QgsScaleBarSettings::LabelCenteredSegment )
  {
    largestLabelWidth = QgsLayoutUtils::textWidthMM( font, largestLabel );
    if ( largestLabelWidth > scaleContext.segmentWidth )
    {
      largestLabelWidth = ( largestLabelWidth - scaleContext.segmentWidth ) / 2;
    }
    else
    {
      largestLabelWidth = 0.0;
    }
  }
  else
  {
    largestLabelWidth = QgsLayoutUtils::textWidthMM( font, largestLabel ) - QgsLayoutUtils::textWidthMM( font, largestNumberLabel ) / 2;
  }

  double totalBarLength = scaleContext.segmentWidth * ( settings.numberOfSegments() + ( settings.numberOfSegmentsLeft() > 0 ? 1 : 0 ) );

  double width = firstLabelWidth + totalBarLength + 2 * settings.pen().widthF() + largestLabelWidth + 2 * settings.boxContentSpace();
  double height = settings.height() + settings.labelBarSpace() + 2 * settings.boxContentSpace() + QgsLayoutUtils::fontAscentMM( font );

  return QSizeF( width, height );
}

QString QgsScaleBarRenderer::firstLabelString( const QgsScaleBarSettings &settings ) const
{
  if ( settings.numberOfSegmentsLeft() > 0 )
  {
    return QString::number( settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit() );
  }
  else
  {
    return QStringLiteral( "0" );
  }
}

double QgsScaleBarRenderer::firstLabelXOffset( const QgsScaleBarSettings &settings ) const
{
  QString firstLabel = firstLabelString( settings );
  Q_NOWARN_DEPRECATED_PUSH
  return QgsLayoutUtils::textWidthMM( settings.font(), firstLabel ) / 2.0;
  Q_NOWARN_DEPRECATED_POP
}

double QgsScaleBarRenderer::firstLabelXOffset( const QgsScaleBarSettings &settings, const QgsRenderContext &context, const ScaleBarContext &scaleContext ) const
{
  QString firstLabel = firstLabelString( settings );
  double firstLabelWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << firstLabel );
  if ( settings.labelHorizontalPlacement() == QgsScaleBarSettings::LabelCenteredSegment )
  {
    if ( firstLabelWidth > scaleContext.segmentWidth )
    {
      firstLabelWidth = ( firstLabelWidth - scaleContext.segmentWidth ) / 2;
    }
    else
    {
      firstLabelWidth = 0.0;
    }
  }
  else
  {
    firstLabelWidth = firstLabelWidth / 2;
  }
  return firstLabelWidth;
}

QList<double> QgsScaleBarRenderer::segmentPositions( const ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const
{
  QList<double> positions;

  double currentXCoord = settings.pen().widthF() + settings.boxContentSpace();

  //left segments
  double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
  positions.reserve( settings.numberOfSegmentsLeft() + settings.numberOfSegments() );
  for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += leftSegmentSize;
  }

  //right segments
  for ( int i = 0; i < settings.numberOfSegments(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += scaleContext.segmentWidth;
  }
  return positions;
}

QList<double> QgsScaleBarRenderer::segmentWidths( const ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const
{
  QList<double> widths;
  widths.reserve( settings.numberOfSegmentsLeft() + settings.numberOfSegments() );

  //left segments
  if ( settings.numberOfSegmentsLeft() > 0 )
  {
    double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
    for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
    {
      widths << leftSegmentSize;
    }
  }

  //right segments
  for ( int i = 0; i < settings.numberOfSegments(); ++i )
  {
    widths << scaleContext.segmentWidth;
  }

  return widths;
}
