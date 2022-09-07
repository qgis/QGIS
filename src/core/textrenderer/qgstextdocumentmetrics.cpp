/***************************************************************************
  qgstextdocumentmetrics.cpp
  -----------------
   begin                : September 2022
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstextdocumentmetrics.h"
#include "qgis.h"
#include "qgsstringutils.h"
#include "qgstextblock.h"
#include "qgstextfragment.h"
#include "qgstextformat.h"
#include "qgstextdocument.h"
#include "qgsrendercontext.h"

#include <QFontMetricsF>

QgsTextDocumentMetrics QgsTextDocumentMetrics::calculateMetrics( const QgsTextDocument &document, const QgsTextFormat &format, const QgsRenderContext &context, double scaleFactor )
{
  QgsTextDocumentMetrics res;

  bool isNullSize = false;
  const QFont font = format.scaledFont( context, scaleFactor, &isNullSize );
  if ( isNullSize )
    return res;

  // for absolute line heights
  const double lineHeightPainterUnits = context.convertToPainterUnits( format.lineHeight(), format.lineHeightUnit() );

  // TODO orientation handling

  double width = 0;
  double heightLabelMode = 0;
  double heightPointRectMode = 0;
  const int blockSize = document.size();
  res.mFragmentFonts.reserve( blockSize );
  double currentLabelBaseline = 0;
  double currentPointBaseline = 0;
  double currentRectBaseline = 0;
  double lastLineLeading = 0;

  double heightVerticalOrientation = 0;

  QVector < double > blockVerticalLineSpacing;

  for ( int blockIndex = 0; blockIndex < blockSize; blockIndex++ )
  {
    const QgsTextBlock &block = document.at( blockIndex );

    double blockWidth = 0;
    double blockHeightUsingAscentDescent = 0;
    double blockHeightUsingLineSpacing = 0;
    double blockHeightVerticalOrientation = 0;
    const int fragmentSize = block.size();

    double maxBlockAscent = 0;
    double maxBlockDescent = 0;
    double maxLineSpacing = 0;
    double maxBlockLeading = 0;
    double maxBlockMaxWidth = 0;

    QList< QFont > fragmentFonts;
    fragmentFonts.reserve( fragmentSize );
    for ( int fragmentIndex = 0; fragmentIndex < fragmentSize; ++fragmentIndex )
    {
      const QgsTextFragment &fragment = block.at( fragmentIndex );

      QFont updatedFont = font;
      fragment.characterFormat().updateFontForFormat( updatedFont, context, scaleFactor );
      const QFontMetricsF fm( updatedFont );

      const double fragmentWidth = fm.horizontalAdvance( fragment.text() ) / scaleFactor;
      const double fragmentHeightUsingAscentDescent = ( fm.ascent() + fm.descent() ) / scaleFactor;
      const double fragmentHeightUsingLineSpacing = fm.lineSpacing() / scaleFactor;

      blockWidth += fragmentWidth;
      blockHeightUsingAscentDescent = std::max( blockHeightUsingAscentDescent, fragmentHeightUsingAscentDescent );
      blockHeightUsingLineSpacing = std::max( blockHeightUsingLineSpacing, fragmentHeightUsingLineSpacing );
      maxBlockAscent = std::max( maxBlockAscent, fm.ascent() / scaleFactor );
      maxBlockDescent = std::max( maxBlockDescent, fm.descent() / scaleFactor );
      maxBlockMaxWidth = std::max( maxBlockMaxWidth, fm.maxWidth() / scaleFactor );

      if ( ( fm.lineSpacing() / scaleFactor ) > maxLineSpacing )
      {
        maxLineSpacing = fm.lineSpacing() / scaleFactor;
        maxBlockLeading = fm.leading() / scaleFactor;
      }

      fragmentFonts << updatedFont;

      const double verticalOrientationFragmentHeight = fragmentIndex == 0 ? ( fm.ascent() / scaleFactor * fragment.text().size() + ( fragment.text().size() - 1 ) * updatedFont.letterSpacing() / scaleFactor )
          : ( fragment.text().size() * ( fm.ascent() / scaleFactor + updatedFont.letterSpacing() / scaleFactor ) );
      blockHeightVerticalOrientation += verticalOrientationFragmentHeight;
    }

    if ( blockIndex == 0 )
    {
      // same logic as used in QgsTextRenderer. (?!!)
      // needed to move bottom of text's descender to within bottom edge of label
      res.mFirstLineAscentOffset = 0.25 * maxBlockAscent; // descent() is not enough
      res.mLastLineAscentOffset = res.mFirstLineAscentOffset;
      const double lineHeight = ( maxBlockAscent + maxBlockDescent ); // ignore +1 for baseline

      // rendering labels needs special handling - in this case text should be
      // drawn with the bottom left corner coinciding with origin, vs top left
      // for standard text rendering. Line height is also slightly different.
      currentLabelBaseline = -res.mFirstLineAscentOffset;

      // standard rendering - designed to exactly replicate QPainter's drawText method
      currentRectBaseline = -res.mFirstLineAscentOffset + lineHeight - 1 /*baseline*/;

      // standard rendering - designed to exactly replicate QPainter's drawText rect method
      currentPointBaseline = 0;

      heightLabelMode += blockHeightUsingAscentDescent;
      heightPointRectMode += blockHeightUsingAscentDescent;
    }
    else
    {
      const double thisLineHeightUsingAscentDescent = format.lineHeightUnit() == QgsUnitTypes::RenderPercentage ? ( format.lineHeight() * ( maxBlockAscent + maxBlockDescent ) ) : lineHeightPainterUnits;
      const double thisLineHeightUsingLineSpacing = format.lineHeightUnit() == QgsUnitTypes::RenderPercentage ? ( format.lineHeight() * maxLineSpacing ) : lineHeightPainterUnits;

      currentLabelBaseline += thisLineHeightUsingAscentDescent;
      currentRectBaseline += thisLineHeightUsingLineSpacing;
      currentPointBaseline += thisLineHeightUsingLineSpacing;

      heightLabelMode += thisLineHeightUsingAscentDescent;
      heightPointRectMode += thisLineHeightUsingLineSpacing;
      if ( blockIndex == blockSize - 1 )
        res.mLastLineAscentOffset = 0.25 * maxBlockAscent;
    }

    blockVerticalLineSpacing << ( format.lineHeightUnit() == QgsUnitTypes::RenderPercentage ? ( maxBlockMaxWidth * format.lineHeight() ) : lineHeightPainterUnits );

    res.mBlockHeights << blockHeightUsingLineSpacing;

    width = std::max( width, blockWidth );
    heightVerticalOrientation = std::max( heightVerticalOrientation, blockHeightVerticalOrientation );
    res.mBlockWidths << blockWidth;
    res.mFragmentFonts << fragmentFonts;
    res.mBaselineOffsetsLabelMode << currentLabelBaseline;
    res.mBaselineOffsetsPointMode << currentPointBaseline;
    res.mBaselineOffsetsRectMode << currentRectBaseline;
    res.mBlockMaxDescent << maxBlockDescent;
    res.mBlockMaxCharacterWidth << maxBlockMaxWidth;

    if ( blockIndex > 0 )
      lastLineLeading = maxBlockLeading;
  }

  heightLabelMode -= lastLineLeading;
  heightPointRectMode -= lastLineLeading;

  res.mDocumentSizeLabelMode = QSizeF( width, heightLabelMode );
  res.mDocumentSizePointRectMode = QSizeF( width, heightPointRectMode );

  // adjust baselines
  if ( !res.mBaselineOffsetsLabelMode.isEmpty() )
  {
    const double labelModeBaselineAdjust = res.mBaselineOffsetsLabelMode.constLast() + res.mLastLineAscentOffset;
    const double pointModeBaselineAdjust = res.mBaselineOffsetsPointMode.constLast();
    for ( int i = 0; i < blockSize; ++i )
    {
      res.mBaselineOffsetsLabelMode[i] -= labelModeBaselineAdjust;
      res.mBaselineOffsetsPointMode[i] -= pointModeBaselineAdjust;
    }
  }

  if ( !res.mBlockMaxCharacterWidth.isEmpty() )
  {
    QList< double > adjustedRightToLeftXOffsets;
    double currentOffset = 0;
    const int size = res.mBlockMaxCharacterWidth.size();

    double widthVerticalOrientation = 0;
    for ( int i = 0; i < size; ++i )
    {
      const double rightToLeftBlockMaxCharacterWidth = res.mBlockMaxCharacterWidth[size - 1 - i ];
      const double rightToLeftLineSpacing = blockVerticalLineSpacing[ size - 1 - i ];

      adjustedRightToLeftXOffsets << currentOffset;
      currentOffset += rightToLeftLineSpacing;

      if ( i == size - 1 )
        widthVerticalOrientation += rightToLeftBlockMaxCharacterWidth;
      else
        widthVerticalOrientation += rightToLeftLineSpacing;
    }
    std::reverse( adjustedRightToLeftXOffsets.begin(), adjustedRightToLeftXOffsets.end() );
    res.mVerticalOrientationXOffsets = adjustedRightToLeftXOffsets;

    res.mDocumentSizeVerticalOrientation = QSizeF( widthVerticalOrientation, heightVerticalOrientation );
  }

  return res;
}

QSizeF QgsTextDocumentMetrics::documentSize( Qgis::TextLayoutMode mode, Qgis::TextOrientation orientation ) const
{
  switch ( orientation )
  {
    case Qgis::TextOrientation::Horizontal:
      switch ( mode )
      {
        case Qgis::TextLayoutMode::Rectangle:
        case Qgis::TextLayoutMode::Point:
          return mDocumentSizePointRectMode;

        case Qgis::TextLayoutMode::Labeling:
          return mDocumentSizeLabelMode;
      };
      BUILTIN_UNREACHABLE

    case Qgis::TextOrientation::Vertical:
      return mDocumentSizeVerticalOrientation;
    case Qgis::TextOrientation::RotationBased:
      return QSizeF(); // label mode only
  }

  BUILTIN_UNREACHABLE
}

double QgsTextDocumentMetrics::blockWidth( int blockIndex ) const
{
  return mBlockWidths.value( blockIndex );
}

double QgsTextDocumentMetrics::blockHeight( int blockIndex ) const
{
  return mBlockHeights.value( blockIndex );
}

double QgsTextDocumentMetrics::baselineOffset( int blockIndex, Qgis::TextLayoutMode mode ) const
{
  switch ( mode )
  {
    case Qgis::TextLayoutMode::Rectangle:
      return mBaselineOffsetsRectMode.value( blockIndex );
    case Qgis::TextLayoutMode::Point:
      return mBaselineOffsetsPointMode.value( blockIndex );
    case Qgis::TextLayoutMode::Labeling:
      return mBaselineOffsetsLabelMode.value( blockIndex );
  }
  BUILTIN_UNREACHABLE
}

double QgsTextDocumentMetrics::verticalOrientationXOffset( int blockIndex ) const
{
  return mVerticalOrientationXOffsets.value( blockIndex );
}

double QgsTextDocumentMetrics::blockMaximumCharacterWidth( int blockIndex ) const
{
  return mBlockMaxCharacterWidth.value( blockIndex );
}

double QgsTextDocumentMetrics::blockMaximumDescent( int blockIndex ) const
{
  return mBlockMaxDescent.value( blockIndex );
}

QFont QgsTextDocumentMetrics::fragmentFont( int blockIndex, int fragmentIndex ) const
{
  return mFragmentFonts.value( blockIndex ).value( fragmentIndex );
}

