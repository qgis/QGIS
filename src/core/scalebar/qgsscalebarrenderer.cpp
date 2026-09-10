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

#include "qgsexpressioncontextutils.h"
#include "qgslayoututils.h"
#include "qgslinesymbol.h"
#include "qgsnumericformat.h"
#include "qgsscalebarsettings.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QString>

using namespace Qt::StringLiterals;

void QgsScaleBarRenderer::drawDefaultLabels( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter *painter = context.painter();

  painter->save();

  const QgsTextFormat format = settings.textFormat();

  QgsExpressionContextScope *scaleScope = new QgsExpressionContextScope( u"scalebar_text"_s );
  const QgsExpressionContextScopePopper scopePopper( context.expressionContext(), scaleScope );

  const QString firstLabel = firstLabelString( settings );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, format );
  const double xOffset = firstLabelXOffset( settings, context, scaleContext );

  const double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), Qgis::RenderUnit::Millimeters );
  const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), Qgis::RenderUnit::Millimeters );
  double scaledHeight;
  if ( ( scaleContext.flags & Flag::FlagUsesSubdivisionsHeight ) && ( settings.numberOfSubdivisions() > 1 ) && ( settings.subdivisionsHeight() > settings.height() ) )
  {
    scaledHeight = context.convertToPainterUnits( settings.subdivisionsHeight(), Qgis::RenderUnit::Millimeters );
  }
  else
  {
    scaledHeight = context.convertToPainterUnits( settings.height(), Qgis::RenderUnit::Millimeters );
  }

  const Qgis::ScaleBarUnitLabelPlacements unitPlacements = settings.unitLabelPlacements();
  const QString unitLabel = settings.unitLabel();
  const bool hasUnitLabel = !unitLabel.isEmpty();

  const bool hasUnitLabelAbove
    = hasUnitLabel
      && ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightAbove ) );

  const bool hasDistanceLabelsAbove = ( settings.labelVerticalPlacement() == Qgis::ScaleBarDistanceLabelVerticalPlacement::AboveSegment );
  const bool hasDistanceLabelsBelow = ( settings.labelVerticalPlacement() == Qgis::ScaleBarDistanceLabelVerticalPlacement::BelowSegment );

  double barTopPosition = scaledBoxContentSpace;
  if ( hasUnitLabelAbove )
  {
    barTopPosition += fontMetrics.ascent() + scaledLabelBarSpace;
  }
  if ( hasDistanceLabelsAbove )
  {
    barTopPosition += fontMetrics.ascent() + scaledLabelBarSpace;
  }

  const double barBottomPosition = barTopPosition + scaledHeight;
  const double barMiddleY = barTopPosition + scaledHeight / 2.0;

  double distanceLabelY = 0.0;
  if ( hasDistanceLabelsAbove )
  {
    distanceLabelY = barTopPosition - scaledLabelBarSpace;
  }
  else if ( hasDistanceLabelsBelow )
  {
    distanceLabelY = barBottomPosition + scaledLabelBarSpace + fontMetrics.ascent();
  }

  const QList<double> positions = segmentPositions( context, scaleContext, settings );
  const int nSegmentsLeft = settings.numberOfSegmentsLeft();
  const QgsNumericFormatContext numericContext;

  switch ( settings.labelHorizontalPlacement() )
  {
    case Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment:
    {
      for ( int i = 0; i < positions.size(); ++i )
      {
        const bool isFirstLabel = ( i == 0 );
        const bool isLastLabel = ( i == positions.size() - 1 );

        QString currentNumericLabel;
        if ( i < nSegmentsLeft )
        {
          if ( i == 0 )
          {
            currentNumericLabel = firstLabel;
          }
          else
          {
            // skip intermediate left segments
            continue;
          }
        }
        else
        {
          const double val = ( i - nSegmentsLeft + 1 ) * settings.unitsPerSegment();
          currentNumericLabel = settings.numericFormat()->formatDouble( val / settings.mapUnitsPerScaleBarUnit(), numericContext );
        }

        const double segmentWidthMM = ( i < nSegmentsLeft && nSegmentsLeft > 0 ) ? ( scaleContext.segmentWidth / nSegmentsLeft ) : scaleContext.segmentWidth;

        const QPointF pos( context.convertToPainterUnits( positions.at( i ) + ( segmentWidthMM / 2.0 ), Qgis::RenderUnit::Millimeters ) + xOffset, distanceLabelY );

        QString labelText = currentNumericLabel;
        if ( hasUnitLabel )
        {
          if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel )
               || ( isFirstLabel && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeFirstDistanceLabel ) ) )
          {
            labelText = unitLabel + labelText;
          }
          if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel )
               || ( isLastLabel && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterLastDistanceLabel ) ) )
          {
            labelText = labelText + unitLabel;
          }
        }

        scaleScope->addVariable( QgsExpressionContextScope::StaticVariable( u"scale_value"_s, currentNumericLabel, true, false ) );
        QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << labelText, context, format );
      }
      break;
    }

    case Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredEdge:
    {
      double currentLabelNumber = 0.0;
      int segmentCounter = 0;
      QString currentNumericLabel;

      for ( int i = 0; i < positions.size(); ++i )
      {
        if ( segmentCounter == 0 && nSegmentsLeft > 0 )
        {
          // label first left segment
          currentNumericLabel = firstLabel;
        }
        else if ( segmentCounter != 0 && segmentCounter == nSegmentsLeft )
        {
          // reset label number to 0 if there are left segments
          currentLabelNumber = 0.0;
        }

        if ( segmentCounter >= nSegmentsLeft )
        {
          currentNumericLabel = settings.numericFormat()->formatDouble( currentLabelNumber / settings.mapUnitsPerScaleBarUnit(), numericContext );
        }

        // don't draw label for intermediate left segments
        if ( segmentCounter == 0 || segmentCounter >= nSegmentsLeft )
        {
          const bool isFirstLabel = ( segmentCounter == 0 );

          scaleScope->addVariable( QgsExpressionContextScope::StaticVariable( u"scale_value"_s, currentNumericLabel, true, false ) );

          QPointF pos( context.convertToPainterUnits( positions.at( i ), Qgis::RenderUnit::Millimeters ) + xOffset, distanceLabelY );

          if ( isFirstLabel
               && hasUnitLabel
               && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeFirstDistanceLabel )
               && !unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) )
          {
            pos.setX( pos.x() + fontMetrics.horizontalAdvance( currentNumericLabel ) / 2.0 );
            QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Right, QStringList() << ( unitLabel + currentNumericLabel ), context, format );
          }
          else
          {
            QString labelText = currentNumericLabel;
            if ( hasUnitLabel )
            {
              if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) )
              {
                labelText = unitLabel + labelText;
              }
              if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel ) )
              {
                labelText = labelText + unitLabel;
              }
            }
            QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << labelText, context, format );
          }
        }

        if ( segmentCounter >= nSegmentsLeft )
        {
          currentLabelNumber += settings.unitsPerSegment();
        }
        ++segmentCounter;
      }

      // draw last label
      if ( !positions.isEmpty() )
      {
        currentNumericLabel = settings.numericFormat()->formatDouble( currentLabelNumber / settings.mapUnitsPerScaleBarUnit(), numericContext );
        QString lastLabelText = currentNumericLabel;

        const bool hasSuffix
          = hasUnitLabel
            && ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterLastDistanceLabel ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel ) );
        const bool hasPrefix = hasUnitLabel && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel );
        if ( hasPrefix )
        {
          lastLabelText = unitLabel + lastLabelText;
        }
        if ( hasSuffix )
        {
          lastLabelText = lastLabelText + unitLabel;
        }

        scaleScope->addVariable( QgsExpressionContextScope::StaticVariable( u"scale_value"_s, currentNumericLabel, true, false ) );
        QPointF pos;
        pos.setY( distanceLabelY );

        if ( hasSuffix
             && !hasPrefix
             && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterLastDistanceLabel )
             && !unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel ) )
        {
          pos.setX(
            context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, Qgis::RenderUnit::Millimeters )
            + xOffset
            - fontMetrics.horizontalAdvance( currentNumericLabel ) / 2.0
          );
          QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Left, QStringList() << ( currentNumericLabel + unitLabel ), context, format );
        }
        else
        {
          pos.setX( context.convertToPainterUnits( positions.at( positions.size() - 1 ) + scaleContext.segmentWidth, Qgis::RenderUnit::Millimeters ) + xOffset );
          QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << lastLabelText, context, format );
        }
      }
      break;
    }
  }

  // draw standalone unit labels
  if ( hasUnitLabel && !positions.isEmpty() )
  {
    const double barLeftX = context.convertToPainterUnits( positions.first(), Qgis::RenderUnit::Millimeters ) + xOffset;
    const double barRightX = context.convertToPainterUnits( positions.last() + scaleContext.segmentWidth, Qgis::RenderUnit::Millimeters ) + xOffset;

    const double unitLabelYAbove = scaledBoxContentSpace + fontMetrics.ascent();

    double unitLabelYBelow = barBottomPosition + scaledLabelBarSpace + fontMetrics.ascent();
    if ( hasDistanceLabelsBelow )
    {
      unitLabelYBelow += fontMetrics.ascent() + scaledLabelBarSpace;
    }

    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeBar ) )
    {
      const QPointF pos( barLeftX - scaledLabelBarSpace, barMiddleY + ( fontMetrics.ascent() - fontMetrics.descent() ) / 2.0 );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Right, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterBar ) )
    {
      const QPointF pos( barRightX + scaledLabelBarSpace, barMiddleY + ( fontMetrics.ascent() - fontMetrics.descent() ) / 2.0 );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Left, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftAbove ) )
    {
      const QPointF pos( barLeftX, unitLabelYAbove );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredAbove ) )
    {
      const QPointF pos( ( barLeftX + barRightX ) / 2.0, unitLabelYAbove );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightAbove ) )
    {
      const QPointF pos( barRightX, unitLabelYAbove );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftBelow ) )
    {
      const QPointF pos( barLeftX, unitLabelYBelow );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredBelow ) )
    {
      const QPointF pos( ( barLeftX + barRightX ) / 2.0, unitLabelYBelow );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightBelow ) )
    {
      const QPointF pos( barRightX, unitLabelYBelow );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::OnBarAfterFirstDivision ) )
    {
      const double firstDivCenterX = context.convertToPainterUnits( positions.value( nSegmentsLeft, positions.first() ) + scaleContext.segmentWidth / 2.0, Qgis::RenderUnit::Millimeters ) + xOffset;
      const QPointF pos( firstDivCenterX, barMiddleY + ( fontMetrics.ascent() - fontMetrics.descent() ) / 2.0 );
      QgsTextRenderer::drawText( pos, 0, Qgis::TextHorizontalAlignment::Center, QStringList() << unitLabel, context, format );
    }
  }

  painter->restore();
}

QgsScaleBarRenderer::Flags QgsScaleBarRenderer::flags() const
{
  return QgsScaleBarRenderer::Flags();
}

int QgsScaleBarRenderer::sortKey() const
{
  return 100;
}

QSizeF QgsScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings, const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  const QFont font = settings.textFormat().toQFont();

  // consider centered first label
  double firstLabelWidth = QgsLayoutUtils::textWidthMM( font, firstLabelString( settings ) );
  if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment )
  {
    if ( firstLabelWidth > scaleContext.segmentWidth )
    {
      firstLabelWidth = ( firstLabelWidth - scaleContext.segmentWidth ) / 2.0;
    }
    else
    {
      firstLabelWidth = 0.0;
    }
  }
  else
  {
    firstLabelWidth /= 2.0;
  }

  // consider last number and label
  const double largestLabelNumber = settings.numberOfSegments() * settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit();
  const QString largestNumberLabel = settings.numericFormat()->formatDouble( largestLabelNumber, QgsNumericFormatContext() );
  const QString largestLabel = largestNumberLabel + settings.unitLabel();
  double largestLabelWidth;
  if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment )
  {
    largestLabelWidth = QgsLayoutUtils::textWidthMM( font, largestLabel );
    if ( largestLabelWidth > scaleContext.segmentWidth )
    {
      largestLabelWidth = ( largestLabelWidth - scaleContext.segmentWidth ) / 2.0;
    }
    else
    {
      largestLabelWidth = 0.0;
    }
  }
  else
  {
    largestLabelWidth = QgsLayoutUtils::textWidthMM( font, largestLabel ) - QgsLayoutUtils::textWidthMM( font, largestNumberLabel ) / 2.0;
  }

  const double totalBarLength = scaleContext.segmentWidth * ( settings.numberOfSegments() + ( settings.numberOfSegmentsLeft() > 0 ? 1 : 0 ) );

  // this whole method is deprecated, so we can still call the deprecated settings.pen() getter
  Q_NOWARN_DEPRECATED_PUSH
  const double width = firstLabelWidth + totalBarLength + 2.0 * settings.pen().widthF() + largestLabelWidth + 2.0 * settings.boxContentSpace();
  Q_NOWARN_DEPRECATED_POP

  const double height = settings.height() + settings.labelBarSpace() + 2.0 * settings.boxContentSpace() + QgsLayoutUtils::fontAscentMM( font );

  return QSizeF( width, height );
}

QSizeF QgsScaleBarRenderer::calculateBoxSize( QgsRenderContext &context, const QgsScaleBarSettings &settings, const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  const double painterToMm = 1.0 / context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );
  const Qgis::ScaleBarUnitLabelPlacements unitPlacements = settings.unitLabelPlacements();
  const QString unitLabel = settings.unitLabel();
  const bool hasUnitLabel = !unitLabel.isEmpty();

  const QString firstNumLabel = ( settings.numberOfSegmentsLeft() > 0 )
                                  ? settings.numericFormat()->formatDouble( settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit(), QgsNumericFormatContext() )
                                  : settings.numericFormat()->formatDouble( 0, QgsNumericFormatContext() );

  const double firstSegmentWidth = ( settings.numberOfSegmentsLeft() > 0 ) ? ( scaleContext.segmentWidth / settings.numberOfSegmentsLeft() ) : scaleContext.segmentWidth;

  double firstLabelWidth = 0.0;
  if ( hasUnitLabel && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeFirstDistanceLabel ) && !unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) )
  {
    const QString fullFirstLabel = unitLabel + firstNumLabel;
    const double fullWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << fullFirstLabel ) * painterToMm;
    const double numWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << firstNumLabel ) * painterToMm;

    if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment && !std::isnan( scaleContext.segmentWidth ) )
    {
      firstLabelWidth = ( fullWidth > firstSegmentWidth ) ? ( fullWidth - firstSegmentWidth ) / 2.0 : 0.0;
    }
    else
    {
      firstLabelWidth = fullWidth - numWidth / 2.0;
    }
  }
  else
  {
    const QString fullFirstLabel = firstLabelString( settings );
    const double fullWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << fullFirstLabel ) * painterToMm;

    if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment && !std::isnan( scaleContext.segmentWidth ) )
    {
      firstLabelWidth = ( fullWidth > firstSegmentWidth ) ? ( fullWidth - firstSegmentWidth ) / 2.0 : 0.0;
    }
    else
    {
      firstLabelWidth = fullWidth / 2.0;
    }
  }

  const double largestLabelNumber = settings.numberOfSegments() * settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit();
  const QString largestNumberLabel = std::isnan( largestLabelNumber ) ? QString() : settings.numericFormat()->formatDouble( largestLabelNumber, QgsNumericFormatContext() );

  QString largestLabel = largestNumberLabel;
  if ( hasUnitLabel )
  {
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) )
    {
      largestLabel = unitLabel + largestLabel;
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterLastDistanceLabel ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel ) )
    {
      largestLabel = largestLabel + unitLabel;
    }
  }

  double largestLabelWidth;
  if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment && !std::isnan( scaleContext.segmentWidth ) )
  {
    largestLabelWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << largestLabel ) * painterToMm;

    if ( largestLabelWidth > scaleContext.segmentWidth )
    {
      largestLabelWidth = ( largestLabelWidth - scaleContext.segmentWidth ) / 2.0;
    }
    else
    {
      largestLabelWidth = 0.0;
    }
  }
  else
  {
    if ( hasUnitLabel
         && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterLastDistanceLabel )
         && !unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel )
         && !unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) )
    {
      largestLabelWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << largestLabel ) * painterToMm
                          - QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << largestNumberLabel ) * painterToMm / 2.0;
    }
    else
    {
      largestLabelWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << largestLabel ) * painterToMm / 2.0;
    }
  }

  // segmentWidth can be NaN in extreme cases, eg trying to make a scalebar for a global map with a very small segment size (eg meters)
  const double totalBarLength = std::isnan( scaleContext.segmentWidth ) ? 0 : scaleContext.segmentWidth * ( settings.numberOfSegments() + ( settings.numberOfSegmentsLeft() > 0 ? 1 : 0 ) );

  double lineWidth = QgsSymbolLayerUtils::estimateMaxSymbolBleed( settings.lineSymbol(), context ) * 2.0;
  // need to convert to mm
  lineWidth /= context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );

  double leftExtension = firstLabelWidth;
  double rightExtension = largestLabelWidth;

  if ( hasUnitLabel )
  {
    const double unitLabelWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << unitLabel ) * painterToMm;
    const double labelBarSpaceMM = settings.labelBarSpace();

    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeBar ) )
    {
      leftExtension = std::max( leftExtension, unitLabelWidth + labelBarSpaceMM );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterBar ) )
    {
      rightExtension = std::max( rightExtension, unitLabelWidth + labelBarSpaceMM );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftBelow ) )
    {
      leftExtension = std::max( leftExtension, unitLabelWidth / 2.0 );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightBelow ) )
    {
      rightExtension = std::max( rightExtension, unitLabelWidth / 2.0 );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredBelow ) )
    {
      const double centerOverhang = ( unitLabelWidth - totalBarLength ) / 2.0;
      if ( centerOverhang > 0 )
      {
        leftExtension = std::max( leftExtension, centerOverhang );
        rightExtension = std::max( rightExtension, centerOverhang );
      }
    }
  }

  const double width = leftExtension + totalBarLength + 2.0 * lineWidth + rightExtension + 2.0 * settings.boxContentSpace();
  double barHeight;
  if ( ( scaleContext.flags & Flag::FlagUsesSubdivisionsHeight ) && ( settings.numberOfSubdivisions() > 1 ) && ( settings.subdivisionsHeight() > settings.height() ) )
  {
    barHeight = settings.subdivisionsHeight();
  }
  else
  {
    barHeight = settings.height();
  }

  double height = barHeight + 2 * settings.boxContentSpace();

  // TODO -- we technically should check the height of ALL labels here and take the maximum
  const double textHeight = QgsTextRenderer::textHeight( context, settings.textFormat(), QStringList() << largestLabel ) * painterToMm;

  const bool hasDistanceLabelsAbove = ( settings.labelVerticalPlacement() == Qgis::ScaleBarDistanceLabelVerticalPlacement::AboveSegment );
  const bool hasDistanceLabelsBelow = ( settings.labelVerticalPlacement() == Qgis::ScaleBarDistanceLabelVerticalPlacement::BelowSegment );

  bool hasUnitLabelAbove = false;
  bool hasUnitLabelBelow = false;
  if ( hasUnitLabel )
  {
    hasUnitLabelAbove = unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftAbove )
                        || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredAbove )
                        || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightAbove );
    hasUnitLabelBelow = unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftBelow )
                        || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredBelow )
                        || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightBelow );
  }

  double topSpace = 0.0;
  if ( hasDistanceLabelsAbove )
  {
    topSpace += settings.labelBarSpace() + textHeight;
  }
  if ( hasUnitLabelAbove )
  {
    const double unitTextHeight = QgsTextRenderer::textHeight( context, settings.textFormat(), QStringList() << unitLabel ) * painterToMm;
    topSpace += settings.labelBarSpace() + unitTextHeight;
  }

  double bottomSpace = 0.0;
  if ( hasDistanceLabelsBelow )
  {
    bottomSpace += settings.labelBarSpace() + textHeight;
  }
  if ( hasUnitLabelBelow )
  {
    const double unitTextHeight = QgsTextRenderer::textHeight( context, settings.textFormat(), QStringList() << unitLabel ) * painterToMm;
    bottomSpace += settings.labelBarSpace() + unitTextHeight;
  }

  height += topSpace + bottomSpace;

  return QSizeF( width, height );
}

bool QgsScaleBarRenderer::applyDefaultSettings( QgsScaleBarSettings & ) const
{
  return false;
}

QString QgsScaleBarRenderer::firstLabelString( const QgsScaleBarSettings &settings ) const
{
  QString label;
  if ( settings.numberOfSegmentsLeft() > 0 )
  {
    label = settings.numericFormat()->formatDouble( settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit(), QgsNumericFormatContext() );
  }
  else
  {
    label = settings.numericFormat()->formatDouble( 0, QgsNumericFormatContext() );
  }

  if ( !settings.unitLabel().isEmpty() )
  {
    const Qgis::ScaleBarUnitLabelPlacements placements = settings.unitLabelPlacements();
    if ( placements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) || placements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeFirstDistanceLabel ) )
    {
      label = settings.unitLabel() + label;
    }
    if ( placements.testFlag( Qgis::ScaleBarUnitLabelPlacement::AfterEveryDistanceLabel ) )
    {
      label = label + settings.unitLabel();
    }
  }

  return label;
}

double QgsScaleBarRenderer::firstLabelXOffset( const QgsScaleBarSettings &settings ) const
{
  const QString firstLabel = firstLabelString( settings );
  Q_NOWARN_DEPRECATED_PUSH
  return QgsLayoutUtils::textWidthMM( settings.font(), firstLabel ) / 2.0;
  Q_NOWARN_DEPRECATED_POP
}

double QgsScaleBarRenderer::firstLabelXOffset( const QgsScaleBarSettings &settings, const QgsRenderContext &context, const ScaleBarContext &scaleContext ) const
{
  const Qgis::ScaleBarUnitLabelPlacements unitPlacements = settings.unitLabelPlacements();
  const QString unitLabel = settings.unitLabel();
  const bool hasUnitLabel = !unitLabel.isEmpty();

  const QString firstNumLabel = ( settings.numberOfSegmentsLeft() > 0 )
                                  ? settings.numericFormat()->formatDouble( settings.unitsPerSegment() / settings.mapUnitsPerScaleBarUnit(), QgsNumericFormatContext() )
                                  : settings.numericFormat()->formatDouble( 0, QgsNumericFormatContext() );

  const double firstSegmentWidth = ( settings.numberOfSegmentsLeft() > 0 ) ? ( scaleContext.segmentWidth / settings.numberOfSegmentsLeft() ) : scaleContext.segmentWidth;
  const double firstSegmentWidthPx = context.convertToPainterUnits( firstSegmentWidth, Qgis::RenderUnit::Millimeters );

  double firstLabelWidth = 0.0;

  if ( hasUnitLabel && unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeFirstDistanceLabel ) && !unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeEveryDistanceLabel ) )
  {
    const QString fullFirstLabel = unitLabel + firstNumLabel;
    const double fullWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << fullFirstLabel );
    const double numWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << firstNumLabel );

    if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment && scaleContext.isValid() )
    {
      if ( fullWidth > firstSegmentWidthPx )
      {
        firstLabelWidth = ( fullWidth - firstSegmentWidthPx ) / 2.0;
      }
      else
      {
        firstLabelWidth = 0.0;
      }
    }
    else
    {
      firstLabelWidth = fullWidth - numWidth / 2.0;
    }
  }
  else
  {
    const QString fullFirstLabel = firstLabelString( settings );
    const double fullWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << fullFirstLabel );

    if ( settings.labelHorizontalPlacement() == Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment && scaleContext.isValid() )
    {
      if ( fullWidth > firstSegmentWidthPx )
      {
        firstLabelWidth = ( fullWidth - firstSegmentWidthPx ) / 2.0;
      }
      else
      {
        firstLabelWidth = 0.0;
      }
    }
    else
    {
      firstLabelWidth = fullWidth / 2.0;
    }
  }

  double xOffset = firstLabelWidth;

  if ( hasUnitLabel )
  {
    const double unitLabelWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << unitLabel );
    const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), Qgis::RenderUnit::Millimeters );

    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::BeforeBar ) )
    {
      xOffset = std::max( xOffset, unitLabelWidth + scaledLabelBarSpace );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftBelow ) )
    {
      xOffset = std::max( xOffset, unitLabelWidth / 2.0 );
    }
    if ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredBelow ) )
    {
      const double totalBarLengthMM = scaleContext.segmentWidth * ( settings.numberOfSegments() + ( settings.numberOfSegmentsLeft() > 0 ? 1 : 0 ) );
      const double totalBarLengthPx = context.convertToPainterUnits( totalBarLengthMM, Qgis::RenderUnit::Millimeters );
      const double centerOverhang = ( unitLabelWidth - totalBarLengthPx ) / 2.0;
      if ( centerOverhang > 0 )
      {
        xOffset = std::max( xOffset, centerOverhang );
      }
    }
  }

  return xOffset;
}

QList<double> QgsScaleBarRenderer::segmentPositions( const ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const
{
  QList<double> positions;

  // this whole method is deprecated, so calling a deprecated function is fine
  Q_NOWARN_DEPRECATED_PUSH
  double currentXCoord = settings.pen().widthF() + settings.boxContentSpace();
  Q_NOWARN_DEPRECATED_POP

  // left segments
  const double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
  positions.reserve( settings.numberOfSegmentsLeft() + settings.numberOfSegments() );
  for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += leftSegmentSize;
  }

  // right segments
  for ( int i = 0; i < settings.numberOfSegments(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += scaleContext.segmentWidth;
  }
  return positions;
}

QList<double> QgsScaleBarRenderer::segmentPositions( QgsRenderContext &context, const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const
{
  QList<double> positions;

  double lineWidth = QgsSymbolLayerUtils::estimateMaxSymbolBleed( settings.lineSymbol(), context ) * 2.0;
  // need to convert to mm
  lineWidth /= context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );

  double currentXCoord = lineWidth + settings.boxContentSpace();

  // left segments
  const double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
  positions.reserve( settings.numberOfSegmentsLeft() + settings.numberOfSegments() );
  for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
  {
    positions << currentXCoord;
    currentXCoord += leftSegmentSize;
  }

  // right segments
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

  // left segments
  if ( settings.numberOfSegmentsLeft() > 0 )
  {
    const double leftSegmentSize = scaleContext.segmentWidth / settings.numberOfSegmentsLeft();
    for ( int i = 0; i < settings.numberOfSegmentsLeft(); ++i )
    {
      widths << leftSegmentSize;
    }
  }

  // right segments
  for ( int i = 0; i < settings.numberOfSegments(); ++i )
  {
    widths << scaleContext.segmentWidth;
  }

  return widths;
}

double QgsScaleBarRenderer::verticalOffset( QgsRenderContext &context, const QgsScaleBarSettings &settings ) const
{
  const double scaledLabelBarSpace = context.convertToPainterUnits( settings.labelBarSpace(), Qgis::RenderUnit::Millimeters );
  const double scaledBoxContentSpace = context.convertToPainterUnits( settings.boxContentSpace(), Qgis::RenderUnit::Millimeters );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, settings.textFormat() );

  const Qgis::ScaleBarUnitLabelPlacements unitPlacements = settings.unitLabelPlacements();
  const bool hasUnitLabelAbove
    = !settings.unitLabel().isEmpty()
      && ( unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::LeftAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::CenteredAbove ) || unitPlacements.testFlag( Qgis::ScaleBarUnitLabelPlacement::RightAbove ) );

  double barTopPosition = scaledBoxContentSpace;
  if ( hasUnitLabelAbove )
  {
    barTopPosition += fontMetrics.ascent() + scaledLabelBarSpace;
  }
  if ( settings.labelVerticalPlacement() == Qgis::ScaleBarDistanceLabelVerticalPlacement::AboveSegment )
  {
    barTopPosition += fontMetrics.ascent() + scaledLabelBarSpace;
  }

  return barTopPosition;
}

bool QgsScaleBarRenderer::ScaleBarContext::isValid() const
{
  return !std::isnan( segmentWidth );
}
