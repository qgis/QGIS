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

  QString firstLabel = firstLabelString( settings );
  QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, format );
  double xOffset = fontMetrics.width( firstLabel ) / 2.0;

  double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );

  double currentLabelNumber = 0.0;

  int nSegmentsLeft = settings.numberOfSegmentsLeft();
  int segmentCounter = 0;
  QString currentNumericLabel;

  QList<double> positions = segmentPositions( scaleContext, settings );

  for ( int i = 0; i < positions.size(); ++i )
  {
    if ( segmentCounter == 0 && nSegmentsLeft > 0 )
    {
      //label first left segment
      currentNumericLabel = firstLabel;
    }
    else if ( segmentCounter != 0 && segmentCounter == nSegmentsLeft ) //reset label number to 0 if there are left segments
    {
      currentLabelNumber = 0;
    }

    if ( segmentCounter >= nSegmentsLeft )
    {
      currentNumericLabel = QString::number( currentLabelNumber / settings.mapUnitsPerScaleBarUnit() );
    }

    if ( segmentCounter == 0 || segmentCounter >= nSegmentsLeft ) //don't draw label for intermediate left segments
    {
      QgsTextRenderer::drawText( QPointF( context.convertToPainterUnits( positions.at( i ), QgsUnitTypes::RenderMillimeters ) + xOffset,
                                          fontMetrics.ascent() + scaledBoxContentSpace ), 0, QgsTextRenderer::AlignCenter,
                                 QStringList() << currentNumericLabel, context, format );
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
    QgsTextRenderer::drawText( QPointF( context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, QgsUnitTypes::RenderMillimeters ) + xOffset
                                        - fontMetrics.width( currentNumericLabel ) / 2.0,
                                        fontMetrics.ascent() + scaledBoxContentSpace ), 0, QgsTextRenderer::AlignLeft,
                               QStringList() << ( currentNumericLabel + ' ' + settings.unitLabel() ), context, format );
  }

  painter->restore();
}

QSizeF QgsScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  QFont font = settings.textFormat().toQFont();

  //consider centered first label
  double firstLabelLeft = QgsLayoutUtils::textWidthMM( font, firstLabelString( settings ) ) / 2;

  //consider last number and label

  double largestLabelNumber = settings.numberOfSegments() * settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit();
  QString largestNumberLabel = QString::number( largestLabelNumber );
  QString largestLabel = QString::number( largestLabelNumber ) + ' ' + settings.unitLabel();
  double largestLabelWidth = QgsLayoutUtils::textWidthMM( font, largestLabel ) - QgsLayoutUtils::textWidthMM( font, largestNumberLabel ) / 2;

  double totalBarLength = scaleContext.segmentWidth * ( settings.numberOfSegments() + ( settings.numberOfSegmentsLeft() > 0 ? 1 : 0 ) );

  double width = firstLabelLeft + totalBarLength + 2 * settings.pen().widthF() + largestLabelWidth + 2 * settings.boxContentSpace();
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

double QgsScaleBarRenderer::firstLabelXOffset( const QgsScaleBarSettings &settings, const QgsRenderContext &context ) const
{
  QString firstLabel = firstLabelString( settings );
  return QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << firstLabel ) / 2.0;
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
